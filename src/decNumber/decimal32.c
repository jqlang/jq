/* ------------------------------------------------------------------ */
/* Decimal 32-bit format module                                       */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2008.  All rights reserved.   */
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
/* This module comprises the routines for decimal32 format numbers.   */
/* Conversions are supplied to and from decNumber and String.         */
/*                                                                    */
/* This is used when decNumber provides operations, either for all    */
/* operations or as a proxy between decNumber and decSingle.          */
/*                                                                    */
/* Error handling is the same as decNumber (qv.).                     */
/* ------------------------------------------------------------------ */
#include <string.h>           // [for memset/memcpy]
#include <stdio.h>            // [for printf]

#define  DECNUMDIGITS  7      // make decNumbers with space for 7
#include "decNumber.h"        // base number library
#include "decNumberLocal.h"   // decNumber local types, etc.
#include "decimal32.h"        // our primary include

/* Utility tables and routines [in decimal64.c] */
// DPD2BIN and the reverse are renamed to prevent link-time conflict
// if decQuad is also built in the same executable
#define DPD2BIN DPD2BINx
#define BIN2DPD BIN2DPDx
extern const uInt   COMBEXP[32], COMBMSD[32];
extern const uShort DPD2BIN[1024];
extern const uShort BIN2DPD[1000];
extern const uByte  BIN2CHAR[4001];

extern void decDigitsToDPD(const decNumber *, uInt *, Int);
extern void decDigitsFromDPD(decNumber *, const uInt *, Int);

#if DECTRACE || DECCHECK
void decimal32Show(const decimal32 *);            // for debug
extern void decNumberShow(const decNumber *);     // ..
#endif

/* Useful macro */
// Clear a structure (e.g., a decNumber)
#define DEC_clear(d) memset(d, 0, sizeof(*d))

/* ------------------------------------------------------------------ */
/* decimal32FromNumber -- convert decNumber to decimal32              */
/*                                                                    */
/*   ds is the target decimal32                                       */
/*   dn is the source number (assumed valid)                          */
/*   set is the context, used only for reporting errors               */
/*                                                                    */
/* The set argument is used only for status reporting and for the     */
/* rounding mode (used if the coefficient is more than DECIMAL32_Pmax */
/* digits or an overflow is detected).  If the exponent is out of the */
/* valid range then Overflow or Underflow will be raised.             */
/* After Underflow a subnormal result is possible.                    */
/*                                                                    */
/* DEC_Clamped is set if the number has to be 'folded down' to fit,   */
/* by reducing its exponent and multiplying the coefficient by a      */
/* power of ten, or if the exponent on a zero had to be clamped.      */
/* ------------------------------------------------------------------ */
decimal32 * decimal32FromNumber(decimal32 *d32, const decNumber *dn,
                              decContext *set) {
  uInt status=0;                   // status accumulator
  Int ae;                          // adjusted exponent
  decNumber  dw;                   // work
  decContext dc;                   // ..
  uInt comb, exp;                  // ..
  uInt uiwork;                     // for macros
  uInt targ=0;                     // target 32-bit

  // If the number has too many digits, or the exponent could be
  // out of range then reduce the number under the appropriate
  // constraints.  This could push the number to Infinity or zero,
  // so this check and rounding must be done before generating the
  // decimal32]
  ae=dn->exponent+dn->digits-1;              // [0 if special]
  if (dn->digits>DECIMAL32_Pmax              // too many digits
   || ae>DECIMAL32_Emax                      // likely overflow
   || ae<DECIMAL32_Emin) {                   // likely underflow
    decContextDefault(&dc, DEC_INIT_DECIMAL32); // [no traps]
    dc.round=set->round;                     // use supplied rounding
    decNumberPlus(&dw, dn, &dc);             // (round and check)
    // [this changes -0 to 0, so enforce the sign...]
    dw.bits|=dn->bits&DECNEG;
    status=dc.status;                        // save status
    dn=&dw;                                  // use the work number
    } // maybe out of range

  if (dn->bits&DECSPECIAL) {                      // a special value
    if (dn->bits&DECINF) targ=DECIMAL_Inf<<24;
     else {                                       // sNaN or qNaN
      if ((*dn->lsu!=0 || dn->digits>1)           // non-zero coefficient
       && (dn->digits<DECIMAL32_Pmax)) {          // coefficient fits
        decDigitsToDPD(dn, &targ, 0);
        }
      if (dn->bits&DECNAN) targ|=DECIMAL_NaN<<24;
       else targ|=DECIMAL_sNaN<<24;
      } // a NaN
    } // special

   else { // is finite
    if (decNumberIsZero(dn)) {               // is a zero
      // set and clamp exponent
      if (dn->exponent<-DECIMAL32_Bias) {
        exp=0;                               // low clamp
        status|=DEC_Clamped;
        }
       else {
        exp=dn->exponent+DECIMAL32_Bias;     // bias exponent
        if (exp>DECIMAL32_Ehigh) {           // top clamp
          exp=DECIMAL32_Ehigh;
          status|=DEC_Clamped;
          }
        }
      comb=(exp>>3) & 0x18;             // msd=0, exp top 2 bits ..
      }
     else {                             // non-zero finite number
      uInt msd;                         // work
      Int pad=0;                        // coefficient pad digits

      // the dn is known to fit, but it may need to be padded
      exp=(uInt)(dn->exponent+DECIMAL32_Bias);    // bias exponent
      if (exp>DECIMAL32_Ehigh) {                  // fold-down case
        pad=exp-DECIMAL32_Ehigh;
        exp=DECIMAL32_Ehigh;                      // [to maximum]
        status|=DEC_Clamped;
        }

      // fastpath common case
      if (DECDPUN==3 && pad==0) {
        targ=BIN2DPD[dn->lsu[0]];
        if (dn->digits>3) targ|=(uInt)(BIN2DPD[dn->lsu[1]])<<10;
        msd=(dn->digits==7 ? dn->lsu[2] : 0);
        }
       else { // general case
        decDigitsToDPD(dn, &targ, pad);
        // save and clear the top digit
        msd=targ>>20;
        targ&=0x000fffff;
        }

      // create the combination field
      if (msd>=8) comb=0x18 | ((exp>>5) & 0x06) | (msd & 0x01);
             else comb=((exp>>3) & 0x18) | msd;
      }
    targ|=comb<<26;                // add combination field ..
    targ|=(exp&0x3f)<<20;          // .. and exponent continuation
    } // finite

  if (dn->bits&DECNEG) targ|=0x80000000;  // add sign bit

  // now write to storage; this is endian
  UBFROMUI(d32->bytes, targ);      // directly store the int

  if (status!=0) decContextSetStatus(set, status); // pass on status
  // decimal32Show(d32);
  return d32;
  } // decimal32FromNumber

/* ------------------------------------------------------------------ */
/* decimal32ToNumber -- convert decimal32 to decNumber                */
/*   d32 is the source decimal32                                      */
/*   dn is the target number, with appropriate space                  */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
decNumber * decimal32ToNumber(const decimal32 *d32, decNumber *dn) {
  uInt msd;                        // coefficient MSD
  uInt exp;                        // exponent top two bits
  uInt comb;                       // combination field
  uInt sour;                       // source 32-bit
  uInt uiwork;                     // for macros

  // load source from storage; this is endian
  sour=UBTOUI(d32->bytes);         // directly load the int

  comb=(sour>>26)&0x1f;            // combination field

  decNumberZero(dn);               // clean number
  if (sour&0x80000000) dn->bits=DECNEG; // set sign if negative

  msd=COMBMSD[comb];               // decode the combination field
  exp=COMBEXP[comb];               // ..

  if (exp==3) {                    // is a special
    if (msd==0) {
      dn->bits|=DECINF;
      return dn;                   // no coefficient needed
      }
    else if (sour&0x02000000) dn->bits|=DECSNAN;
    else dn->bits|=DECNAN;
    msd=0;                         // no top digit
    }
   else {                          // is a finite number
    dn->exponent=(exp<<6)+((sour>>20)&0x3f)-DECIMAL32_Bias; // unbiased
    }

  // get the coefficient
  sour&=0x000fffff;                // clean coefficient continuation
  if (msd) {                       // non-zero msd
    sour|=msd<<20;                 // prefix to coefficient
    decDigitsFromDPD(dn, &sour, 3); // process 3 declets
    return dn;
    }
  // msd=0
  if (!sour) return dn;            // easy: coefficient is 0
  if (sour&0x000ffc00)             // need 2 declets?
    decDigitsFromDPD(dn, &sour, 2); // process 2 declets
   else
    decDigitsFromDPD(dn, &sour, 1); // process 1 declet
  return dn;
  } // decimal32ToNumber

/* ------------------------------------------------------------------ */
/* to-scientific-string -- conversion to numeric string               */
/* to-engineering-string -- conversion to numeric string              */
/*                                                                    */
/*   decimal32ToString(d32, string);                                  */
/*   decimal32ToEngString(d32, string);                               */
/*                                                                    */
/*  d32 is the decimal32 format number to convert                     */
/*  string is the string where the result will be laid out            */
/*                                                                    */
/*  string must be at least 24 characters                             */
/*                                                                    */
/*  No error is possible, and no status can be set.                   */
/* ------------------------------------------------------------------ */
char * decimal32ToEngString(const decimal32 *d32, char *string){
  decNumber dn;                         // work
  decimal32ToNumber(d32, &dn);
  decNumberToEngString(&dn, string);
  return string;
  } // decimal32ToEngString

char * decimal32ToString(const decimal32 *d32, char *string){
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
  uInt sour;                       // source 32-bit

  // load source from storage; this is endian
  sour=UBTOUI(d32->bytes);         // directly load the int

  c=string;                        // where result will go
  if (((Int)sour)<0) *c++='-';     // handle sign

  comb=(sour>>26)&0x1f;            // combination field
  msd=COMBMSD[comb];               // decode the combination field
  exp=COMBEXP[comb];               // ..

  if (exp==3) {
    if (msd==0) {                  // infinity
      strcpy(c,   "Inf");
      strcpy(c+3, "inity");
      return string;               // easy
      }
    if (sour&0x02000000) *c++='s'; // sNaN
    strcpy(c, "NaN");              // complete word
    c+=3;                          // step past
    if ((sour&0x000fffff)==0) return string; // zero payload
    // otherwise drop through to add integer; set correct exp
    exp=0; msd=0;                  // setup for following code
    }
   else exp=(exp<<6)+((sour>>20)&0x3f)-DECIMAL32_Bias; // unbiased

  // convert 7 digits of significand to characters
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

  dpd=(sour>>10)&0x3ff;            // declet 1
  dpd2char;
  dpd=(sour)&0x3ff;                // declet 2
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
    // a maximum length of 3 digits (E-101 case)
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
  } // decimal32ToString

/* ------------------------------------------------------------------ */
/* to-number -- conversion from numeric string                        */
/*                                                                    */
/*   decimal32FromString(result, string, set);                        */
/*                                                                    */
/*  result  is the decimal32 format number which gets the result of   */
/*          the conversion                                            */
/*  *string is the character string which should contain a valid      */
/*          number (which may be a special value)                     */
/*  set     is the context                                            */
/*                                                                    */
/* The context is supplied to this routine is used for error handling */
/* (setting of status and traps) and for the rounding mode, only.     */
/* If an error occurs, the result will be a valid decimal32 NaN.      */
/* ------------------------------------------------------------------ */
decimal32 * decimal32FromString(decimal32 *result, const char *string,
                                decContext *set) {
  decContext dc;                             // work
  decNumber dn;                              // ..

  decContextDefault(&dc, DEC_INIT_DECIMAL32); // no traps, please
  dc.round=set->round;                        // use supplied rounding

  decNumberFromString(&dn, string, &dc);     // will round if needed
  decimal32FromNumber(result, &dn, &dc);
  if (dc.status!=0) {                        // something happened
    decContextSetStatus(set, dc.status);     // .. pass it on
    }
  return result;
  } // decimal32FromString

/* ------------------------------------------------------------------ */
/* decimal32IsCanonical -- test whether encoding is canonical         */
/*   d32 is the source decimal32                                      */
/*   returns 1 if the encoding of d32 is canonical, 0 otherwise       */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
uInt decimal32IsCanonical(const decimal32 *d32) {
  decNumber dn;                         // work
  decimal32 canon;                      // ..
  decContext dc;                        // ..
  decContextDefault(&dc, DEC_INIT_DECIMAL32);
  decimal32ToNumber(d32, &dn);
  decimal32FromNumber(&canon, &dn, &dc);// canon will now be canonical
  return memcmp(d32, &canon, DECIMAL32_Bytes)==0;
  } // decimal32IsCanonical

/* ------------------------------------------------------------------ */
/* decimal32Canonical -- copy an encoding, ensuring it is canonical   */
/*   d32 is the source decimal32                                      */
/*   result is the target (may be the same decimal32)                 */
/*   returns result                                                   */
/* No error is possible.                                              */
/* ------------------------------------------------------------------ */
decimal32 * decimal32Canonical(decimal32 *result, const decimal32 *d32) {
  decNumber dn;                         // work
  decContext dc;                        // ..
  decContextDefault(&dc, DEC_INIT_DECIMAL32);
  decimal32ToNumber(d32, &dn);
  decimal32FromNumber(result, &dn, &dc);// result will now be canonical
  return result;
  } // decimal32Canonical

#if DECTRACE || DECCHECK
/* Macros for accessing decimal32 fields.  These assume the argument
   is a reference (pointer) to the decimal32 structure, and the
   decimal32 is in network byte order (big-endian) */
// Get sign
#define decimal32Sign(d)       ((unsigned)(d)->bytes[0]>>7)

// Get combination field
#define decimal32Comb(d)       (((d)->bytes[0] & 0x7c)>>2)

// Get exponent continuation [does not remove bias]
#define decimal32ExpCon(d)     ((((d)->bytes[0] & 0x03)<<4)           \
                             | ((unsigned)(d)->bytes[1]>>4))

// Set sign [this assumes sign previously 0]
#define decimal32SetSign(d, b) {                                      \
  (d)->bytes[0]|=((unsigned)(b)<<7);}

// Set exponent continuation [does not apply bias]
// This assumes range has been checked and exponent previously 0;
// type of exponent must be unsigned
#define decimal32SetExpCon(d, e) {                                    \
  (d)->bytes[0]|=(uByte)((e)>>4);                                     \
  (d)->bytes[1]|=(uByte)(((e)&0x0F)<<4);}

/* ------------------------------------------------------------------ */
/* decimal32Show -- display a decimal32 in hexadecimal [debug aid]    */
/*   d32 -- the number to show                                        */
/* ------------------------------------------------------------------ */
// Also shows sign/cob/expconfields extracted - valid bigendian only
void decimal32Show(const decimal32 *d32) {
  char buf[DECIMAL32_Bytes*2+1];
  Int i, j=0;

  if (DECLITEND) {
    for (i=0; i<DECIMAL32_Bytes; i++, j+=2) {
      sprintf(&buf[j], "%02x", d32->bytes[3-i]);
      }
    printf(" D32> %s [S:%d Cb:%02x Ec:%02x] LittleEndian\n", buf,
           d32->bytes[3]>>7, (d32->bytes[3]>>2)&0x1f,
           ((d32->bytes[3]&0x3)<<4)| (d32->bytes[2]>>4));
    }
   else {
    for (i=0; i<DECIMAL32_Bytes; i++, j+=2) {
      sprintf(&buf[j], "%02x", d32->bytes[i]);
      }
    printf(" D32> %s [S:%d Cb:%02x Ec:%02x] BigEndian\n", buf,
           decimal32Sign(d32), decimal32Comb(d32), decimal32ExpCon(d32));
    }
  } // decimal32Show
#endif
