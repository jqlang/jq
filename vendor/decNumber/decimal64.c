/* ------------------------------------------------------------------ */
/* Decimal 64-bit format module                                       */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2009.  All rights reserved.   */
/*                                                                    */
/* This software is made available under the terms of the             */
/* ICU License -- ICU 1.8.1 and later.                                */
/*                                                                    */
/* The description and User's Guide ("The decNumber C Library") for   */
/* this software is called decNumber.pdf.  This document is           */
/* available, together with arithmetic and format specifications,     */
/* testcases, and Web links, on the General Decimal Arithmetic page.  */
/*                                                                    */
/* Please send comments, suggestions, and corrections to the author:  */
/*   mfc@uk.ibm.com                                                   */
/*   Mike Cowlishaw, IBM Fellow                                       */
/*   IBM UK, PO Box 31, Birmingham Road, Warwick CV34 5JL, UK         */
/* ------------------------------------------------------------------ */
/* This module comprises the routines for decimal64 format numbers.   */
/* Conversions are supplied to and from decNumber and String.         */
/*                                                                    */
/* This is used when decNumber provides operations, either for all    */
/* operations or as a proxy between decNumber and decSingle.          */
/*                                                                    */
/* Error handling is the same as decNumber (qv.).                     */
/* ------------------------------------------------------------------ */
#include <string.h>           // [for memset/memcpy]
#include <stdio.h>            // [for printf]

#define  DECNUMDIGITS 16      // make decNumbers with space for 16
#include "decNumber.h"        // base number library
#include "decNumberLocal.h"   // decNumber local types, etc.
#include "decimal64.h"        // our primary include

/* Utility routines and tables [in decimal64.c]; externs for C++ */
// DPD2BIN and the reverse are renamed to prevent link-time conflict
// if decQuad is also built in the same executable
#define DPD2BIN DPD2BINx
#define BIN2DPD BIN2DPDx
extern const uInt COMBEXP[32], COMBMSD[32];
extern const uShort DPD2BIN[1024];
extern const uShort BIN2DPD[1000];
extern const uByte  BIN2CHAR[4001];

extern void decDigitsFromDPD(decNumber *, const uInt *, Int);
extern void decDigitsToDPD(const decNumber *, uInt *, Int);

#if DECTRACE || DECCHECK
void decimal64Show(const decimal64 *);            // for debug
extern void decNumberShow(const decNumber *);     // ..
#endif

/* Useful macro */
// Clear a structure (e.g., a decNumber)
#define DEC_clear(d) memset(d, 0, sizeof(*d))

/* define and include the tables to use for conversions */
#define DEC_BIN2CHAR 1
#define DEC_DPD2BIN  1
#define DEC_BIN2DPD  1             // used for all sizes
#include "decDPD.h"                // lookup tables

/* ------------------------------------------------------------------ */
/* decimal64FromNumber -- convert decNumber to decimal64              */
/*                                                                    */
/*   ds is the target decimal64                                       */
/*   dn is the source number (assumed valid)                          */
/*   set is the context, used only for reporting errors               */
/*                                                                    */
/* The set argument is used only for status reporting and for the     */
/* rounding mode (used if the coefficient is more than DECIMAL64_Pmax */
/* digits or an overflow is detected).  If the exponent is out of the */
/* valid range then Overflow or Underflow will be raised.             */
/* After Underflow a subnormal result is possible.                    */
/*                                                                    */
/* DEC_Clamped is set if the number has to be 'folded down' to fit,   */
/* by reducing its exponent and multiplying the coefficient by a      */
/* power of ten, or if the exponent on a zero had to be clamped.      */
/* ------------------------------------------------------------------ */
decimal64 * decimal64FromNumber(decimal64 *d64, const decNumber *dn,
                                decContext *set) {
  uInt status=0;                   // status accumulator
  Int ae;                          // adjusted exponent
  decNumber  dw;                   // work
  decContext dc;                   // ..
  uInt comb, exp;                  // ..
  uInt uiwork;                     // for macros
  uInt targar[2]={0, 0};           // target 64-bit
  #define targhi targar[1]         // name the word with the sign
  #define targlo targar[0]         // and the other

  // If the number has too many digits, or the exponent could be
  // out of range then reduce the number under the appropriate
  // constraints.  This could push the number to Infinity or zero,
  // so this check and rounding must be done before generating the
  // decimal64]
  ae=dn->exponent+dn->digits-1;              // [0 if special]
  if (dn->digits>DECIMAL64_Pmax              // too many digits
   || ae>DECIMAL64_Emax                      // likely overflow
   || ae<DECIMAL64_Emin) {                   // likely underflow
    decContextDefault(&dc, DEC_INIT_DECIMAL64); // [no traps]
    dc.round=set->round;                     // use supplied rounding
    decNumberPlus(&dw, dn, &dc);             // (round and check)
    // [this changes -0 to 0, so enforce the sign...]
    dw.bits|=dn->bits&DECNEG;
    status=dc.status;                        // save status
    dn=&dw;                                  // use the work number
    } // maybe out of range

  if (dn->bits&DECSPECIAL) {                      // a special value
    if (dn->bits&DECINF) targhi=DECIMAL_Inf<<24;
     else {                                       // sNaN or qNaN
      if ((*dn->lsu!=0 || dn->digits>1)           // non-zero coefficient
       && (dn->digits<DECIMAL64_Pmax)) {          // coefficient fits
        decDigitsToDPD(dn, targar, 0);
        }
      if (dn->bits&DECNAN) targhi|=DECIMAL_NaN<<24;
       else targhi|=DECIMAL_sNaN<<24;
      } // a NaN
    } // special

   else { // is finite
    if (decNumberIsZero(dn)) {               // is a zero
      // set and clamp exponent
      if (dn->exponent<-DECIMAL64_Bias) {
        exp=0;                               // low clamp
        status|=DEC_Clamped;
        }
       else {
        exp=dn->exponent+DECIMAL64_Bias;     // bias exponent
        if (exp>DECIMAL64_Ehigh) {           // top clamp
          exp=DECIMAL64_Ehigh;
          status|=DEC_Clamped;
          }
        }
      comb=(exp>>5) & 0x18;             // msd=0, exp top 2 bits ..
      }
     else {                             // non-zero finite number
      uInt msd;                         // work
      Int pad=0;                        // coefficient pad digits

      // the dn is known to fit, but it may need to be padded
      exp=(uInt)(dn->exponent+DECIMAL64_Bias);    // bias exponent
      if (exp>DECIMAL64_Ehigh) {                  // fold-down case
        pad=exp-DECIMAL64_Ehigh;
        exp=DECIMAL64_Ehigh;                      // [to maximum]
        status|=DEC_Clamped;
        }

      // fastpath common case
      if (DECDPUN==3 && pad==0) {
        uInt dpd[6]={0,0,0,0,0,0};
        uInt i;
        Int d=dn->digits;
        for (i=0; d>0; i++, d-=3) dpd[i]=BIN2DPD[dn->lsu[i]];
        targlo =dpd[0];
        targlo|=dpd[1]<<10;
        targlo|=dpd[2]<<20;
        if (dn->digits>6) {
          targlo|=dpd[3]<<30;
          targhi =dpd[3]>>2;
          targhi|=dpd[4]<<8;
          }
        msd=dpd[5];                // [did not really need conversion]
        }
       else { // general case
        decDigitsToDPD(dn, targar, pad);
        // save and clear the top digit
        msd=targhi>>18;
        targhi&=0x0003ffff;
        }

      // create the combination field
      if (msd>=8) comb=0x18 | ((exp>>7) & 0x06) | (msd & 0x01);
             else comb=((exp>>5) & 0x18) | msd;
      }
    targhi|=comb<<26;              // add combination field ..
    targhi|=(exp&0xff)<<18;        // .. and exponent continuation
    } // finite

  if (dn->bits&DECNEG) targhi|=0x80000000; // add sign bit

  // now write to storage; this is now always endian
  if (DECLITEND) {
    // lo int then hi
    UBFROMUI(d64->bytes,   targar[0]);
    UBFROMUI(d64->bytes+4, targar[1]);
    }
   else {
    // hi int then lo
    UBFROMUI(d64->bytes,   targar[1]);
    UBFROMUI(d64->bytes+4, targar[0]);
    }

  if (status!=0) decContextSetStatus(set, status); // pass on status
  // decimal64Show(d64);
  return d64;
  } // decimal64FromNumber

/* ------------------------------------------------------------------ */
/* decimal64ToNumber -- convert decimal64 to decNumber                */
/*   d64 is the source decimal64                                      */
/*   dn is the target number, with appropriate space                  */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
decNumber * decimal64ToNumber(const decimal64 *d64, decNumber *dn) {
  uInt msd;                        // coefficient MSD
  uInt exp;                        // exponent top two bits
  uInt comb;                       // combination field
  Int  need;                       // work
  uInt uiwork;                     // for macros
  uInt sourar[2];                  // source 64-bit
  #define sourhi sourar[1]         // name the word with the sign
  #define sourlo sourar[0]         // and the lower word

  // load source from storage; this is endian
  if (DECLITEND) {
    sourlo=UBTOUI(d64->bytes  );   // directly load the low int
    sourhi=UBTOUI(d64->bytes+4);   // then the high int
    }
   else {
    sourhi=UBTOUI(d64->bytes  );   // directly load the high int
    sourlo=UBTOUI(d64->bytes+4);   // then the low int
    }

  comb=(sourhi>>26)&0x1f;          // combination field

  decNumberZero(dn);               // clean number
  if (sourhi&0x80000000) dn->bits=DECNEG; // set sign if negative

  msd=COMBMSD[comb];               // decode the combination field
  exp=COMBEXP[comb];               // ..

  if (exp==3) {                    // is a special
    if (msd==0) {
      dn->bits|=DECINF;
      return dn;                   // no coefficient needed
      }
    else if (sourhi&0x02000000) dn->bits|=DECSNAN;
    else dn->bits|=DECNAN;
    msd=0;                         // no top digit
    }
   else {                          // is a finite number
    dn->exponent=(exp<<8)+((sourhi>>18)&0xff)-DECIMAL64_Bias; // unbiased
    }

  // get the coefficient
  sourhi&=0x0003ffff;              // clean coefficient continuation
  if (msd) {                       // non-zero msd
    sourhi|=msd<<18;               // prefix to coefficient
    need=6;                        // process 6 declets
    }
   else { // msd=0
    if (!sourhi) {                 // top word 0
      if (!sourlo) return dn;      // easy: coefficient is 0
      need=3;                      // process at least 3 declets
      if (sourlo&0xc0000000) need++; // process 4 declets
      // [could reduce some more, here]
      }
     else {                        // some bits in top word, msd=0
      need=4;                      // process at least 4 declets
      if (sourhi&0x0003ff00) need++; // top declet!=0, process 5
      }
    } //msd=0

  decDigitsFromDPD(dn, sourar, need);   // process declets
  return dn;
  } // decimal64ToNumber


/* ------------------------------------------------------------------ */
/* to-scientific-string -- conversion to numeric string               */
/* to-engineering-string -- conversion to numeric string              */
/*                                                                    */
/*   decimal64ToString(d64, string);                                  */
/*   decimal64ToEngString(d64, string);                               */
/*                                                                    */
/*  d64 is the decimal64 format number to convert                     */
/*  string is the string where the result will be laid out            */
/*                                                                    */
/*  string must be at least 24 characters                             */
/*                                                                    */
/*  No error is possible, and no status can be set.                   */
/* ------------------------------------------------------------------ */
char * decimal64ToEngString(const decimal64 *d64, char *string){
  decNumber dn;                         // work
  decimal64ToNumber(d64, &dn);
  decNumberToEngString(&dn, string);
  return string;
  } // decimal64ToEngString

char * decimal64ToString(const decimal64 *d64, char *string){
  uInt msd;                        // coefficient MSD
  Int  exp;                        // exponent top two bits or full
  uInt comb;                       // combination field
  char *cstart;                    // coefficient start
  char *c;                         // output pointer in string
  const uByte *u;                  // work
  char *s, *t;                     // .. (source, target)
  Int  dpd;                        // ..
  Int  pre, e;                     // ..
  uInt uiwork;                     // for macros

  uInt sourar[2];                  // source 64-bit
  #define sourhi sourar[1]         // name the word with the sign
  #define sourlo sourar[0]         // and the lower word

  // load source from storage; this is endian
  if (DECLITEND) {
    sourlo=UBTOUI(d64->bytes  );   // directly load the low int
    sourhi=UBTOUI(d64->bytes+4);   // then the high int
    }
   else {
    sourhi=UBTOUI(d64->bytes  );   // directly load the high int
    sourlo=UBTOUI(d64->bytes+4);   // then the low int
    }

  c=string;                        // where result will go
  if (((Int)sourhi)<0) *c++='-';   // handle sign

  comb=(sourhi>>26)&0x1f;          // combination field
  msd=COMBMSD[comb];               // decode the combination field
  exp=COMBEXP[comb];               // ..

  if (exp==3) {
    if (msd==0) {                  // infinity
      strcpy(c,   "Inf");
      strcpy(c+3, "inity");
      return string;               // easy
      }
    if (sourhi&0x02000000) *c++='s'; // sNaN
    strcpy(c, "NaN");              // complete word
    c+=3;                          // step past
    if (sourlo==0 && (sourhi&0x0003ffff)==0) return string; // zero payload
    // otherwise drop through to add integer; set correct exp
    exp=0; msd=0;                  // setup for following code
    }
   else exp=(exp<<8)+((sourhi>>18)&0xff)-DECIMAL64_Bias;

  // convert 16 digits of significand to characters
  cstart=c;                        // save start of coefficient
  if (msd) *c++='0'+(char)msd;     // non-zero most significant digit

  // Now decode the declets.  After extracting each one, it is
  // decoded to binary and then to a 4-char sequence by table lookup;
  // the 4-chars are a 1-char length (significant digits, except 000
  // has length 0).  This allows us to left-align the first declet
  // with non-zero content, then remaining ones are full 3-char
  // length.  We use fixed-length memcpys because variable-length
  // causes a subroutine call in GCC.  (These are length 4 for speed
  // and are safe because the array has an extra terminator byte.)
  #define dpd2char u=&BIN2CHAR[DPD2BIN[dpd]*4];                   \
                   if (c!=cstart) {memcpy(c, u+1, 4); c+=3;}      \
                    else if (*u)  {memcpy(c, u+4-*u, 4); c+=*u;}

  dpd=(sourhi>>8)&0x3ff;                     // declet 1
  dpd2char;
  dpd=((sourhi&0xff)<<2) | (sourlo>>30);     // declet 2
  dpd2char;
  dpd=(sourlo>>20)&0x3ff;                    // declet 3
  dpd2char;
  dpd=(sourlo>>10)&0x3ff;                    // declet 4
  dpd2char;
  dpd=(sourlo)&0x3ff;                        // declet 5
  dpd2char;

  if (c==cstart) *c++='0';         // all zeros -- make 0

  if (exp==0) {                    // integer or NaN case -- easy
    *c='\0';                       // terminate
    return string;
    }

  /* non-0 exponent */
  e=0;                             // assume no E
  pre=c-cstart+exp;
  // [here, pre-exp is the digits count (==1 for zero)]
  if (exp>0 || pre<-5) {           // need exponential form
    e=pre-1;                       // calculate E value
    pre=1;                         // assume one digit before '.'
    } // exponential form

  /* modify the coefficient, adding 0s, '.', and E+nn as needed */
  s=c-1;                           // source (LSD)
  if (pre>0) {                     // ddd.ddd (plain), perhaps with E
    char *dotat=cstart+pre;
    if (dotat<c) {                 // if embedded dot needed...
      t=c;                              // target
      for (; s>=dotat; s--, t--) *t=*s; // open the gap; leave t at gap
      *t='.';                           // insert the dot
      c++;                              // length increased by one
      }

    // finally add the E-part, if needed; it will never be 0, and has
    // a maximum length of 3 digits
    if (e!=0) {
      *c++='E';                    // starts with E
      *c++='+';                    // assume positive
      if (e<0) {
        *(c-1)='-';                // oops, need '-'
        e=-e;                      // uInt, please
        }
      u=&BIN2CHAR[e*4];            // -> length byte
      memcpy(c, u+4-*u, 4);        // copy fixed 4 characters [is safe]
      c+=*u;                       // bump pointer appropriately
      }
    *c='\0';                       // add terminator
    //printf("res %s\n", string);
    return string;
    } // pre>0

  /* -5<=pre<=0: here for plain 0.ddd or 0.000ddd forms (can never have E) */
  t=c+1-pre;
  *(t+1)='\0';                          // can add terminator now
  for (; s>=cstart; s--, t--) *t=*s;    // shift whole coefficient right
  c=cstart;
  *c++='0';                             // always starts with 0.
  *c++='.';
  for (; pre<0; pre++) *c++='0';        // add any 0's after '.'
  //printf("res %s\n", string);
  return string;
  } // decimal64ToString

/* ------------------------------------------------------------------ */
/* to-number -- conversion from numeric string                        */
/*                                                                    */
/*   decimal64FromString(result, string, set);                        */
/*                                                                    */
/*  result  is the decimal64 format number which gets the result of   */
/*          the conversion                                            */
/*  *string is the character string which should contain a valid      */
/*          number (which may be a special value)                     */
/*  set     is the context                                            */
/*                                                                    */
/* The context is supplied to this routine is used for error handling */
/* (setting of status and traps) and for the rounding mode, only.     */
/* If an error occurs, the result will be a valid decimal64 NaN.      */
/* ------------------------------------------------------------------ */
decimal64 * decimal64FromString(decimal64 *result, const char *string,
                                decContext *set) {
  decContext dc;                             // work
  decNumber dn;                              // ..

  decContextDefault(&dc, DEC_INIT_DECIMAL64); // no traps, please
  dc.round=set->round;                        // use supplied rounding

  decNumberFromString(&dn, string, &dc);     // will round if needed

  decimal64FromNumber(result, &dn, &dc);
  if (dc.status!=0) {                        // something happened
    decContextSetStatus(set, dc.status);     // .. pass it on
    }
  return result;
  } // decimal64FromString

/* ------------------------------------------------------------------ */
/* decimal64IsCanonical -- test whether encoding is canonical         */
/*   d64 is the source decimal64                                      */
/*   returns 1 if the encoding of d64 is canonical, 0 otherwise       */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
uInt decimal64IsCanonical(const decimal64 *d64) {
  decNumber dn;                         // work
  decimal64 canon;                      // ..
  decContext dc;                        // ..
  decContextDefault(&dc, DEC_INIT_DECIMAL64);
  decimal64ToNumber(d64, &dn);
  decimal64FromNumber(&canon, &dn, &dc);// canon will now be canonical
  return memcmp(d64, &canon, DECIMAL64_Bytes)==0;
  } // decimal64IsCanonical

/* ------------------------------------------------------------------ */
/* decimal64Canonical -- copy an encoding, ensuring it is canonical   */
/*   d64 is the source decimal64                                      */
/*   result is the target (may be the same decimal64)                 */
/*   returns result                                                   */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
decimal64 * decimal64Canonical(decimal64 *result, const decimal64 *d64) {
  decNumber dn;                         // work
  decContext dc;                        // ..
  decContextDefault(&dc, DEC_INIT_DECIMAL64);
  decimal64ToNumber(d64, &dn);
  decimal64FromNumber(result, &dn, &dc);// result will now be canonical
  return result;
  } // decimal64Canonical

#if DECTRACE || DECCHECK
/* Macros for accessing decimal64 fields.  These assume the
   argument is a reference (pointer) to the decimal64 structure,
   and the decimal64 is in network byte order (big-endian) */
// Get sign
#define decimal64Sign(d)       ((unsigned)(d)->bytes[0]>>7)

// Get combination field
#define decimal64Comb(d)       (((d)->bytes[0] & 0x7c)>>2)

// Get exponent continuation [does not remove bias]
#define decimal64ExpCon(d)     ((((d)->bytes[0] & 0x03)<<6)           \
                             | ((unsigned)(d)->bytes[1]>>2))

// Set sign [this assumes sign previously 0]
#define decimal64SetSign(d, b) {                                      \
  (d)->bytes[0]|=((unsigned)(b)<<7);}

// Set exponent continuation [does not apply bias]
// This assumes range has been checked and exponent previously 0;
// type of exponent must be unsigned
#define decimal64SetExpCon(d, e) {                                    \
  (d)->bytes[0]|=(uByte)((e)>>6);                                     \
  (d)->bytes[1]|=(uByte)(((e)&0x3F)<<2);}

/* ------------------------------------------------------------------ */
/* decimal64Show -- display a decimal64 in hexadecimal [debug aid]    */
/*   d64 -- the number to show                                        */
/* ------------------------------------------------------------------ */
// Also shows sign/cob/expconfields extracted
void decimal64Show(const decimal64 *d64) {
  char buf[DECIMAL64_Bytes*2+1];
  Int i, j=0;

  if (DECLITEND) {
    for (i=0; i<DECIMAL64_Bytes; i++, j+=2) {
      sprintf(&buf[j], "%02x", d64->bytes[7-i]);
      }
    printf(" D64> %s [S:%d Cb:%02x Ec:%02x] LittleEndian\n", buf,
           d64->bytes[7]>>7, (d64->bytes[7]>>2)&0x1f,
           ((d64->bytes[7]&0x3)<<6)| (d64->bytes[6]>>2));
    }
   else { // big-endian
    for (i=0; i<DECIMAL64_Bytes; i++, j+=2) {
      sprintf(&buf[j], "%02x", d64->bytes[i]);
      }
    printf(" D64> %s [S:%d Cb:%02x Ec:%02x] BigEndian\n", buf,
           decimal64Sign(d64), decimal64Comb(d64), decimal64ExpCon(d64));
    }
  } // decimal64Show
#endif

/* ================================================================== */
/* Shared utility routines and tables                                 */
/* ================================================================== */
// define and include the conversion tables to use for shared code
#if DECDPUN==3
  #define DEC_DPD2BIN 1
#else
  #define DEC_DPD2BCD 1
#endif
#include "decDPD.h"           // lookup tables

// The maximum number of decNumberUnits needed for a working copy of
// the units array is the ceiling of digits/DECDPUN, where digits is
// the maximum number of digits in any of the formats for which this
// is used.  decimal128.h must not be included in this module, so, as
// a very special case, that number is defined as a literal here.
#define DECMAX754   34
#define DECMAXUNITS ((DECMAX754+DECDPUN-1)/DECDPUN)

/* ------------------------------------------------------------------ */
/* Combination field lookup tables (uInts to save measurable work)    */
/*                                                                    */
/*      COMBEXP - 2-bit most-significant-bits of exponent             */
/*                [11 if an Infinity or NaN]                          */
/*      COMBMSD - 4-bit most-significant-digit                        */
/*                [0=Infinity, 1=NaN if COMBEXP=11]                   */
/*                                                                    */
/* Both are indexed by the 5-bit combination field (0-31)             */
/* ------------------------------------------------------------------ */
const uInt COMBEXP[32]={0, 0, 0, 0, 0, 0, 0, 0,
                        1, 1, 1, 1, 1, 1, 1, 1,
                        2, 2, 2, 2, 2, 2, 2, 2,
                        0, 0, 1, 1, 2, 2, 3, 3};
const uInt COMBMSD[32]={0, 1, 2, 3, 4, 5, 6, 7,
                        0, 1, 2, 3, 4, 5, 6, 7,
                        0, 1, 2, 3, 4, 5, 6, 7,
                        8, 9, 8, 9, 8, 9, 0, 1};

/* ------------------------------------------------------------------ */
/* decDigitsToDPD -- pack coefficient into DPD form                   */
/*                                                                    */
/*   dn   is the source number (assumed valid, max DECMAX754 digits)  */
/*   targ is 1, 2, or 4-element uInt array, which the caller must     */
/*        have cleared to zeros                                       */
/*   shift is the number of 0 digits to add on the right (normally 0) */
/*                                                                    */
/* The coefficient must be known small enough to fit.  The full       */
/* coefficient is copied, including the leading 'odd' digit.  This    */
/* digit is retrieved and packed into the combination field by the    */
/* caller.                                                            */
/*                                                                    */
/* The target uInts are altered only as necessary to receive the      */
/* digits of the decNumber.  When more than one uInt is needed, they  */
/* are filled from left to right (that is, the uInt at offset 0 will  */
/* end up with the least-significant digits).                         */
/*                                                                    */
/* shift is used for 'fold-down' padding.                             */
/*                                                                    */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
#if DECDPUN<=4
// Constant multipliers for divide-by-power-of five using reciprocal
// multiply, after removing powers of 2 by shifting, and final shift
// of 17 [we only need up to **4]
static const uInt multies[]={131073, 26215, 5243, 1049, 210};
// QUOT10 -- macro to return the quotient of unit u divided by 10**n
#define QUOT10(u, n) ((((uInt)(u)>>(n))*multies[n])>>17)
#endif
void decDigitsToDPD(const decNumber *dn, uInt *targ, Int shift) {
  Int  cut;                   // work
  Int  n;                     // output bunch counter
  Int  digits=dn->digits;     // digit countdown
  uInt dpd;                   // densely packed decimal value
  uInt bin;                   // binary value 0-999
  uInt *uout=targ;            // -> current output uInt
  uInt  uoff=0;               // -> current output offset [from right]
  const Unit *inu=dn->lsu;    // -> current input unit
  Unit  uar[DECMAXUNITS];     // working copy of units, iff shifted
  #if DECDPUN!=3              // not fast path
    Unit in;                  // current unit
  #endif

  if (shift!=0) {             // shift towards most significant required
    // shift the units array to the left by pad digits and copy
    // [this code is a special case of decShiftToMost, which could
    // be used instead if exposed and the array were copied first]
    const Unit *source;                 // ..
    Unit  *target, *first;              // ..
    uInt  next=0;                       // work

    source=dn->lsu+D2U(digits)-1;       // where msu comes from
    target=uar+D2U(digits)-1+D2U(shift);// where upper part of first cut goes
    cut=DECDPUN-MSUDIGITS(shift);       // where to slice
    if (cut==0) {                       // unit-boundary case
      for (; source>=dn->lsu; source--, target--) *target=*source;
      }
     else {
      first=uar+D2U(digits+shift)-1;    // where msu will end up
      for (; source>=dn->lsu; source--, target--) {
        // split the source Unit and accumulate remainder for next
        #if DECDPUN<=4
          uInt quot=QUOT10(*source, cut);
          uInt rem=*source-quot*DECPOWERS[cut];
          next+=quot;
        #else
          uInt rem=*source%DECPOWERS[cut];
          next+=*source/DECPOWERS[cut];
        #endif
        if (target<=first) *target=(Unit)next; // write to target iff valid
        next=rem*DECPOWERS[DECDPUN-cut];       // save remainder for next Unit
        }
      } // shift-move
    // propagate remainder to one below and clear the rest
    for (; target>=uar; target--) {
      *target=(Unit)next;
      next=0;
      }
    digits+=shift;                 // add count (shift) of zeros added
    inu=uar;                       // use units in working array
    }

  /* now densely pack the coefficient into DPD declets */

  #if DECDPUN!=3                   // not fast path
    in=*inu;                       // current unit
    cut=0;                         // at lowest digit
    bin=0;                         // [keep compiler quiet]
  #endif

  for(n=0; digits>0; n++) {        // each output bunch
    #if DECDPUN==3                 // fast path, 3-at-a-time
      bin=*inu;                    // 3 digits ready for convert
      digits-=3;                   // [may go negative]
      inu++;                       // may need another

    #else                          // must collect digit-by-digit
      Unit dig;                    // current digit
      Int j;                       // digit-in-declet count
      for (j=0; j<3; j++) {
        #if DECDPUN<=4
          Unit temp=(Unit)((uInt)(in*6554)>>16);
          dig=(Unit)(in-X10(temp));
          in=temp;
        #else
          dig=in%10;
          in=in/10;
        #endif
        if (j==0) bin=dig;
         else if (j==1)  bin+=X10(dig);
         else /* j==2 */ bin+=X100(dig);
        digits--;
        if (digits==0) break;      // [also protects *inu below]
        cut++;
        if (cut==DECDPUN) {inu++; in=*inu; cut=0;}
        }
    #endif
    // here there are 3 digits in bin, or have used all input digits

    dpd=BIN2DPD[bin];

    // write declet to uInt array
    *uout|=dpd<<uoff;
    uoff+=10;
    if (uoff<32) continue;         // no uInt boundary cross
    uout++;
    uoff-=32;
    *uout|=dpd>>(10-uoff);         // collect top bits
    } // n declets
  return;
  } // decDigitsToDPD

/* ------------------------------------------------------------------ */
/* decDigitsFromDPD -- unpack a format's coefficient                  */
/*                                                                    */
/*   dn is the target number, with 7, 16, or 34-digit space.          */
/*   sour is a 1, 2, or 4-element uInt array containing only declets  */
/*   declets is the number of (right-aligned) declets in sour to      */
/*     be processed.  This may be 1 more than the obvious number in   */
/*     a format, as any top digit is prefixed to the coefficient      */
/*     continuation field.  It also may be as small as 1, as the      */
/*     caller may pre-process leading zero declets.                   */
/*                                                                    */
/* When doing the 'extra declet' case care is taken to avoid writing  */
/* extra digits when there are leading zeros, as these could overflow */
/* the units array when DECDPUN is not 3.                             */
/*                                                                    */
/* The target uInts are used only as necessary to process declets     */
/* declets into the decNumber.  When more than one uInt is needed,    */
/* they are used from left to right (that is, the uInt at offset 0    */
/* provides the least-significant digits).                            */
/*                                                                    */
/* dn->digits is set, but not the sign or exponent.                   */
/* No error is possible [the redundant 888 codes are allowed].        */
/* ------------------------------------------------------------------ */
void decDigitsFromDPD(decNumber *dn, const uInt *sour, Int declets) {

  uInt  dpd;                       // collector for 10 bits
  Int   n;                         // counter
  Unit  *uout=dn->lsu;             // -> current output unit
  Unit  *last=uout;                // will be unit containing msd
  const uInt *uin=sour;            // -> current input uInt
  uInt  uoff=0;                    // -> current input offset [from right]

  #if DECDPUN!=3
  uInt  bcd;                       // BCD result
  uInt  nibble;                    // work
  Unit  out=0;                     // accumulator
  Int   cut=0;                     // power of ten in current unit
  #endif
  #if DECDPUN>4
  uInt const *pow;                 // work
  #endif

  // Expand the densely-packed integer, right to left
  for (n=declets-1; n>=0; n--) {   // count down declets of 10 bits
    dpd=*uin>>uoff;
    uoff+=10;
    if (uoff>32) {                 // crossed uInt boundary
      uin++;
      uoff-=32;                    // [if using this code for wider, check this]
      dpd|=*uin<<(10-uoff);        // get waiting bits
      }
    dpd&=0x3ff;                    // clear uninteresting bits

  #if DECDPUN==3
    if (dpd==0) *uout=0;
     else {
      *uout=DPD2BIN[dpd];          // convert 10 bits to binary 0-999
      last=uout;                   // record most significant unit
      }
    uout++;
    } // n

  #else // DECDPUN!=3
    if (dpd==0) {                  // fastpath [e.g., leading zeros]
      // write out three 0 digits (nibbles); out may have digit(s)
      cut++;
      if (cut==DECDPUN) {*uout=out; if (out) {last=uout; out=0;} uout++; cut=0;}
      if (n==0) break;             // [as below, works even if MSD=0]
      cut++;
      if (cut==DECDPUN) {*uout=out; if (out) {last=uout; out=0;} uout++; cut=0;}
      cut++;
      if (cut==DECDPUN) {*uout=out; if (out) {last=uout; out=0;} uout++; cut=0;}
      continue;
      }

    bcd=DPD2BCD[dpd];              // convert 10 bits to 12 bits BCD

    // now accumulate the 3 BCD nibbles into units
    nibble=bcd & 0x00f;
    if (nibble) out=(Unit)(out+nibble*DECPOWERS[cut]);
    cut++;
    if (cut==DECDPUN) {*uout=out; if (out) {last=uout; out=0;} uout++; cut=0;}
    bcd>>=4;

    // if this is the last declet and the remaining nibbles in bcd
    // are 00 then process no more nibbles, because this could be
    // the 'odd' MSD declet and writing any more Units would then
    // overflow the unit array
    if (n==0 && !bcd) break;

    nibble=bcd & 0x00f;
    if (nibble) out=(Unit)(out+nibble*DECPOWERS[cut]);
    cut++;
    if (cut==DECDPUN) {*uout=out; if (out) {last=uout; out=0;} uout++; cut=0;}
    bcd>>=4;

    nibble=bcd & 0x00f;
    if (nibble) out=(Unit)(out+nibble*DECPOWERS[cut]);
    cut++;
    if (cut==DECDPUN) {*uout=out; if (out) {last=uout; out=0;} uout++; cut=0;}
    } // n
  if (cut!=0) {                         // some more left over
    *uout=out;                          // write out final unit
    if (out) last=uout;                 // and note if non-zero
    }
  #endif

  // here, last points to the most significant unit with digits;
  // inspect it to get the final digits count -- this is essentially
  // the same code as decGetDigits in decNumber.c
  dn->digits=(last-dn->lsu)*DECDPUN+1;  // floor of digits, plus
                                        // must be at least 1 digit
  #if DECDPUN>1
  if (*last<10) return;                 // common odd digit or 0
  dn->digits++;                         // must be 2 at least
  #if DECDPUN>2
  if (*last<100) return;                // 10-99
  dn->digits++;                         // must be 3 at least
  #if DECDPUN>3
  if (*last<1000) return;               // 100-999
  dn->digits++;                         // must be 4 at least
  #if DECDPUN>4
  for (pow=&DECPOWERS[4]; *last>=*pow; pow++) dn->digits++;
  #endif
  #endif
  #endif
  #endif
  return;
  } //decDigitsFromDPD
