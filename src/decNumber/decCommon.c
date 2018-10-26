/* ------------------------------------------------------------------ */
/* decCommon.c -- common code for all three fixed-size types          */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2010.  All rights reserved.   */
/*                                                                    */
/* This software is made available under the terms of the             */
/* ICU License -- ICU 1.8.1 and later.                                */
/*                                                                    */
/* The description and User's Guide ("The decNumber C Library") for   */
/* this software is included in the package as decNumber.pdf.  This   */
/* document is also available in HTML, together with specifications,  */
/* testcases, and Web links, on the General Decimal Arithmetic page.  */
/*                                                                    */
/* Please send comments, suggestions, and corrections to the author:  */
/*   mfc@uk.ibm.com                                                   */
/*   Mike Cowlishaw, IBM Fellow                                       */
/*   IBM UK, PO Box 31, Birmingham Road, Warwick CV34 5JL, UK         */
/* ------------------------------------------------------------------ */
/* This module comprises code that is shared between all the formats  */
/* (decSingle, decDouble, and decQuad); it includes set and extract   */
/* of format components, widening, narrowing, and string conversions. */
/*                                                                    */
/* Unlike decNumber, parameterization takes place at compile time     */
/* rather than at runtime.  The parameters are set in the decDouble.c */
/* (etc.) files, which then include this one to produce the compiled  */
/* code.  The functions here, therefore, are code shared between      */
/* multiple formats.                                                  */
/* ------------------------------------------------------------------ */
// Names here refer to decFloat rather than to decDouble, etc., and
// the functions are in strict alphabetical order.
// Constants, tables, and debug function(s) are included only for QUAD
// (which will always be compiled if DOUBLE or SINGLE are used).
//
// Whenever a decContext is used, only the status may be set (using
// OR) or the rounding mode read; all other fields are ignored and
// untouched.

// names for simpler testing and default context
#if DECPMAX==7
  #define SINGLE     1
  #define DOUBLE     0
  #define QUAD       0
  #define DEFCONTEXT DEC_INIT_DECIMAL32
#elif DECPMAX==16
  #define SINGLE     0
  #define DOUBLE     1
  #define QUAD       0
  #define DEFCONTEXT DEC_INIT_DECIMAL64
#elif DECPMAX==34
  #define SINGLE     0
  #define DOUBLE     0
  #define QUAD       1
  #define DEFCONTEXT DEC_INIT_DECIMAL128
#else
  #error Unexpected DECPMAX value
#endif

/* Assertions */

#if DECPMAX!=7 && DECPMAX!=16 && DECPMAX!=34
  #error Unexpected Pmax (DECPMAX) value for this module
#endif

// Assert facts about digit characters, etc.
#if ('9'&0x0f)!=9
  #error This module assumes characters are of the form 0b....nnnn
  // where .... are don't care 4 bits and nnnn is 0000 through 1001
#endif
#if ('9'&0xf0)==('.'&0xf0)
  #error This module assumes '.' has a different mask than a digit
#endif

// Assert ToString lay-out conditions
#if DECSTRING<DECPMAX+9
  #error ToString needs at least 8 characters for lead-in and dot
#endif
#if DECPMAX+DECEMAXD+5 > DECSTRING
  #error Exponent form can be too long for ToString to lay out safely
#endif
#if DECEMAXD > 4
  #error Exponent form is too long for ToString to lay out
  // Note: code for up to 9 digits exists in archives [decOct]
#endif

/* Private functions used here and possibly in decBasic.c, etc. */
static decFloat * decFinalize(decFloat *, bcdnum *, decContext *);
static Flag decBiStr(const char *, const char *, const char *);

/* Macros and private tables; those which are not format-dependent    */
/* are only included if decQuad is being built.                       */

/* ------------------------------------------------------------------ */
/* Combination field lookup tables (uInts to save measurable work)    */
/*                                                                    */
/*   DECCOMBEXP  - 2 most-significant-bits of exponent (00, 01, or    */
/*                 10), shifted left for format, or DECFLOAT_Inf/NaN  */
/*   DECCOMBWEXP - The same, for the next-wider format (unless QUAD)  */
/*   DECCOMBMSD  - 4-bit most-significant-digit                       */
/*                 [0 if the index is a special (Infinity or NaN)]    */
/*   DECCOMBFROM - 5-bit combination field from EXP top bits and MSD  */
/*                 (placed in uInt so no shift is needed)             */
/*                                                                    */
/* DECCOMBEXP, DECCOMBWEXP, and DECCOMBMSD are indexed by the sign    */
/*   and 5-bit combination field (0-63, the second half of the table  */
/*   identical to the first half)                                     */
/* DECCOMBFROM is indexed by expTopTwoBits*16 + msd                   */
/*                                                                    */
/* DECCOMBMSD and DECCOMBFROM are not format-dependent and so are     */
/* only included once, when QUAD is being built                       */
/* ------------------------------------------------------------------ */
static const uInt DECCOMBEXP[64]={
  0, 0, 0, 0, 0, 0, 0, 0,
  1<<DECECONL, 1<<DECECONL, 1<<DECECONL, 1<<DECECONL,
  1<<DECECONL, 1<<DECECONL, 1<<DECECONL, 1<<DECECONL,
  2<<DECECONL, 2<<DECECONL, 2<<DECECONL, 2<<DECECONL,
  2<<DECECONL, 2<<DECECONL, 2<<DECECONL, 2<<DECECONL,
  0,           0,           1<<DECECONL, 1<<DECECONL,
  2<<DECECONL, 2<<DECECONL, DECFLOAT_Inf, DECFLOAT_NaN,
  0, 0, 0, 0, 0, 0, 0, 0,
  1<<DECECONL, 1<<DECECONL, 1<<DECECONL, 1<<DECECONL,
  1<<DECECONL, 1<<DECECONL, 1<<DECECONL, 1<<DECECONL,
  2<<DECECONL, 2<<DECECONL, 2<<DECECONL, 2<<DECECONL,
  2<<DECECONL, 2<<DECECONL, 2<<DECECONL, 2<<DECECONL,
  0,           0,           1<<DECECONL, 1<<DECECONL,
  2<<DECECONL, 2<<DECECONL, DECFLOAT_Inf, DECFLOAT_NaN};
#if !QUAD
static const uInt DECCOMBWEXP[64]={
  0, 0, 0, 0, 0, 0, 0, 0,
  1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL,
  1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL,
  2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL,
  2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL,
  0,            0,            1<<DECWECONL, 1<<DECWECONL,
  2<<DECWECONL, 2<<DECWECONL, DECFLOAT_Inf, DECFLOAT_NaN,
  0, 0, 0, 0, 0, 0, 0, 0,
  1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL,
  1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL, 1<<DECWECONL,
  2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL,
  2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL, 2<<DECWECONL,
  0,            0,            1<<DECWECONL, 1<<DECWECONL,
  2<<DECWECONL, 2<<DECWECONL, DECFLOAT_Inf, DECFLOAT_NaN};
#endif

#if QUAD
const uInt DECCOMBMSD[64]={
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 9, 8, 9, 0, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 9, 8, 9, 0, 0};

const uInt DECCOMBFROM[48]={
  0x00000000, 0x04000000, 0x08000000, 0x0C000000, 0x10000000, 0x14000000,
  0x18000000, 0x1C000000, 0x60000000, 0x64000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x20000000, 0x24000000,
  0x28000000, 0x2C000000, 0x30000000, 0x34000000, 0x38000000, 0x3C000000,
  0x68000000, 0x6C000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x40000000, 0x44000000, 0x48000000, 0x4C000000,
  0x50000000, 0x54000000, 0x58000000, 0x5C000000, 0x70000000, 0x74000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

/* ------------------------------------------------------------------ */
/* Request and include the tables to use for conversions              */
/* ------------------------------------------------------------------ */
#define DEC_BCD2DPD  1        // 0-0x999 -> DPD
#define DEC_BIN2DPD  1        // 0-999 -> DPD
#define DEC_BIN2BCD8 1        // 0-999 -> ddd, len
#define DEC_DPD2BCD8 1        // DPD -> ddd, len
#define DEC_DPD2BIN  1        // DPD -> 0-999
#define DEC_DPD2BINK 1        // DPD -> 0-999000
#define DEC_DPD2BINM 1        // DPD -> 0-999000000
#include "decDPD.h"           // source of the lookup tables

#endif

/* ----------------------------------------------------------------- */
/* decBiStr -- compare string with pairwise options                  */
/*                                                                   */
/*   targ is the string to compare                                   */
/*   str1 is one of the strings to compare against (length may be 0) */
/*   str2 is the other; it must be the same length as str1           */
/*                                                                   */
/*   returns 1 if strings compare equal, (that is, targ is the same  */
/*   length as str1 and str2, and each character of targ is in one   */
/*   of str1 or str2 in the corresponding position), or 0 otherwise  */
/*                                                                   */
/* This is used for generic caseless compare, including the awkward  */
/* case of the Turkish dotted and dotless Is.  Use as (for example): */
/*   if (decBiStr(test, "mike", "MIKE")) ...                         */
/* ----------------------------------------------------------------- */
static Flag decBiStr(const char *targ, const char *str1, const char *str2) {
  for (;;targ++, str1++, str2++) {
    if (*targ!=*str1 && *targ!=*str2) return 0;
    // *targ has a match in one (or both, if terminator)
    if (*targ=='\0') break;
    } // forever
  return 1;
  } // decBiStr

/* ------------------------------------------------------------------ */
/* decFinalize -- adjust and store a final result                     */
/*                                                                    */
/*  df  is the decFloat format number which gets the final result     */
/*  num is the descriptor of the number to be checked and encoded     */
/*         [its values, including the coefficient, may be modified]   */
/*  set is the context to use                                         */
/*  returns df                                                        */
/*                                                                    */
/* The num descriptor may point to a bcd8 string of any length; this  */
/* string may have leading insignificant zeros.  If it has more than  */
/* DECPMAX digits then the final digit can be a round-for-reround     */
/* digit (i.e., it may include a sticky bit residue).                 */
/*                                                                    */
/* The exponent (q) may be one of the codes for a special value and   */
/* can be up to 999999999 for conversion from string.                 */
/*                                                                    */
/* No error is possible, but Inexact, Underflow, and/or Overflow may  */
/* be set.                                                            */
/* ------------------------------------------------------------------ */
// Constant whose size varies with format; also the check for surprises
static uByte allnines[DECPMAX]=
#if SINGLE
  {9, 9, 9, 9, 9, 9, 9};
#elif DOUBLE
  {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
#elif QUAD
  {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
#endif

static decFloat * decFinalize(decFloat *df, bcdnum *num,
                              decContext *set) {
  uByte *ub;                  // work
  uInt   dpd;                 // ..
  uInt   uiwork;              // for macros
  uByte *umsd=num->msd;       // local copy
  uByte *ulsd=num->lsd;       // ..
  uInt   encode;              // encoding accumulator
  Int    length;              // coefficient length

  #if DECCHECK
  Int clen=ulsd-umsd+1;
  #if QUAD
    #define COEXTRA 2                        // extra-long coefficent
  #else
    #define COEXTRA 0
  #endif
  if (clen<1 || clen>DECPMAX*3+2+COEXTRA)
    printf("decFinalize: suspect coefficient [length=%ld]\n", (LI)clen);
  if (num->sign!=0 && num->sign!=DECFLOAT_Sign)
    printf("decFinalize: bad sign [%08lx]\n", (LI)num->sign);
  if (!EXPISSPECIAL(num->exponent)
      && (num->exponent>1999999999 || num->exponent<-1999999999))
    printf("decFinalize: improbable exponent [%ld]\n", (LI)num->exponent);
  // decShowNum(num, "final");
  #endif

  // A special will have an 'exponent' which is very positive and a
  // coefficient < DECPMAX
  length=(uInt)(ulsd-umsd+1);                // coefficient length

  if (!NUMISSPECIAL(num)) {
    Int   drop;                              // digits to be dropped
    // skip leading insignificant zeros to calculate an exact length
    // [this is quite expensive]
    if (*umsd==0) {
      for (; umsd+3<ulsd && UBTOUI(umsd)==0;) umsd+=4;
      for (; *umsd==0 && umsd<ulsd;) umsd++;
      length=ulsd-umsd+1;                    // recalculate
      }
    drop=MAXI(length-DECPMAX, DECQTINY-num->exponent);
    // drop can now be > digits for bottom-clamp (subnormal) cases
    if (drop>0) {                            // rounding needed
      // (decFloatQuantize has very similar code to this, so any
      // changes may need to be made there, too)
      uByte *roundat;                        // -> re-round digit
      uByte reround;                         // reround value
      // printf("Rounding; drop=%ld\n", (LI)drop);

      num->exponent+=drop;                   // always update exponent

      // Three cases here:
      //   1. new LSD is in coefficient (almost always)
      //   2. new LSD is digit to left of coefficient (so MSD is
      //      round-for-reround digit)
      //   3. new LSD is to left of case 2 (whole coefficient is sticky)
      // [duplicate check-stickies code to save a test]
      // [by-digit check for stickies as runs of zeros are rare]
      if (drop<length) {                     // NB lengths not addresses
        roundat=umsd+length-drop;
        reround=*roundat;
        for (ub=roundat+1; ub<=ulsd; ub++) {
          if (*ub!=0) {                      // non-zero to be discarded
            reround=DECSTICKYTAB[reround];   // apply sticky bit
            break;                           // [remainder don't-care]
            }
          } // check stickies
        ulsd=roundat-1;                      // new LSD
        }
       else {                                // edge case
        if (drop==length) {
          roundat=umsd;
          reround=*roundat;
          }
         else {
          roundat=umsd-1;
          reround=0;
          }
        for (ub=roundat+1; ub<=ulsd; ub++) {
          if (*ub!=0) {                      // non-zero to be discarded
            reround=DECSTICKYTAB[reround];   // apply sticky bit
            break;                           // [remainder don't-care]
            }
          } // check stickies
        *umsd=0;                             // coefficient is a 0
        ulsd=umsd;                           // ..
        }

      if (reround!=0) {                      // discarding non-zero
        uInt bump=0;
        set->status|=DEC_Inexact;
        // if adjusted exponent [exp+digits-1] is < EMIN then num is
        // subnormal -- so raise Underflow
        if (num->exponent<DECEMIN && (num->exponent+(ulsd-umsd+1)-1)<DECEMIN)
          set->status|=DEC_Underflow;

        // next decide whether increment of the coefficient is needed
        if (set->round==DEC_ROUND_HALF_EVEN) {    // fastpath slowest case
          if (reround>5) bump=1;                  // >0.5 goes up
           else if (reround==5)                   // exactly 0.5000 ..
            bump=*ulsd & 0x01;                    // .. up iff [new] lsd is odd
          } // r-h-e
         else switch (set->round) {
          case DEC_ROUND_DOWN: {
            // no change
            break;} // r-d
          case DEC_ROUND_HALF_DOWN: {
            if (reround>5) bump=1;
            break;} // r-h-d
          case DEC_ROUND_HALF_UP: {
            if (reround>=5) bump=1;
            break;} // r-h-u
          case DEC_ROUND_UP: {
            if (reround>0) bump=1;
            break;} // r-u
          case DEC_ROUND_CEILING: {
            // same as _UP for positive numbers, and as _DOWN for negatives
            if (!num->sign && reround>0) bump=1;
            break;} // r-c
          case DEC_ROUND_FLOOR: {
            // same as _UP for negative numbers, and as _DOWN for positive
            // [negative reround cannot occur on 0]
            if (num->sign && reround>0) bump=1;
            break;} // r-f
          case DEC_ROUND_05UP: {
            if (reround>0) { // anything out there is 'sticky'
              // bump iff lsd=0 or 5; this cannot carry so it could be
              // effected immediately with no bump -- but the code
              // is clearer if this is done the same way as the others
              if (*ulsd==0 || *ulsd==5) bump=1;
              }
            break;} // r-r
          default: {      // e.g., DEC_ROUND_MAX
            set->status|=DEC_Invalid_context;
            #if DECCHECK
            printf("Unknown rounding mode: %ld\n", (LI)set->round);
            #endif
            break;}
          } // switch (not r-h-e)
        // printf("ReRound: %ld  bump: %ld\n", (LI)reround, (LI)bump);

        if (bump!=0) {                       // need increment
          // increment the coefficient; this might end up with 1000...
          // (after the all nines case)
          ub=ulsd;
          for(; ub-3>=umsd && UBTOUI(ub-3)==0x09090909; ub-=4)  {
            UBFROMUI(ub-3, 0);               // to 00000000
            }
          // [note ub could now be to left of msd, and it is not safe
          // to write to the the left of the msd]
          // now at most 3 digits left to non-9 (usually just the one)
          for (; ub>=umsd; *ub=0, ub--) {
            if (*ub==9) continue;            // carry
            *ub+=1;
            break;
            }
          if (ub<umsd) {                     // had all-nines
            *umsd=1;                         // coefficient to 1000...
            // usually the 1000... coefficient can be used as-is
            if ((ulsd-umsd+1)==DECPMAX) {
              num->exponent++;
              }
             else {
              // if coefficient is shorter than Pmax then num is
              // subnormal, so extend it; this is safe as drop>0
              // (or, if the coefficient was supplied above, it could
              // not be 9); this may make the result normal.
              ulsd++;
              *ulsd=0;
              // [exponent unchanged]
              #if DECCHECK
              if (num->exponent!=DECQTINY) // sanity check
                printf("decFinalize: bad all-nines extend [^%ld, %ld]\n",
                       (LI)num->exponent, (LI)(ulsd-umsd+1));
              #endif
              } // subnormal extend
            } // had all-nines
          } // bump needed
        } // inexact rounding

      length=ulsd-umsd+1;               // recalculate (may be <DECPMAX)
      } // need round (drop>0)

    // The coefficient will now fit and has final length unless overflow
    // decShowNum(num, "rounded");

    // if exponent is >=emax may have to clamp, overflow, or fold-down
    if (num->exponent>DECEMAX-(DECPMAX-1)) { // is edge case
      // printf("overflow checks...\n");
      if (*ulsd==0 && ulsd==umsd) {     // have zero
        num->exponent=DECEMAX-(DECPMAX-1); // clamp to max
        }
       else if ((num->exponent+length-1)>DECEMAX) { // > Nmax
        // Overflow -- these could go straight to encoding, here, but
        // instead num is adjusted to keep the code cleaner
        Flag needmax=0;                 // 1 for finite result
        set->status|=(DEC_Overflow | DEC_Inexact);
        switch (set->round) {
          case DEC_ROUND_DOWN: {
            needmax=1;                  // never Infinity
            break;} // r-d
          case DEC_ROUND_05UP: {
            needmax=1;                  // never Infinity
            break;} // r-05
          case DEC_ROUND_CEILING: {
            if (num->sign) needmax=1;   // Infinity iff non-negative
            break;} // r-c
          case DEC_ROUND_FLOOR: {
            if (!num->sign) needmax=1;  // Infinity iff negative
            break;} // r-f
          default: break;               // Infinity in all other cases
          }
        if (!needmax) {                 // easy .. set Infinity
          num->exponent=DECFLOAT_Inf;
          *umsd=0;                      // be clean: coefficient to 0
          ulsd=umsd;                    // ..
          }
         else {                         // return Nmax
          umsd=allnines;                // use constant array
          ulsd=allnines+DECPMAX-1;
          num->exponent=DECEMAX-(DECPMAX-1);
          }
        }
       else { // no overflow but non-zero and may have to fold-down
        Int shift=num->exponent-(DECEMAX-(DECPMAX-1));
        if (shift>0) {                  // fold-down needed
          // fold down needed; must copy to buffer in order to pad
          // with zeros safely; fortunately this is not the worst case
          // path because cannot have had a round
          uByte buffer[ROUNDUP(DECPMAX+3, 4)]; // [+3 allows uInt padding]
          uByte *s=umsd;                // source
          uByte *t=buffer;              // safe target
          uByte *tlsd=buffer+(ulsd-umsd)+shift; // target LSD
          // printf("folddown shift=%ld\n", (LI)shift);
          for (; s<=ulsd; s+=4, t+=4) UBFROMUI(t, UBTOUI(s));
          for (t=tlsd-shift+1; t<=tlsd; t+=4) UBFROMUI(t, 0);  // pad 0s
          num->exponent-=shift;
          umsd=buffer;
          ulsd=tlsd;
          }
        } // fold-down?
      length=ulsd-umsd+1;               // recalculate length
      } // high-end edge case
    } // finite number

  /*------------------------------------------------------------------*/
  /* At this point the result will properly fit the decFloat          */
  /* encoding, and it can be encoded with no possibility of error     */
  /*------------------------------------------------------------------*/
  // Following code does not alter coefficient (could be allnines array)

  // fast path possible when DECPMAX digits
  if (length==DECPMAX) {
    return decFloatFromBCD(df, num->exponent, umsd, num->sign);
    } // full-length

  // slower path when not a full-length number; must care about length
  // [coefficient length here will be < DECPMAX]
  if (!NUMISSPECIAL(num)) {             // is still finite
    // encode the combination field and exponent continuation
    uInt uexp=(uInt)(num->exponent+DECBIAS); // biased exponent
    uInt code=(uexp>>DECECONL)<<4;      // top two bits of exp
    // [msd==0]
    // look up the combination field and make high word
    encode=DECCOMBFROM[code];           // indexed by (0-2)*16+msd
    encode|=(uexp<<(32-6-DECECONL)) & 0x03ffffff; // exponent continuation
    }
   else encode=num->exponent;           // special [already in word]
  encode|=num->sign;                    // add sign

  // private macro to extract a declet, n (where 0<=n<DECLETS and 0
  // refers to the declet from the least significant three digits)
  // and put the corresponding DPD code into dpd.  Access to umsd and
  // ulsd (pointers to the most and least significant digit of the
  // variable-length coefficient) is assumed, along with use of a
  // working pointer, uInt *ub.
  // As not full-length then chances are there are many leading zeros
  // [and there may be a partial triad]
  #define getDPDt(dpd, n) ub=ulsd-(3*(n))-2;                          \
    if (ub<umsd-2) dpd=0;                                             \
     else if (ub>=umsd) dpd=BCD2DPD[(*ub*256)+(*(ub+1)*16)+*(ub+2)];  \
     else {dpd=*(ub+2); if (ub+1==umsd) dpd+=*(ub+1)*16; dpd=BCD2DPD[dpd];}

  // place the declets in the encoding words and copy to result (df),
  // according to endianness; in all cases complete the sign word
  // first
  #if DECPMAX==7
    getDPDt(dpd, 1);
    encode|=dpd<<10;
    getDPDt(dpd, 0);
    encode|=dpd;
    DFWORD(df, 0)=encode;     // just the one word

  #elif DECPMAX==16
    getDPDt(dpd, 4); encode|=dpd<<8;
    getDPDt(dpd, 3); encode|=dpd>>2;
    DFWORD(df, 0)=encode;
    encode=dpd<<30;
    getDPDt(dpd, 2); encode|=dpd<<20;
    getDPDt(dpd, 1); encode|=dpd<<10;
    getDPDt(dpd, 0); encode|=dpd;
    DFWORD(df, 1)=encode;

  #elif DECPMAX==34
    getDPDt(dpd,10); encode|=dpd<<4;
    getDPDt(dpd, 9); encode|=dpd>>6;
    DFWORD(df, 0)=encode;

    encode=dpd<<26;
    getDPDt(dpd, 8); encode|=dpd<<16;
    getDPDt(dpd, 7); encode|=dpd<<6;
    getDPDt(dpd, 6); encode|=dpd>>4;
    DFWORD(df, 1)=encode;

    encode=dpd<<28;
    getDPDt(dpd, 5); encode|=dpd<<18;
    getDPDt(dpd, 4); encode|=dpd<<8;
    getDPDt(dpd, 3); encode|=dpd>>2;
    DFWORD(df, 2)=encode;

    encode=dpd<<30;
    getDPDt(dpd, 2); encode|=dpd<<20;
    getDPDt(dpd, 1); encode|=dpd<<10;
    getDPDt(dpd, 0); encode|=dpd;
    DFWORD(df, 3)=encode;
  #endif

  // printf("Status: %08lx\n", (LI)set->status);
  // decFloatShow(df, "final2");
  return df;
  } // decFinalize

/* ------------------------------------------------------------------ */
/* decFloatFromBCD -- set decFloat from exponent, BCD8, and sign      */
/*                                                                    */
/*  df is the target decFloat                                         */
/*  exp is the in-range unbiased exponent, q, or a special value in   */
/*    the form returned by decFloatGetExponent                        */
/*  bcdar holds DECPMAX digits to set the coefficient from, one       */
/*    digit in each byte (BCD8 encoding); the first (MSD) is ignored  */
/*    if df is a NaN; all are ignored if df is infinite.              */
/*    All bytes must be in 0-9; results are undefined otherwise.      */
/*  sig is DECFLOAT_Sign to set the sign bit, 0 otherwise             */
/*  returns df, which will be canonical                               */
/*                                                                    */
/* No error is possible, and no status will be set.                   */
/* ------------------------------------------------------------------ */
decFloat * decFloatFromBCD(decFloat *df, Int exp, const uByte *bcdar,
                           Int sig) {
  uInt encode, dpd;                     // work
  const uByte *ub;                      // ..

  if (EXPISSPECIAL(exp)) encode=exp|sig;// specials already encoded
   else {                               // is finite
    // encode the combination field and exponent continuation
    uInt uexp=(uInt)(exp+DECBIAS);      // biased exponent
    uInt code=(uexp>>DECECONL)<<4;      // top two bits of exp
    code+=bcdar[0];                     // add msd
    // look up the combination field and make high word
    encode=DECCOMBFROM[code]|sig;       // indexed by (0-2)*16+msd
    encode|=(uexp<<(32-6-DECECONL)) & 0x03ffffff; // exponent continuation
    }

  // private macro to extract a declet, n (where 0<=n<DECLETS and 0
  // refers to the declet from the least significant three digits)
  // and put the corresponding DPD code into dpd.
  // Use of a working pointer, uInt *ub, is assumed.

  #define getDPDb(dpd, n) ub=bcdar+DECPMAX-1-(3*(n))-2;     \
    dpd=BCD2DPD[(*ub*256)+(*(ub+1)*16)+*(ub+2)];

  // place the declets in the encoding words and copy to result (df),
  // according to endianness; in all cases complete the sign word
  // first
  #if DECPMAX==7
    getDPDb(dpd, 1);
    encode|=dpd<<10;
    getDPDb(dpd, 0);
    encode|=dpd;
    DFWORD(df, 0)=encode;     // just the one word

  #elif DECPMAX==16
    getDPDb(dpd, 4); encode|=dpd<<8;
    getDPDb(dpd, 3); encode|=dpd>>2;
    DFWORD(df, 0)=encode;
    encode=dpd<<30;
    getDPDb(dpd, 2); encode|=dpd<<20;
    getDPDb(dpd, 1); encode|=dpd<<10;
    getDPDb(dpd, 0); encode|=dpd;
    DFWORD(df, 1)=encode;

  #elif DECPMAX==34
    getDPDb(dpd,10); encode|=dpd<<4;
    getDPDb(dpd, 9); encode|=dpd>>6;
    DFWORD(df, 0)=encode;

    encode=dpd<<26;
    getDPDb(dpd, 8); encode|=dpd<<16;
    getDPDb(dpd, 7); encode|=dpd<<6;
    getDPDb(dpd, 6); encode|=dpd>>4;
    DFWORD(df, 1)=encode;

    encode=dpd<<28;
    getDPDb(dpd, 5); encode|=dpd<<18;
    getDPDb(dpd, 4); encode|=dpd<<8;
    getDPDb(dpd, 3); encode|=dpd>>2;
    DFWORD(df, 2)=encode;

    encode=dpd<<30;
    getDPDb(dpd, 2); encode|=dpd<<20;
    getDPDb(dpd, 1); encode|=dpd<<10;
    getDPDb(dpd, 0); encode|=dpd;
    DFWORD(df, 3)=encode;
  #endif
  // decFloatShow(df, "fromB");
  return df;
  } // decFloatFromBCD

/* ------------------------------------------------------------------ */
/* decFloatFromPacked -- set decFloat from exponent and packed BCD    */
/*                                                                    */
/*  df is the target decFloat                                         */
/*  exp is the in-range unbiased exponent, q, or a special value in   */
/*    the form returned by decFloatGetExponent                        */
/*  packed holds DECPMAX packed decimal digits plus a sign nibble     */
/*    (all 6 codes are OK); the first (MSD) is ignored if df is a NaN */
/*    and all except sign are ignored if df is infinite.  For DOUBLE  */
/*    and QUAD the first (pad) nibble is also ignored in all cases.   */
/*    All coefficient nibbles must be in 0-9 and sign in A-F; results */
/*    are undefined otherwise.                                        */
/*  returns df, which will be canonical                               */
/*                                                                    */
/* No error is possible, and no status will be set.                   */
/* ------------------------------------------------------------------ */
decFloat * decFloatFromPacked(decFloat *df, Int exp, const uByte *packed) {
  uByte bcdar[DECPMAX+2];               // work [+1 for pad, +1 for sign]
  const uByte *ip;                      // ..
  uByte *op;                            // ..
  Int   sig=0;                          // sign

  // expand coefficient and sign to BCDAR
  #if SINGLE
  op=bcdar+1;                           // no pad digit
  #else
  op=bcdar;                             // first (pad) digit ignored
  #endif
  for (ip=packed; ip<packed+((DECPMAX+2)/2); ip++) {
    *op++=*ip>>4;
    *op++=(uByte)(*ip&0x0f);            // [final nibble is sign]
    }
  op--;                                 // -> sign byte
  if (*op==DECPMINUS || *op==DECPMINUSALT) sig=DECFLOAT_Sign;

  if (EXPISSPECIAL(exp)) {              // Infinity or NaN
    if (!EXPISINF(exp)) bcdar[1]=0;     // a NaN: ignore MSD
     else memset(bcdar+1, 0, DECPMAX);  // Infinite: coefficient to 0
    }
  return decFloatFromBCD(df, exp, bcdar+1, sig);
  } // decFloatFromPacked

/* ------------------------------------------------------------------ */
/* decFloatFromPackedChecked -- set from exponent and packed; checked */
/*                                                                    */
/*  df is the target decFloat                                         */
/*  exp is the in-range unbiased exponent, q, or a special value in   */
/*    the form returned by decFloatGetExponent                        */
/*  packed holds DECPMAX packed decimal digits plus a sign nibble     */
/*    (all 6 codes are OK); the first (MSD) must be 0 if df is a NaN  */
/*    and all digits must be 0 if df is infinite.  For DOUBLE and     */
/*    QUAD the first (pad) nibble must be 0.                          */
/*    All coefficient nibbles must be in 0-9 and sign in A-F.         */
/*  returns df, which will be canonical or NULL if any of the         */
/*    requirements are not met (if this case df is unchanged); that   */
/*    is, the input data must be as returned by decFloatToPacked,     */
/*    except that all six sign codes are acccepted.                   */
/*                                                                    */
/* No status will be set.                                             */
/* ------------------------------------------------------------------ */
decFloat * decFloatFromPackedChecked(decFloat *df, Int exp,
                                     const uByte *packed) {
  uByte bcdar[DECPMAX+2];               // work [+1 for pad, +1 for sign]
  const uByte *ip;                      // ..
  uByte *op;                            // ..
  Int   sig=0;                          // sign

  // expand coefficient and sign to BCDAR
  #if SINGLE
  op=bcdar+1;                           // no pad digit
  #else
  op=bcdar;                             // first (pad) digit here
  #endif
  for (ip=packed; ip<packed+((DECPMAX+2)/2); ip++) {
    *op=*ip>>4;
    if (*op>9) return NULL;
    op++;
    *op=(uByte)(*ip&0x0f);              // [final nibble is sign]
    if (*op>9 && ip<packed+((DECPMAX+2)/2)-1) return NULL;
    op++;
    }
  op--;                                 // -> sign byte
  if (*op<=9) return NULL;              // bad sign
  if (*op==DECPMINUS || *op==DECPMINUSALT) sig=DECFLOAT_Sign;

  #if !SINGLE
  if (bcdar[0]!=0) return NULL;         // bad pad nibble
  #endif

  if (EXPISNAN(exp)) {                  // a NaN
    if (bcdar[1]!=0) return NULL;       // bad msd
    } // NaN
   else if (EXPISINF(exp)) {            // is infinite
    Int i;
    for (i=0; i<DECPMAX; i++) {
      if (bcdar[i+1]!=0) return NULL;   // should be all zeros
      }
    } // infinity
   else {                               // finite
    // check the exponent is in range
    if (exp>DECEMAX-DECPMAX+1) return NULL;
    if (exp<DECEMIN-DECPMAX+1) return NULL;
    }
  return decFloatFromBCD(df, exp, bcdar+1, sig);
  } // decFloatFromPacked

/* ------------------------------------------------------------------ */
/* decFloatFromString -- conversion from numeric string               */
/*                                                                    */
/*  result  is the decFloat format number which gets the result of    */
/*          the conversion                                            */
/*  *string is the character string which should contain a valid      */
/*          number (which may be a special value), \0-terminated      */
/*          If there are too many significant digits in the           */
/*          coefficient it will be rounded.                           */
/*  set     is the context                                            */
/*  returns result                                                    */
/*                                                                    */
/* The length of the coefficient and the size of the exponent are     */
/* checked by this routine, so the correct error (Underflow or        */
/* Overflow) can be reported or rounding applied, as necessary.       */
/*                                                                    */
/* There is no limit to the coefficient length for finite inputs;     */
/* NaN payloads must be integers with no more than DECPMAX-1 digits.  */
/* Exponents may have up to nine significant digits.                  */
/*                                                                    */
/* If bad syntax is detected, the result will be a quiet NaN.         */
/* ------------------------------------------------------------------ */
decFloat * decFloatFromString(decFloat *result, const char *string,
                              decContext *set) {
  Int    digits;                   // count of digits in coefficient
  const  char *dotchar=NULL;       // where dot was found [NULL if none]
  const  char *cfirst=string;      // -> first character of decimal part
  const  char *c;                  // work
  uByte *ub;                       // ..
  uInt   uiwork;                   // for macros
  bcdnum num;                      // collects data for finishing
  uInt   error=DEC_Conversion_syntax;      // assume the worst
  uByte  buffer[ROUNDUP(DECSTRING+11, 8)]; // room for most coefficents,
                                           // some common rounding, +3, & pad
  #if DECTRACE
  // printf("FromString %s ...\n", string);
  #endif

  for(;;) {                             // once-only 'loop'
    num.sign=0;                         // assume non-negative
    num.msd=buffer;                     // MSD is here always

    // detect and validate the coefficient, including any leading,
    // trailing, or embedded '.'
    // [could test four-at-a-time here (saving 10% for decQuads),
    // but that risks storage violation because the position of the
    // terminator is unknown]
    for (c=string;; c++) {              // -> input character
      if (((unsigned)(*c-'0'))<=9) continue; // '0' through '9' is good
      if (*c=='\0') break;              // most common non-digit
      if (*c=='.') {
        if (dotchar!=NULL) break;       // not first '.'
        dotchar=c;                      // record offset into decimal part
        continue;}
      if (c==string) {                  // first in string...
        if (*c=='-') {                  // valid - sign
          cfirst++;
          num.sign=DECFLOAT_Sign;
          continue;}
        if (*c=='+') {                  // valid + sign
          cfirst++;
          continue;}
        }
      // *c is not a digit, terminator, or a valid +, -, or '.'
      break;
      } // c loop

    digits=(uInt)(c-cfirst);            // digits (+1 if a dot)

    if (digits>0) {                     // had digits and/or dot
      const char *clast=c-1;            // note last coefficient char position
      Int exp=0;                        // exponent accumulator
      if (*c!='\0') {                   // something follows the coefficient
        uInt edig;                      // unsigned work
        // had some digits and more to come; expect E[+|-]nnn now
        const char *firstexp;           // exponent first non-zero
        if (*c!='E' && *c!='e') break;
        c++;                            // to (optional) sign
        if (*c=='-' || *c=='+') c++;    // step over sign (c=clast+2)
        if (*c=='\0') break;            // no digits!  (e.g., '1.2E')
        for (; *c=='0';) c++;           // skip leading zeros [even last]
        firstexp=c;                     // remember start [maybe '\0']
        // gather exponent digits
        edig=(uInt)*c-(uInt)'0';
        if (edig<=9) {                  // [check not bad or terminator]
          exp+=edig;                    // avoid initial X10
          c++;
          for (;; c++) {
            edig=(uInt)*c-(uInt)'0';
            if (edig>9) break;
            exp=exp*10+edig;
            }
          }
        // if not now on the '\0', *c must not be a digit
        if (*c!='\0') break;

        // (this next test must be after the syntax checks)
        // if definitely more than the possible digits for format then
        // the exponent may have wrapped, so simply set it to a certain
        // over/underflow value
        if (c>firstexp+DECEMAXD) exp=DECEMAX*2;
        if (*(clast+2)=='-') exp=-exp;  // was negative
        } // exponent part

      if (dotchar!=NULL) {              // had a '.'
        digits--;                       // remove from digits count
        if (digits==0) break;           // was dot alone: bad syntax
        exp-=(Int)(clast-dotchar);      // adjust exponent
        // [the '.' can now be ignored]
        }
      num.exponent=exp;                 // exponent is good; store it

      // Here when whole string has been inspected and syntax is good
      // cfirst->first digit or dot, clast->last digit or dot
      error=0;                          // no error possible now

      // if the number of digits in the coefficient will fit in buffer
      // then it can simply be converted to bcd8 and copied -- decFinalize
      // will take care of leading zeros and rounding; the buffer is big
      // enough for all canonical coefficients, including 0.00000nn...
      ub=buffer;
      if (digits<=(Int)(sizeof(buffer)-3)) { // [-3 allows by-4s copy]
        c=cfirst;
        if (dotchar!=NULL) {                 // a dot to worry about
          if (*(c+1)=='.') {                 // common canonical case
            *ub++=(uByte)(*c-'0');           // copy leading digit
            c+=2;                            // prepare to handle rest
            }
           else for (; c<=clast;) {          // '.' could be anywhere
            // as usual, go by fours when safe; NB it has been asserted
            // that a '.' does not have the same mask as a digit
            if (c<=clast-3                             // safe for four
             && (UBTOUI(c)&0xf0f0f0f0)==CHARMASK) {    // test four
              UBFROMUI(ub, UBTOUI(c)&0x0f0f0f0f);      // to BCD8
              ub+=4;
              c+=4;
              continue;
              }
            if (*c=='.') {                   // found the dot
              c++;                           // step over it ..
              break;                         // .. and handle the rest
              }
            *ub++=(uByte)(*c++-'0');
            }
          } // had dot
        // Now no dot; do this by fours (where safe)
        for (; c<=clast-3; c+=4, ub+=4) UBFROMUI(ub, UBTOUI(c)&0x0f0f0f0f);
        for (; c<=clast; c++, ub++) *ub=(uByte)(*c-'0');
        num.lsd=buffer+digits-1;             // record new LSD
        } // fits

       else {                                // too long for buffer
        // [This is a rare and unusual case; arbitrary-length input]
        // strip leading zeros [but leave final 0 if all 0's]
        if (*cfirst=='.') cfirst++;          // step past dot at start
        if (*cfirst=='0') {                  // [cfirst always -> digit]
          for (; cfirst<clast; cfirst++) {
            if (*cfirst!='0') {              // non-zero found
              if (*cfirst=='.') continue;    // [ignore]
              break;                         // done
              }
            digits--;                        // 0 stripped
            } // cfirst
          } // at least one leading 0

        // the coefficient is now as short as possible, but may still
        // be too long; copy up to Pmax+1 digits to the buffer, then
        // just record any non-zeros (set round-for-reround digit)
        for (c=cfirst; c<=clast && ub<=buffer+DECPMAX; c++) {
          // (see commentary just above)
          if (c<=clast-3                          // safe for four
           && (UBTOUI(c)&0xf0f0f0f0)==CHARMASK) { // four digits
            UBFROMUI(ub, UBTOUI(c)&0x0f0f0f0f);   // to BCD8
            ub+=4;
            c+=3;                            // [will become 4]
            continue;
            }
          if (*c=='.') continue;             // [ignore]
          *ub++=(uByte)(*c-'0');
          }
        ub--;                                // -> LSD
        for (; c<=clast; c++) {              // inspect remaining chars
          if (*c!='0') {                     // sticky bit needed
            if (*c=='.') continue;           // [ignore]
            *ub=DECSTICKYTAB[*ub];           // update round-for-reround
            break;                           // no need to look at more
            }
          }
        num.lsd=ub;                          // record LSD
        // adjust exponent for dropped digits
        num.exponent+=digits-(Int)(ub-buffer+1);
        } // too long for buffer
      } // digits and/or dot

     else {                             // no digits or dot were found
      // only Infinities and NaNs are allowed, here
      if (*c=='\0') break;              // nothing there is bad
      buffer[0]=0;                      // default a coefficient of 0
      num.lsd=buffer;                   // ..
      if (decBiStr(c, "infinity", "INFINITY")
       || decBiStr(c, "inf", "INF")) num.exponent=DECFLOAT_Inf;
       else {                           // should be a NaN
        num.exponent=DECFLOAT_qNaN;     // assume quiet NaN
        if (*c=='s' || *c=='S') {       // probably an sNaN
          num.exponent=DECFLOAT_sNaN;   // effect the 's'
          c++;                          // and step over it
          }
        if (*c!='N' && *c!='n') break;  // check caseless "NaN"
        c++;
        if (*c!='a' && *c!='A') break;  // ..
        c++;
        if (*c!='N' && *c!='n') break;  // ..
        c++;
        // now either nothing, or nnnn payload (no dots), expected
        // -> start of integer, and skip leading 0s [including plain 0]
        for (cfirst=c; *cfirst=='0';) cfirst++;
        if (*cfirst!='\0') {            // not empty or all-0, payload
          // payload found; check all valid digits and copy to buffer as bcd8
          ub=buffer;
          for (c=cfirst;; c++, ub++) {
            if ((unsigned)(*c-'0')>9) break; // quit if not 0-9
            if (c-cfirst==DECPMAX-1) break;  // too many digits
            *ub=(uByte)(*c-'0');        // good bcd8
            }
          if (*c!='\0') break;          // not all digits, or too many
          num.lsd=ub-1;                 // record new LSD
          }
        } // NaN or sNaN
      error=0;                          // syntax is OK
      } // digits=0 (special expected)
    break;                              // drop out
    }                                   // [for(;;) once-loop]

  // decShowNum(&num, "fromStr");

  if (error!=0) {
    set->status|=error;
    num.exponent=DECFLOAT_qNaN;         // set up quiet NaN
    num.sign=0;                         // .. with 0 sign
    buffer[0]=0;                        // .. and coefficient
    num.lsd=buffer;                     // ..
    // decShowNum(&num, "oops");
    }

  // decShowNum(&num, "dffs");
  decFinalize(result, &num, set);       // round, check, and lay out
  // decFloatShow(result, "fromString");
  return result;
  } // decFloatFromString

/* ------------------------------------------------------------------ */
/* decFloatFromWider -- conversion from next-wider format             */
/*                                                                    */
/*  result  is the decFloat format number which gets the result of    */
/*          the conversion                                            */
/*  wider   is the decFloatWider format number which will be narrowed */
/*  set     is the context                                            */
/*  returns result                                                    */
/*                                                                    */
/* Narrowing can cause rounding, overflow, etc., but not Invalid      */
/* operation (sNaNs are copied and do not signal).                    */
/* ------------------------------------------------------------------ */
// narrow-to is not possible for decQuad format numbers; simply omit
#if !QUAD
decFloat * decFloatFromWider(decFloat *result, const decFloatWider *wider,
                             decContext *set) {
  bcdnum num;                           // collects data for finishing
  uByte  bcdar[DECWPMAX];               // room for wider coefficient
  uInt   widerhi=DFWWORD(wider, 0);     // top word
  Int    exp;

  GETWCOEFF(wider, bcdar);

  num.msd=bcdar;                        // MSD is here always
  num.lsd=bcdar+DECWPMAX-1;             // LSD is here always
  num.sign=widerhi&0x80000000;          // extract sign [DECFLOAT_Sign=Neg]

  // decode the wider combination field to exponent
  exp=DECCOMBWEXP[widerhi>>26];         // decode from wider combination field
  // if it is a special there's nothing to do unless sNaN; if it's
  // finite then add the (wider) exponent continuation and unbias
  if (EXPISSPECIAL(exp)) exp=widerhi&0x7e000000; // include sNaN selector
   else exp+=GETWECON(wider)-DECWBIAS;
  num.exponent=exp;

  // decShowNum(&num, "dffw");
  return decFinalize(result, &num, set);// round, check, and lay out
  } // decFloatFromWider
#endif

/* ------------------------------------------------------------------ */
/* decFloatGetCoefficient -- get coefficient as BCD8                  */
/*                                                                    */
/*  df is the decFloat from which to extract the coefficient          */
/*  bcdar is where DECPMAX bytes will be written, one BCD digit in    */
/*    each byte (BCD8 encoding); if df is a NaN the first byte will   */
/*    be zero, and if it is infinite they will all be zero            */
/*  returns the sign of the coefficient (DECFLOAT_Sign if negative,   */
/*    0 otherwise)                                                    */
/*                                                                    */
/* No error is possible, and no status will be set.  If df is a       */
/* special value the array is set to zeros (for Infinity) or to the   */
/* payload of a qNaN or sNaN.                                         */
/* ------------------------------------------------------------------ */
Int decFloatGetCoefficient(const decFloat *df, uByte *bcdar) {
  if (DFISINF(df)) memset(bcdar, 0, DECPMAX);
   else {
    GETCOEFF(df, bcdar);           // use macro
    if (DFISNAN(df)) bcdar[0]=0;   // MSD needs correcting
    }
  return GETSIGN(df);
  } // decFloatGetCoefficient

/* ------------------------------------------------------------------ */
/* decFloatGetExponent -- get unbiased exponent                       */
/*                                                                    */
/*  df is the decFloat from which to extract the exponent             */
/*  returns the exponent, q.                                          */
/*                                                                    */
/* No error is possible, and no status will be set.  If df is a       */
/* special value the first seven bits of the decFloat are returned,   */
/* left adjusted and with the first (sign) bit set to 0 (followed by  */
/* 25 0 bits).  e.g., -sNaN would return 0x7e000000 (DECFLOAT_sNaN).  */
/* ------------------------------------------------------------------ */
Int decFloatGetExponent(const decFloat *df) {
  if (DFISSPECIAL(df)) return DFWORD(df, 0)&0x7e000000;
  return GETEXPUN(df);
  } // decFloatGetExponent

/* ------------------------------------------------------------------ */
/* decFloatSetCoefficient -- set coefficient from BCD8                */
/*                                                                    */
/*  df is the target decFloat (and source of exponent/special value)  */
/*  bcdar holds DECPMAX digits to set the coefficient from, one       */
/*    digit in each byte (BCD8 encoding); the first (MSD) is ignored  */
/*    if df is a NaN; all are ignored if df is infinite.              */
/*  sig is DECFLOAT_Sign to set the sign bit, 0 otherwise             */
/*  returns df, which will be canonical                               */
/*                                                                    */
/* No error is possible, and no status will be set.                   */
/* ------------------------------------------------------------------ */
decFloat * decFloatSetCoefficient(decFloat *df, const uByte *bcdar,
                                  Int sig) {
  uInt exp;                        // for exponent
  uByte bcdzero[DECPMAX];          // for infinities

  // Exponent/special code is extracted from df
  if (DFISSPECIAL(df)) {
    exp=DFWORD(df, 0)&0x7e000000;
    if (DFISINF(df)) {
      memset(bcdzero, 0, DECPMAX);
      return decFloatFromBCD(df, exp, bcdzero, sig);
      }
    }
   else exp=GETEXPUN(df);
  return decFloatFromBCD(df, exp, bcdar, sig);
  } // decFloatSetCoefficient

/* ------------------------------------------------------------------ */
/* decFloatSetExponent -- set exponent or special value               */
/*                                                                    */
/*  df  is the target decFloat (and source of coefficient/payload)    */
/*  set is the context for reporting status                           */
/*  exp is the unbiased exponent, q, or a special value in the form   */
/*    returned by decFloatGetExponent                                 */
/*  returns df, which will be canonical                               */
/*                                                                    */
/* No error is possible, but Overflow or Underflow might occur.       */
/* ------------------------------------------------------------------ */
decFloat * decFloatSetExponent(decFloat *df, decContext *set, Int exp) {
  uByte  bcdcopy[DECPMAX];         // for coefficient
  bcdnum num;                      // work
  num.exponent=exp;
  num.sign=decFloatGetCoefficient(df, bcdcopy); // extract coefficient
  if (DFISSPECIAL(df)) {           // MSD or more needs correcting
    if (DFISINF(df)) memset(bcdcopy, 0, DECPMAX);
    bcdcopy[0]=0;
    }
  num.msd=bcdcopy;
  num.lsd=bcdcopy+DECPMAX-1;
  return decFinalize(df, &num, set);
  } // decFloatSetExponent

/* ------------------------------------------------------------------ */
/* decFloatRadix -- returns the base (10)                             */
/*                                                                    */
/*   df is any decFloat of this format                                */
/* ------------------------------------------------------------------ */
uInt decFloatRadix(const decFloat *df) {
  if (df) return 10;                         // to placate compiler
  return 10;
  } // decFloatRadix

/* The following function is not available if DECPRINT=0              */
#if DECPRINT
/* ------------------------------------------------------------------ */
/* decFloatShow -- printf a decFloat in hexadecimal and decimal       */
/*   df  is the decFloat to show                                      */
/*   tag is a tag string displayed with the number                    */
/*                                                                    */
/* This is a debug aid; the precise format of the string may change.  */
/* ------------------------------------------------------------------ */
void decFloatShow(const decFloat *df, const char *tag) {
  char hexbuf[DECBYTES*2+DECBYTES/4+1]; // NB blank after every fourth
  char buff[DECSTRING];                 // for value in decimal
  Int i, j=0;

  for (i=0; i<DECBYTES; i++) {
    #if DECLITEND
      sprintf(&hexbuf[j], "%02x", df->bytes[DECBYTES-1-i]);
    #else
      sprintf(&hexbuf[j], "%02x", df->bytes[i]);
    #endif
    j+=2;
    // the next line adds blank (and terminator) after final pair, too
    if ((i+1)%4==0) {strcpy(&hexbuf[j], " "); j++;}
    }
  decFloatToString(df, buff);
  printf(">%s> %s [big-endian]  %s\n", tag, hexbuf, buff);
  return;
  } // decFloatShow
#endif

/* ------------------------------------------------------------------ */
/* decFloatToBCD -- get sign, exponent, and BCD8 from a decFloat      */
/*                                                                    */
/*  df is the source decFloat                                         */
/*  exp will be set to the unbiased exponent, q, or to a special      */
/*    value in the form returned by decFloatGetExponent               */
/*  bcdar is where DECPMAX bytes will be written, one BCD digit in    */
/*    each byte (BCD8 encoding); if df is a NaN the first byte will   */
/*    be zero, and if it is infinite they will all be zero            */
/*  returns the sign of the coefficient (DECFLOAT_Sign if negative,   */
/*    0 otherwise)                                                    */
/*                                                                    */
/* No error is possible, and no status will be set.                   */
/* ------------------------------------------------------------------ */
Int decFloatToBCD(const decFloat *df, Int *exp, uByte *bcdar) {
  if (DFISINF(df)) {
    memset(bcdar, 0, DECPMAX);
    *exp=DFWORD(df, 0)&0x7e000000;
    }
   else {
    GETCOEFF(df, bcdar);           // use macro
    if (DFISNAN(df)) {
      bcdar[0]=0;                  // MSD needs correcting
      *exp=DFWORD(df, 0)&0x7e000000;
      }
     else {                        // finite
      *exp=GETEXPUN(df);
      }
    }
  return GETSIGN(df);
  } // decFloatToBCD

/* ------------------------------------------------------------------ */
/* decFloatToEngString -- conversion to numeric string, engineering   */
/*                                                                    */
/*  df is the decFloat format number to convert                       */
/*  string is the string where the result will be laid out            */
/*                                                                    */
/* string must be at least DECPMAX+9 characters (the worst case is    */
/* "-0.00000nnn...nnn\0", which is as long as the exponent form when  */
/* DECEMAXD<=4); this condition is asserted above                     */
/*                                                                    */
/* No error is possible, and no status will be set                    */
/* ------------------------------------------------------------------ */
char * decFloatToEngString(const decFloat *df, char *string){
  uInt msd;                        // coefficient MSD
  Int  exp;                        // exponent top two bits or full
  uInt comb;                       // combination field
  char *cstart;                    // coefficient start
  char *c;                         // output pointer in string
  char *s, *t;                     // .. (source, target)
  Int  pre, e;                     // work
  const uByte *u;                  // ..
  uInt  uiwork;                    // for macros [one compiler needs
                                   // volatile here to avoid bug, but
                                   // that doubles execution time]

  // Source words; macro handles endianness
  uInt sourhi=DFWORD(df, 0);       // word with sign
  #if DECPMAX==16
  uInt sourlo=DFWORD(df, 1);
  #elif DECPMAX==34
  uInt sourmh=DFWORD(df, 1);
  uInt sourml=DFWORD(df, 2);
  uInt sourlo=DFWORD(df, 3);
  #endif

  c=string;                        // where result will go
  if (((Int)sourhi)<0) *c++='-';   // handle sign
  comb=sourhi>>26;                 // sign+combination field
  msd=DECCOMBMSD[comb];            // decode the combination field
  exp=DECCOMBEXP[comb];            // ..

  if (EXPISSPECIAL(exp)) {         // special
    if (exp==DECFLOAT_Inf) {       // infinity
      strcpy(c,   "Inf");
      strcpy(c+3, "inity");
      return string;               // easy
      }
    if (sourhi&0x02000000) *c++='s'; // sNaN
    strcpy(c, "NaN");              // complete word
    c+=3;                          // step past
    // quick exit if the payload is zero
    #if DECPMAX==7
    if ((sourhi&0x000fffff)==0) return string;
    #elif DECPMAX==16
    if (sourlo==0 && (sourhi&0x0003ffff)==0) return string;
    #elif DECPMAX==34
    if (sourlo==0 && sourml==0 && sourmh==0
     && (sourhi&0x00003fff)==0) return string;
    #endif
    // otherwise drop through to add integer; set correct exp etc.
    exp=0; msd=0;                  // setup for following code
    }
   else { // complete exponent; top two bits are in place
    exp+=GETECON(df)-DECBIAS;      // .. + continuation and unbias
    }

  /* convert the digits of the significand to characters */
  cstart=c;                        // save start of coefficient
  if (msd) *c++=(char)('0'+(char)msd);  // non-zero most significant digit

  // Decode the declets.  After extracting each declet, it is
  // decoded to a 4-uByte sequence by table lookup; the four uBytes
  // are the three encoded BCD8 digits followed by a 1-byte length
  // (significant digits, except that 000 has length 0).  This allows
  // us to left-align the first declet with non-zero content, then
  // the remaining ones are full 3-char length.  Fixed-length copies
  // are used because variable-length memcpy causes a subroutine call
  // in at least two compilers.  (The copies are length 4 for speed
  // and are safe because the last item in the array is of length
  // three and has the length byte following.)
  #define dpd2char(dpdin) u=&DPD2BCD8[((dpdin)&0x3ff)*4];        \
         if (c!=cstart) {UBFROMUI(c, UBTOUI(u)|CHARMASK); c+=3;} \
          else if (*(u+3)) {                                     \
           UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK); c+=*(u+3);}

  #if DECPMAX==7
  dpd2char(sourhi>>10);                 // declet 1
  dpd2char(sourhi);                     // declet 2

  #elif DECPMAX==16
  dpd2char(sourhi>>8);                  // declet 1
  dpd2char((sourhi<<2) | (sourlo>>30)); // declet 2
  dpd2char(sourlo>>20);                 // declet 3
  dpd2char(sourlo>>10);                 // declet 4
  dpd2char(sourlo);                     // declet 5

  #elif DECPMAX==34
  dpd2char(sourhi>>4);                  // declet 1
  dpd2char((sourhi<<6) | (sourmh>>26)); // declet 2
  dpd2char(sourmh>>16);                 // declet 3
  dpd2char(sourmh>>6);                  // declet 4
  dpd2char((sourmh<<4) | (sourml>>28)); // declet 5
  dpd2char(sourml>>18);                 // declet 6
  dpd2char(sourml>>8);                  // declet 7
  dpd2char((sourml<<2) | (sourlo>>30)); // declet 8
  dpd2char(sourlo>>20);                 // declet 9
  dpd2char(sourlo>>10);                 // declet 10
  dpd2char(sourlo);                     // declet 11
  #endif

  if (c==cstart) *c++='0';         // all zeros, empty -- make "0"

  if (exp==0) {                    // integer or NaN case -- easy
    *c='\0';                       // terminate
    return string;
    }
  /* non-0 exponent */

  e=0;                             // assume no E
  pre=(Int)(c-cstart)+exp;         // length+exp  [c->LSD+1]
  // [here, pre-exp is the digits count (==1 for zero)]

  if (exp>0 || pre<-5) {           // need exponential form
    e=pre-1;                       // calculate E value
    pre=1;                         // assume one digit before '.'
    if (e!=0) {                    // engineering: may need to adjust
      Int adj;                     // adjustment
      // The C remainder operator is undefined for negative numbers, so
      // a positive remainder calculation must be used here
      if (e<0) {
        adj=(-e)%3;
        if (adj!=0) adj=3-adj;
        }
       else { // e>0
        adj=e%3;
        }
      e=e-adj;
      // if dealing with zero still produce an exponent which is a
      // multiple of three, as expected, but there will only be the
      // one zero before the E, still.  Otherwise note the padding.
      if (!DFISZERO(df)) pre+=adj;
       else {  // is zero
        if (adj!=0) {              // 0.00Esnn needed
          e=e+3;
          pre=-(2-adj);
          }
        } // zero
      } // engineering adjustment
    } // exponential form
  // printf("e=%ld pre=%ld exp=%ld\n", (LI)e, (LI)pre, (LI)exp);

  /* modify the coefficient, adding 0s, '.', and E+nn as needed */
  if (pre>0) {                     // ddd.ddd (plain), perhaps with E
                                   // or dd00 padding for engineering
    char *dotat=cstart+pre;
    if (dotat<c) {                      // if embedded dot needed...
      // move by fours; there must be space for junk at the end
      // because there is still space for exponent
      s=dotat+ROUNDDOWN4(c-dotat);      // source
      t=s+1;                            // target
      // open the gap [cannot use memcpy]
      for (; s>=dotat; s-=4, t-=4) UBFROMUI(t, UBTOUI(s));
      *dotat='.';
      c++;                              // length increased by one
      } // need dot?
     else for (; c<dotat; c++) *c='0';  // pad for engineering
    } // pre>0
   else {
    /* -5<=pre<=0: here for plain 0.ddd or 0.000ddd forms (may have
       E, but only for 0.00E+3 kind of case -- with plenty of spare
       space in this case */
    pre=-pre+2;                         // gap width, including "0."
    t=cstart+ROUNDDOWN4(c-cstart)+pre;  // preferred first target point
    // backoff if too far to the right
    if (t>string+DECSTRING-5) t=string+DECSTRING-5; // adjust to fit
    // now shift the entire coefficient to the right, being careful not
    // to access to the left of string [cannot use memcpy]
    for (s=t-pre; s>=string; s-=4, t-=4) UBFROMUI(t, UBTOUI(s));
    // for Quads and Singles there may be a character or two left...
    s+=3;                               // where next would come from
    for(; s>=cstart; s--, t--) *(t+3)=*(s);
    // now have fill 0. through 0.00000; use overlaps to avoid tests
    if (pre>=4) {
      memcpy(cstart+pre-4, "0000", 4);
      memcpy(cstart, "0.00", 4);
      }
     else { // 2 or 3
      *(cstart+pre-1)='0';
      memcpy(cstart, "0.", 2);
      }
    c+=pre;                             // to end
    }

  // finally add the E-part, if needed; it will never be 0, and has
  // a maximum length of 3 or 4 digits (asserted above)
  if (e!=0) {
    memcpy(c, "E+", 2);                 // starts with E, assume +
    c++;
    if (e<0) {
      *c='-';                           // oops, need '-'
      e=-e;                             // uInt, please
      }
    c++;
    // Three-character exponents are easy; 4-character a little trickier
    #if DECEMAXD<=3
      u=&BIN2BCD8[e*4];                 // -> 3 digits + length byte
      // copy fixed 4 characters [is safe], starting at non-zero
      // and with character mask to convert BCD to char
      UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK);
      c+=*(u+3);                        // bump pointer appropriately
    #elif DECEMAXD==4
      if (e<1000) {                     // 3 (or fewer) digits case
        u=&BIN2BCD8[e*4];               // -> 3 digits + length byte
        UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK); // [as above]
        c+=*(u+3);                      // bump pointer appropriately
        }
       else {                           // 4-digits
        Int thou=((e>>3)*1049)>>17;     // e/1000
        Int rem=e-(1000*thou);          // e%1000
        *c++=(char)('0'+(char)thou);    // the thousands digit
        u=&BIN2BCD8[rem*4];             // -> 3 digits + length byte
        UBFROMUI(c, UBTOUI(u)|CHARMASK);// copy fixed 3+1 characters [is safe]
        c+=3;                           // bump pointer, always 3 digits
        }
    #endif
    }
  *c='\0';                              // terminate
  //printf("res %s\n", string);
  return string;
  } // decFloatToEngString

/* ------------------------------------------------------------------ */
/* decFloatToPacked -- convert decFloat to Packed decimal + exponent  */
/*                                                                    */
/*  df is the source decFloat                                         */
/*  exp will be set to the unbiased exponent, q, or to a special      */
/*    value in the form returned by decFloatGetExponent               */
/*  packed is where DECPMAX nibbles will be written with the sign as  */
/*    final nibble (0x0c for +, 0x0d for -); a NaN has a first nibble */
/*    of zero, and an infinity is all zeros. decDouble and decQuad    */
/*    have a additional leading zero nibble, leading to result        */
/*    lengths of 4, 9, and 18 bytes.                                  */
/*  returns the sign of the coefficient (DECFLOAT_Sign if negative,   */
/*    0 otherwise)                                                    */
/*                                                                    */
/* No error is possible, and no status will be set.                   */
/* ------------------------------------------------------------------ */
Int decFloatToPacked(const decFloat *df, Int *exp, uByte *packed) {
  uByte bcdar[DECPMAX+2];          // work buffer
  uByte *ip=bcdar, *op=packed;     // work pointers
  if (DFISINF(df)) {
    memset(bcdar, 0, DECPMAX+2);
    *exp=DECFLOAT_Inf;
    }
   else {
    GETCOEFF(df, bcdar+1);         // use macro
    if (DFISNAN(df)) {
      bcdar[1]=0;                  // MSD needs clearing
      *exp=DFWORD(df, 0)&0x7e000000;
      }
     else {                        // finite
      *exp=GETEXPUN(df);
      }
    }
  // now pack; coefficient currently at bcdar+1
  #if SINGLE
    ip++;                          // ignore first byte
  #else
    *ip=0;                         // need leading zero
  #endif
  // set final byte to Packed BCD sign value
  bcdar[DECPMAX+1]=(DFISSIGNED(df) ? DECPMINUS : DECPPLUS);
  // pack an even number of bytes...
  for (; op<packed+((DECPMAX+2)/2); op++, ip+=2) {
    *op=(uByte)((*ip<<4)+*(ip+1));
    }
  return (bcdar[DECPMAX+1]==DECPMINUS ? DECFLOAT_Sign : 0);
  } // decFloatToPacked

/* ------------------------------------------------------------------ */
/* decFloatToString -- conversion to numeric string                   */
/*                                                                    */
/*  df is the decFloat format number to convert                       */
/*  string is the string where the result will be laid out            */
/*                                                                    */
/* string must be at least DECPMAX+9 characters (the worst case is    */
/* "-0.00000nnn...nnn\0", which is as long as the exponent form when  */
/* DECEMAXD<=4); this condition is asserted above                     */
/*                                                                    */
/* No error is possible, and no status will be set                    */
/* ------------------------------------------------------------------ */
char * decFloatToString(const decFloat *df, char *string){
  uInt msd;                        // coefficient MSD
  Int  exp;                        // exponent top two bits or full
  uInt comb;                       // combination field
  char *cstart;                    // coefficient start
  char *c;                         // output pointer in string
  char *s, *t;                     // .. (source, target)
  Int  pre, e;                     // work
  const uByte *u;                  // ..
  uInt  uiwork;                    // for macros [one compiler needs
                                   // volatile here to avoid bug, but
                                   // that doubles execution time]

  // Source words; macro handles endianness
  uInt sourhi=DFWORD(df, 0);       // word with sign
  #if DECPMAX==16
  uInt sourlo=DFWORD(df, 1);
  #elif DECPMAX==34
  uInt sourmh=DFWORD(df, 1);
  uInt sourml=DFWORD(df, 2);
  uInt sourlo=DFWORD(df, 3);
  #endif

  c=string;                        // where result will go
  if (((Int)sourhi)<0) *c++='-';   // handle sign
  comb=sourhi>>26;                 // sign+combination field
  msd=DECCOMBMSD[comb];            // decode the combination field
  exp=DECCOMBEXP[comb];            // ..

  if (!EXPISSPECIAL(exp)) {        // finite
    // complete exponent; top two bits are in place
    exp+=GETECON(df)-DECBIAS;      // .. + continuation and unbias
    }
   else {                          // IS special
    if (exp==DECFLOAT_Inf) {       // infinity
      strcpy(c, "Infinity");
      return string;               // easy
      }
    if (sourhi&0x02000000) *c++='s'; // sNaN
    strcpy(c, "NaN");              // complete word
    c+=3;                          // step past
    // quick exit if the payload is zero
    #if DECPMAX==7
    if ((sourhi&0x000fffff)==0) return string;
    #elif DECPMAX==16
    if (sourlo==0 && (sourhi&0x0003ffff)==0) return string;
    #elif DECPMAX==34
    if (sourlo==0 && sourml==0 && sourmh==0
     && (sourhi&0x00003fff)==0) return string;
    #endif
    // otherwise drop through to add integer; set correct exp etc.
    exp=0; msd=0;                  // setup for following code
    }

  /* convert the digits of the significand to characters */
  cstart=c;                        // save start of coefficient
  if (msd) *c++=(char)('0'+(char)msd);  // non-zero most significant digit

  // Decode the declets.  After extracting each declet, it is
  // decoded to a 4-uByte sequence by table lookup; the four uBytes
  // are the three encoded BCD8 digits followed by a 1-byte length
  // (significant digits, except that 000 has length 0).  This allows
  // us to left-align the first declet with non-zero content, then
  // the remaining ones are full 3-char length.  Fixed-length copies
  // are used because variable-length memcpy causes a subroutine call
  // in at least two compilers.  (The copies are length 4 for speed
  // and are safe because the last item in the array is of length
  // three and has the length byte following.)
  #define dpd2char(dpdin) u=&DPD2BCD8[((dpdin)&0x3ff)*4];        \
         if (c!=cstart) {UBFROMUI(c, UBTOUI(u)|CHARMASK); c+=3;} \
          else if (*(u+3)) {                                     \
           UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK); c+=*(u+3);}

  #if DECPMAX==7
  dpd2char(sourhi>>10);                 // declet 1
  dpd2char(sourhi);                     // declet 2

  #elif DECPMAX==16
  dpd2char(sourhi>>8);                  // declet 1
  dpd2char((sourhi<<2) | (sourlo>>30)); // declet 2
  dpd2char(sourlo>>20);                 // declet 3
  dpd2char(sourlo>>10);                 // declet 4
  dpd2char(sourlo);                     // declet 5

  #elif DECPMAX==34
  dpd2char(sourhi>>4);                  // declet 1
  dpd2char((sourhi<<6) | (sourmh>>26)); // declet 2
  dpd2char(sourmh>>16);                 // declet 3
  dpd2char(sourmh>>6);                  // declet 4
  dpd2char((sourmh<<4) | (sourml>>28)); // declet 5
  dpd2char(sourml>>18);                 // declet 6
  dpd2char(sourml>>8);                  // declet 7
  dpd2char((sourml<<2) | (sourlo>>30)); // declet 8
  dpd2char(sourlo>>20);                 // declet 9
  dpd2char(sourlo>>10);                 // declet 10
  dpd2char(sourlo);                     // declet 11
  #endif

  if (c==cstart) *c++='0';         // all zeros, empty -- make "0"

  //[This fast path is valid but adds 3-5 cycles to worst case length]
  //if (exp==0) {                  // integer or NaN case -- easy
  //  *c='\0';                     // terminate
  //  return string;
  //  }

  e=0;                             // assume no E
  pre=(Int)(c-cstart)+exp;         // length+exp  [c->LSD+1]
  // [here, pre-exp is the digits count (==1 for zero)]

  if (exp>0 || pre<-5) {           // need exponential form
    e=pre-1;                       // calculate E value
    pre=1;                         // assume one digit before '.'
    } // exponential form

  /* modify the coefficient, adding 0s, '.', and E+nn as needed */
  if (pre>0) {                     // ddd.ddd (plain), perhaps with E
    char *dotat=cstart+pre;
    if (dotat<c) {                      // if embedded dot needed...
      // [memmove is a disaster, here]
      // move by fours; there must be space for junk at the end
      // because exponent is still possible
      s=dotat+ROUNDDOWN4(c-dotat);      // source
      t=s+1;                            // target
      // open the gap [cannot use memcpy]
      for (; s>=dotat; s-=4, t-=4) UBFROMUI(t, UBTOUI(s));
      *dotat='.';
      c++;                              // length increased by one
      } // need dot?

    // finally add the E-part, if needed; it will never be 0, and has
    // a maximum length of 3 or 4 digits (asserted above)
    if (e!=0) {
      memcpy(c, "E+", 2);               // starts with E, assume +
      c++;
      if (e<0) {
        *c='-';                         // oops, need '-'
        e=-e;                           // uInt, please
        }
      c++;
      // Three-character exponents are easy; 4-character a little trickier
      #if DECEMAXD<=3
        u=&BIN2BCD8[e*4];               // -> 3 digits + length byte
        // copy fixed 4 characters [is safe], starting at non-zero
        // and with character mask to convert BCD to char
        UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK);
        c+=*(u+3);                      // bump pointer appropriately
      #elif DECEMAXD==4
        if (e<1000) {                   // 3 (or fewer) digits case
          u=&BIN2BCD8[e*4];             // -> 3 digits + length byte
          UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK); // [as above]
          c+=*(u+3);                    // bump pointer appropriately
          }
         else {                         // 4-digits
          Int thou=((e>>3)*1049)>>17;   // e/1000
          Int rem=e-(1000*thou);        // e%1000
          *c++=(char)('0'+(char)thou);  // the thousands digit
          u=&BIN2BCD8[rem*4];           // -> 3 digits + length byte
          UBFROMUI(c, UBTOUI(u)|CHARMASK); // copy fixed 3+1 characters [is safe]
          c+=3;                         // bump pointer, always 3 digits
          }
      #endif
      }
    *c='\0';                            // add terminator
    //printf("res %s\n", string);
    return string;
    } // pre>0

  /* -5<=pre<=0: here for plain 0.ddd or 0.000ddd forms (can never have E) */
  // Surprisingly, this is close to being the worst-case path, so the
  // shift is done by fours; this is a little tricky because the
  // rightmost character to be written must not be beyond where the
  // rightmost terminator could be -- so backoff to not touch
  // terminator position if need be (this can make exact alignments
  // for full Doubles, but in some cases needs care not to access too
  // far to the left)

  pre=-pre+2;                           // gap width, including "0."
  t=cstart+ROUNDDOWN4(c-cstart)+pre;    // preferred first target point
  // backoff if too far to the right
  if (t>string+DECSTRING-5) t=string+DECSTRING-5; // adjust to fit
  // now shift the entire coefficient to the right, being careful not
  // to access to the left of string [cannot use memcpy]
  for (s=t-pre; s>=string; s-=4, t-=4) UBFROMUI(t, UBTOUI(s));
  // for Quads and Singles there may be a character or two left...
  s+=3;                                 // where next would come from
  for(; s>=cstart; s--, t--) *(t+3)=*(s);
  // now have fill 0. through 0.00000; use overlaps to avoid tests
  if (pre>=4) {
    memcpy(cstart+pre-4, "0000", 4);
    memcpy(cstart, "0.00", 4);
    }
   else { // 2 or 3
    *(cstart+pre-1)='0';
    memcpy(cstart, "0.", 2);
    }
  *(c+pre)='\0';                        // terminate
  return string;
  } // decFloatToString

/* ------------------------------------------------------------------ */
/* decFloatToWider -- conversion to next-wider format                 */
/*                                                                    */
/*  source  is the decFloat format number which gets the result of    */
/*          the conversion                                            */
/*  wider   is the decFloatWider format number which will be narrowed */
/*  returns wider                                                     */
/*                                                                    */
/* Widening is always exact; no status is set (sNaNs are copied and   */
/* do not signal).  The result will be canonical if the source is,    */
/* and may or may not be if the source is not.                        */
/* ------------------------------------------------------------------ */
// widening is not possible for decQuad format numbers; simply omit
#if !QUAD
decFloatWider * decFloatToWider(const decFloat *source, decFloatWider *wider) {
  uInt msd;

  /* Construct and copy the sign word */
  if (DFISSPECIAL(source)) {
    // copy sign, combination, and first bit of exponent (sNaN selector)
    DFWWORD(wider, 0)=DFWORD(source, 0)&0xfe000000;
    msd=0;
    }
   else { // is finite number
    uInt exp=GETEXPUN(source)+DECWBIAS; // get unbiased exponent and rebias
    uInt code=(exp>>DECWECONL)<<29;     // set two bits of exp [msd=0]
    code|=(exp<<(32-6-DECWECONL)) & 0x03ffffff; // add exponent continuation
    code|=DFWORD(source, 0)&0x80000000; // add sign
    DFWWORD(wider, 0)=code;             // .. and place top word in wider
    msd=GETMSD(source);                 // get source coefficient MSD [0-9]
    }
  /* Copy the coefficient and clear any 'unused' words to left */
  #if SINGLE
    DFWWORD(wider, 1)=(DFWORD(source, 0)&0x000fffff)|(msd<<20);
  #elif DOUBLE
    DFWWORD(wider, 2)=(DFWORD(source, 0)&0x0003ffff)|(msd<<18);
    DFWWORD(wider, 3)=DFWORD(source, 1);
    DFWWORD(wider, 1)=0;
  #endif
  return wider;
  } // decFloatToWider
#endif

/* ------------------------------------------------------------------ */
/* decFloatVersion -- return package version string                   */
/*                                                                    */
/*  returns a constant string describing this package                 */
/* ------------------------------------------------------------------ */
const char *decFloatVersion(void) {
  return DECVERSION;
  } // decFloatVersion

/* ------------------------------------------------------------------ */
/* decFloatZero -- set to canonical (integer) zero                    */
/*                                                                    */
/*  df is the decFloat format number to integer +0 (q=0, c=+0)        */
/*  returns df                                                        */
/*                                                                    */
/* No error is possible, and no status can be set.                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatZero(decFloat *df){
  DFWORD(df, 0)=ZEROWORD;     // set appropriate top word
  #if DOUBLE || QUAD
    DFWORD(df, 1)=0;
    #if QUAD
      DFWORD(df, 2)=0;
      DFWORD(df, 3)=0;
    #endif
  #endif
  // decFloatShow(df, "zero");
  return df;
  } // decFloatZero

/* ------------------------------------------------------------------ */
/* Private generic function (not format-specific) for development use */
/* ------------------------------------------------------------------ */
// This is included once only, for all to use
#if QUAD && (DECCHECK || DECTRACE)
  /* ---------------------------------------------------------------- */
  /* decShowNum -- display bcd8 number in debug form                  */
  /*                                                                  */
  /*   num is the bcdnum to display                                   */
  /*   tag is a string to label the display                           */
  /* ---------------------------------------------------------------- */
  void decShowNum(const bcdnum *num, const char *tag) {
    const char *csign="+";              // sign character
    uByte *ub;                          // work
    uInt  uiwork;                       // for macros
    if (num->sign==DECFLOAT_Sign) csign="-";

    printf(">%s> ", tag);
    if (num->exponent==DECFLOAT_Inf) printf("%sInfinity", csign);
    else if (num->exponent==DECFLOAT_qNaN) printf("%sqNaN", csign);
    else if (num->exponent==DECFLOAT_sNaN) printf("%ssNaN", csign);
    else {                              // finite
     char qbuf[10];                     // for right-aligned q
     char *c;                           // work
     const uByte *u;                    // ..
     Int e=num->exponent;               // .. exponent
     strcpy(qbuf, "q=");
     c=&qbuf[2];                        // where exponent will go
     // lay out the exponent
     if (e<0) {
       *c++='-';                        // add '-'
       e=-e;                            // uInt, please
       }
     #if DECEMAXD>4
       #error Exponent form is too long for ShowNum to lay out
     #endif
     if (e==0) *c++='0';                // 0-length case
      else if (e<1000) {                // 3 (or fewer) digits case
       u=&BIN2BCD8[e*4];                // -> 3 digits + length byte
       UBFROMUI(c, UBTOUI(u+3-*(u+3))|CHARMASK); // [as above]
       c+=*(u+3);                       // bump pointer appropriately
       }
      else {                            // 4-digits
       Int thou=((e>>3)*1049)>>17;      // e/1000
       Int rem=e-(1000*thou);           // e%1000
       *c++=(char)('0'+(char)thou);     // the thousands digit
       u=&BIN2BCD8[rem*4];              // -> 3 digits + length byte
       UBFROMUI(c, UBTOUI(u)|CHARMASK); // copy fixed 3+1 characters [is safe]
       c+=3;                            // bump pointer, always 3 digits
       }
     *c='\0';                           // add terminator
     printf("%7s c=%s", qbuf, csign);
     }

    if (!EXPISSPECIAL(num->exponent) || num->msd!=num->lsd || *num->lsd!=0) {
      for (ub=num->msd; ub<=num->lsd; ub++) { // coefficient...
        printf("%1x", *ub);
        if ((num->lsd-ub)%3==0 && ub!=num->lsd) printf(" "); // 4-space
        }
      }
    printf("\n");
    } // decShowNum
#endif
