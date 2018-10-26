/* ------------------------------------------------------------------ */
/* decBasic.c -- common base code for Basic decimal types             */
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
/* This module comprises code that is shared between decDouble and    */
/* decQuad (but not decSingle).  The main arithmetic operations are   */
/* here (Add, Subtract, Multiply, FMA, and Division operators).       */
/*                                                                    */
/* Unlike decNumber, parameterization takes place at compile time     */
/* rather than at runtime.  The parameters are set in the decDouble.c */
/* (etc.) files, which then include this one to produce the compiled  */
/* code.  The functions here, therefore, are code shared between      */
/* multiple formats.                                                  */
/*                                                                    */
/* This must be included after decCommon.c.                           */
/* ------------------------------------------------------------------ */
// Names here refer to decFloat rather than to decDouble, etc., and
// the functions are in strict alphabetical order.

// The compile-time flags SINGLE, DOUBLE, and QUAD are set up in
// decCommon.c
#if !defined(QUAD)
  #error decBasic.c must be included after decCommon.c
#endif
#if SINGLE
  #error Routines in decBasic.c are for decDouble and decQuad only
#endif

/* Private constants */
#define DIVIDE      0x80000000     // Divide operations [as flags]
#define REMAINDER   0x40000000     // ..
#define DIVIDEINT   0x20000000     // ..
#define REMNEAR     0x10000000     // ..

/* Private functions (local, used only by routines in this module) */
static decFloat *decDivide(decFloat *, const decFloat *,
                              const decFloat *, decContext *, uInt);
static decFloat *decCanonical(decFloat *, const decFloat *);
static void      decFiniteMultiply(bcdnum *, uByte *, const decFloat *,
                              const decFloat *);
static decFloat *decInfinity(decFloat *, const decFloat *);
static decFloat *decInvalid(decFloat *, decContext *);
static decFloat *decNaNs(decFloat *, const decFloat *, const decFloat *,
                              decContext *);
static Int       decNumCompare(const decFloat *, const decFloat *, Flag);
static decFloat *decToIntegral(decFloat *, const decFloat *, decContext *,
                              enum rounding, Flag);
static uInt      decToInt32(const decFloat *, decContext *, enum rounding,
                              Flag, Flag);

/* ------------------------------------------------------------------ */
/* decCanonical -- copy a decFloat, making canonical                  */
/*                                                                    */
/*   result gets the canonicalized df                                 */
/*   df     is the decFloat to copy and make canonical                */
/*   returns result                                                   */
/*                                                                    */
/* This is exposed via decFloatCanonical for Double and Quad only.    */
/* This works on specials, too; no error or exception is possible.    */
/* ------------------------------------------------------------------ */
static decFloat * decCanonical(decFloat *result, const decFloat *df) {
  uInt encode, precode, dpd;       // work
  uInt inword, uoff, canon;        // ..
  Int  n;                          // counter (down)
  if (df!=result) *result=*df;     // effect copy if needed
  if (DFISSPECIAL(result)) {
    if (DFISINF(result)) return decInfinity(result, df); // clean Infinity
    // is a NaN
    DFWORD(result, 0)&=~ECONNANMASK;    // clear ECON except selector
    if (DFISCCZERO(df)) return result;  // coefficient continuation is 0
    // drop through to check payload
    }
  // return quickly if the coefficient continuation is canonical
  { // declare block
  #if DOUBLE
    uInt sourhi=DFWORD(df, 0);
    uInt sourlo=DFWORD(df, 1);
    if (CANONDPDOFF(sourhi, 8)
     && CANONDPDTWO(sourhi, sourlo, 30)
     && CANONDPDOFF(sourlo, 20)
     && CANONDPDOFF(sourlo, 10)
     && CANONDPDOFF(sourlo, 0)) return result;
  #elif QUAD
    uInt sourhi=DFWORD(df, 0);
    uInt sourmh=DFWORD(df, 1);
    uInt sourml=DFWORD(df, 2);
    uInt sourlo=DFWORD(df, 3);
    if (CANONDPDOFF(sourhi, 4)
     && CANONDPDTWO(sourhi, sourmh, 26)
     && CANONDPDOFF(sourmh, 16)
     && CANONDPDOFF(sourmh, 6)
     && CANONDPDTWO(sourmh, sourml, 28)
     && CANONDPDOFF(sourml, 18)
     && CANONDPDOFF(sourml, 8)
     && CANONDPDTWO(sourml, sourlo, 30)
     && CANONDPDOFF(sourlo, 20)
     && CANONDPDOFF(sourlo, 10)
     && CANONDPDOFF(sourlo, 0)) return result;
  #endif
  } // block

  // Loop to repair a non-canonical coefficent, as needed
  inword=DECWORDS-1;               // current input word
  uoff=0;                          // bit offset of declet
  encode=DFWORD(result, inword);
  for (n=DECLETS-1; n>=0; n--) {   // count down declets of 10 bits
    dpd=encode>>uoff;
    uoff+=10;
    if (uoff>32) {                 // crossed uInt boundary
      inword--;
      encode=DFWORD(result, inword);
      uoff-=32;
      dpd|=encode<<(10-uoff);      // get pending bits
      }
    dpd&=0x3ff;                    // clear uninteresting bits
    if (dpd<0x16e) continue;       // must be canonical
    canon=BIN2DPD[DPD2BIN[dpd]];   // determine canonical declet
    if (canon==dpd) continue;      // have canonical declet
    // need to replace declet
    if (uoff>=10) {                // all within current word
      encode&=~(0x3ff<<(uoff-10)); // clear the 10 bits ready for replace
      encode|=canon<<(uoff-10);    // insert the canonical form
      DFWORD(result, inword)=encode;    // .. and save
      continue;
      }
    // straddled words
    precode=DFWORD(result, inword+1);   // get previous
    precode&=0xffffffff>>(10-uoff);     // clear top bits
    DFWORD(result, inword+1)=precode|(canon<<(32-(10-uoff)));
    encode&=0xffffffff<<uoff;           // clear bottom bits
    encode|=canon>>(10-uoff);           // insert canonical
    DFWORD(result, inword)=encode;      // .. and save
    } // n
  return result;
  } // decCanonical

/* ------------------------------------------------------------------ */
/* decDivide -- divide operations                                     */
/*                                                                    */
/*   result gets the result of dividing dfl by dfr:                   */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   op     is the operation selector                                 */
/*   returns result                                                   */
/*                                                                    */
/* op is one of DIVIDE, REMAINDER, DIVIDEINT, or REMNEAR.             */
/* ------------------------------------------------------------------ */
#define DIVCOUNT  0                // 1 to instrument subtractions counter
#define DIVBASE   ((uInt)BILLION)  // the base used for divide
#define DIVOPLEN  DECPMAX9         // operand length ('digits' base 10**9)
#define DIVACCLEN (DIVOPLEN*3)     // accumulator length (ditto)
static decFloat * decDivide(decFloat *result, const decFloat *dfl,
                            const decFloat *dfr, decContext *set, uInt op) {
  decFloat quotient;               // for remainders
  bcdnum num;                      // for final conversion
  uInt   acc[DIVACCLEN];           // coefficent in base-billion ..
  uInt   div[DIVOPLEN];            // divisor in base-billion ..
  uInt   quo[DIVOPLEN+1];          // quotient in base-billion ..
  uByte  bcdacc[(DIVOPLEN+1)*9+2]; // for quotient in BCD, +1, +1
  uInt   *msua, *msud, *msuq;      // -> msu of acc, div, and quo
  Int    divunits, accunits;       // lengths
  Int    quodigits;                // digits in quotient
  uInt   *lsua, *lsuq;             // -> current acc and quo lsus
  Int    length, multiplier;       // work
  uInt   carry, sign;              // ..
  uInt   *ua, *ud, *uq;            // ..
  uByte  *ub;                      // ..
  uInt   uiwork;                   // for macros
  uInt   divtop;                   // top unit of div adjusted for estimating
  #if DIVCOUNT
  static uInt maxcount=0;          // worst-seen subtractions count
  uInt   divcount=0;               // subtractions count [this divide]
  #endif

  // calculate sign
  num.sign=(DFWORD(dfl, 0)^DFWORD(dfr, 0)) & DECFLOAT_Sign;

  if (DFISSPECIAL(dfl) || DFISSPECIAL(dfr)) { // either is special?
    // NaNs are handled as usual
    if (DFISNAN(dfl) || DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    // one or two infinities
    if (DFISINF(dfl)) {
      if (DFISINF(dfr)) return decInvalid(result, set); // Two infinities bad
      if (op&(REMAINDER|REMNEAR)) return decInvalid(result, set); // as is rem
      // Infinity/x is infinite and quiet, even if x=0
      DFWORD(result, 0)=num.sign;
      return decInfinity(result, result);
      }
    // must be x/Infinity -- remainders are lhs
    if (op&(REMAINDER|REMNEAR)) return decCanonical(result, dfl);
    // divides: return zero with correct sign and exponent depending
    // on op (Etiny for divide, 0 for divideInt)
    decFloatZero(result);
    if (op==DIVIDEINT) DFWORD(result, 0)|=num.sign; // add sign
     else DFWORD(result, 0)=num.sign;        // zeros the exponent, too
    return result;
    }
  // next, handle zero operands (x/0 and 0/x)
  if (DFISZERO(dfr)) {                       // x/0
    if (DFISZERO(dfl)) {                     // 0/0 is undefined
      decFloatZero(result);
      DFWORD(result, 0)=DECFLOAT_qNaN;
      set->status|=DEC_Division_undefined;
      return result;
      }
    if (op&(REMAINDER|REMNEAR)) return decInvalid(result, set); // bad rem
    set->status|=DEC_Division_by_zero;
    DFWORD(result, 0)=num.sign;
    return decInfinity(result, result);      // x/0 -> signed Infinity
    }
  num.exponent=GETEXPUN(dfl)-GETEXPUN(dfr);  // ideal exponent
  if (DFISZERO(dfl)) {                       // 0/x (x!=0)
    // if divide, result is 0 with ideal exponent; divideInt has
    // exponent=0, remainders give zero with lower exponent
    if (op&DIVIDEINT) {
      decFloatZero(result);
      DFWORD(result, 0)|=num.sign;           // add sign
      return result;
      }
    if (!(op&DIVIDE)) {                      // a remainder
      // exponent is the minimum of the operands
      num.exponent=MINI(GETEXPUN(dfl), GETEXPUN(dfr));
      // if the result is zero the sign shall be sign of dfl
      num.sign=DFWORD(dfl, 0)&DECFLOAT_Sign;
      }
    bcdacc[0]=0;
    num.msd=bcdacc;                          // -> 0
    num.lsd=bcdacc;                          // ..
    return decFinalize(result, &num, set);   // [divide may clamp exponent]
    } // 0/x
  // [here, both operands are known to be finite and non-zero]

  // extract the operand coefficents into 'units' which are
  // base-billion; the lhs is high-aligned in acc and the msu of both
  // acc and div is at the right-hand end of array (offset length-1);
  // the quotient can need one more unit than the operands as digits
  // in it are not necessarily aligned neatly; further, the quotient
  // may not start accumulating until after the end of the initial
  // operand in acc if that is small (e.g., 1) so the accumulator
  // must have at least that number of units extra (at the ls end)
  GETCOEFFBILL(dfl, acc+DIVACCLEN-DIVOPLEN);
  GETCOEFFBILL(dfr, div);
  // zero the low uInts of acc
  acc[0]=0;
  acc[1]=0;
  acc[2]=0;
  acc[3]=0;
  #if DOUBLE
    #if DIVOPLEN!=2
      #error Unexpected Double DIVOPLEN
    #endif
  #elif QUAD
  acc[4]=0;
  acc[5]=0;
  acc[6]=0;
  acc[7]=0;
    #if DIVOPLEN!=4
      #error Unexpected Quad DIVOPLEN
    #endif
  #endif

  // set msu and lsu pointers
  msua=acc+DIVACCLEN-1;       // [leading zeros removed below]
  msuq=quo+DIVOPLEN;
  //[loop for div will terminate because operands are non-zero]
  for (msud=div+DIVOPLEN-1; *msud==0;) msud--;
  // the initial least-significant unit of acc is set so acc appears
  // to have the same length as div.
  // This moves one position towards the least possible for each
  // iteration
  divunits=(Int)(msud-div+1); // precalculate
  lsua=msua-divunits+1;       // initial working lsu of acc
  lsuq=msuq;                  // and of quo

  // set up the estimator for the multiplier; this is the msu of div,
  // plus two bits from the unit below (if any) rounded up by one if
  // there are any non-zero bits or units below that [the extra two
  // bits makes for a much better estimate when the top unit is small]
  divtop=*msud<<2;
  if (divunits>1) {
    uInt *um=msud-1;
    uInt d=*um;
    if (d>=750000000) {divtop+=3; d-=750000000;}
     else if (d>=500000000) {divtop+=2; d-=500000000;}
     else if (d>=250000000) {divtop++; d-=250000000;}
    if (d) divtop++;
     else for (um--; um>=div; um--) if (*um) {
      divtop++;
      break;
      }
    } // >1 unit

  #if DECTRACE
  {Int i;
  printf("----- div=");
  for (i=divunits-1; i>=0; i--) printf("%09ld ", (LI)div[i]);
  printf("\n");}
  #endif

  // now collect up to DECPMAX+1 digits in the quotient (this may
  // need OPLEN+1 uInts if unaligned)
  quodigits=0;                // no digits yet
  for (;; lsua--) {           // outer loop -- each input position
    #if DECCHECK
    if (lsua<acc) {
      printf("Acc underrun...\n");
      break;
      }
    #endif
    #if DECTRACE
    printf("Outer: quodigits=%ld acc=", (LI)quodigits);
    for (ua=msua; ua>=lsua; ua--) printf("%09ld ", (LI)*ua);
    printf("\n");
    #endif
    *lsuq=0;                  // default unit result is 0
    for (;;) {                // inner loop -- calculate quotient unit
      // strip leading zero units from acc (either there initially or
      // from subtraction below); this may strip all if exactly 0
      for (; *msua==0 && msua>=lsua;) msua--;
      accunits=(Int)(msua-lsua+1);                // [maybe 0]
      // subtraction is only necessary and possible if there are as
      // least as many units remaining in acc for this iteration as
      // there are in div
      if (accunits<divunits) {
        if (accunits==0) msua++;                  // restore
        break;
        }

      // If acc is longer than div then subtraction is definitely
      // possible (as msu of both is non-zero), but if they are the
      // same length a comparison is needed.
      // If a subtraction is needed then a good estimate of the
      // multiplier for the subtraction is also needed in order to
      // minimise the iterations of this inner loop because the
      // subtractions needed dominate division performance.
      if (accunits==divunits) {
        // compare the high divunits of acc and div:
        // acc<div:  this quotient unit is unchanged; subtraction
        //           will be possible on the next iteration
        // acc==div: quotient gains 1, set acc=0
        // acc>div:  subtraction necessary at this position
        for (ud=msud, ua=msua; ud>div; ud--, ua--) if (*ud!=*ua) break;
        // [now at first mismatch or lsu]
        if (*ud>*ua) break;                       // next time...
        if (*ud==*ua) {                           // all compared equal
          *lsuq+=1;                               // increment result
          msua=lsua;                              // collapse acc units
          *msua=0;                                // .. to a zero
          break;
          }

        // subtraction necessary; estimate multiplier [see above]
        // if both *msud and *msua are small it is cost-effective to
        // bring in part of the following units (if any) to get a
        // better estimate (assume some other non-zero in div)
        #define DIVLO 1000000U
        #define DIVHI (DIVBASE/DIVLO)
        #if DECUSE64
          if (divunits>1) {
            // there cannot be a *(msud-2) for DECDOUBLE so next is
            // an exact calculation unless DECQUAD (which needs to
            // assume bits out there if divunits>2)
            uLong mul=(uLong)*msua * DIVBASE + *(msua-1);
            uLong div=(uLong)*msud * DIVBASE + *(msud-1);
            #if QUAD
            if (divunits>2) div++;
            #endif
            mul/=div;
            multiplier=(Int)mul;
            }
           else multiplier=*msua/(*msud);
        #else
          if (divunits>1 && *msua<DIVLO && *msud<DIVLO) {
            multiplier=(*msua*DIVHI + *(msua-1)/DIVLO)
                      /(*msud*DIVHI + *(msud-1)/DIVLO +1);
            }
           else multiplier=(*msua<<2)/divtop;
        #endif
        }
       else {                                     // accunits>divunits
        // msud is one unit 'lower' than msua, so estimate differently
        #if DECUSE64
          uLong mul;
          // as before, bring in extra digits if possible
          if (divunits>1 && *msua<DIVLO && *msud<DIVLO) {
            mul=((uLong)*msua * DIVHI * DIVBASE) + *(msua-1) * DIVHI
               + *(msua-2)/DIVLO;
            mul/=(*msud*DIVHI + *(msud-1)/DIVLO +1);
            }
           else if (divunits==1) {
            mul=(uLong)*msua * DIVBASE + *(msua-1);
            mul/=*msud;       // no more to the right
            }
           else {
            mul=(uLong)(*msua) * (uInt)(DIVBASE<<2)
                + (*(msua-1)<<2);
            mul/=divtop;      // [divtop already allows for sticky bits]
            }
          multiplier=(Int)mul;
        #else
          multiplier=*msua * ((DIVBASE<<2)/divtop);
        #endif
        }
      if (multiplier==0) multiplier=1;            // marginal case
      *lsuq+=multiplier;

      #if DIVCOUNT
      // printf("Multiplier: %ld\n", (LI)multiplier);
      divcount++;
      #endif

      // Carry out the subtraction  acc-(div*multiplier); for each
      // unit in div, do the multiply, split to units (see
      // decFloatMultiply for the algorithm), and subtract from acc
      #define DIVMAGIC  2305843009U               // 2**61/10**9
      #define DIVSHIFTA 29
      #define DIVSHIFTB 32
      carry=0;
      for (ud=div, ua=lsua; ud<=msud; ud++, ua++) {
        uInt lo, hop;
        #if DECUSE64
          uLong sub=(uLong)multiplier*(*ud)+carry;
          if (sub<DIVBASE) {
            carry=0;
            lo=(uInt)sub;
            }
           else {
            hop=(uInt)(sub>>DIVSHIFTA);
            carry=(uInt)(((uLong)hop*DIVMAGIC)>>DIVSHIFTB);
            // the estimate is now in hi; now calculate sub-hi*10**9
            // to get the remainder (which will be <DIVBASE))
            lo=(uInt)sub;
            lo-=carry*DIVBASE;                    // low word of result
            if (lo>=DIVBASE) {
              lo-=DIVBASE;                        // correct by +1
              carry++;
              }
            }
        #else // 32-bit
          uInt hi;
          // calculate multiplier*(*ud) into hi and lo
          LONGMUL32HI(hi, *ud, multiplier);       // get the high word
          lo=multiplier*(*ud);                    // .. and the low
          lo+=carry;                              // add the old hi
          carry=hi+(lo<carry);                    // .. with any carry
          if (carry || lo>=DIVBASE) {             // split is needed
            hop=(carry<<3)+(lo>>DIVSHIFTA);       // hi:lo/2**29
            LONGMUL32HI(carry, hop, DIVMAGIC);    // only need the high word
            // [DIVSHIFTB is 32, so carry can be used directly]
            // the estimate is now in carry; now calculate hi:lo-est*10**9;
            // happily the top word of the result is irrelevant because it
            // will always be zero so this needs only one multiplication
            lo-=(carry*DIVBASE);
            // the correction here will be at most +1; do it
            if (lo>=DIVBASE) {
              lo-=DIVBASE;
              carry++;
              }
            }
        #endif
        if (lo>*ua) {              // borrow needed
          *ua+=DIVBASE;
          carry++;
          }
        *ua-=lo;
        } // ud loop
      if (carry) *ua-=carry;       // accdigits>divdigits [cannot borrow]
      } // inner loop

    // the outer loop terminates when there is either an exact result
    // or enough digits; first update the quotient digit count and
    // pointer (if any significant digits)
    #if DECTRACE
    if (*lsuq || quodigits) printf("*lsuq=%09ld\n", (LI)*lsuq);
    #endif
    if (quodigits) {
      quodigits+=9;                // had leading unit earlier
      lsuq--;
      if (quodigits>DECPMAX+1) break;   // have enough
      }
     else if (*lsuq) {             // first quotient digits
      const uInt *pow;
      for (pow=DECPOWERS; *lsuq>=*pow; pow++) quodigits++;
      lsuq--;
      // [cannot have >DECPMAX+1 on first unit]
      }

    if (*msua!=0) continue;        // not an exact result
    // acc is zero iff used all of original units and zero down to lsua
    // (must also continue to original lsu for correct quotient length)
    if (lsua>acc+DIVACCLEN-DIVOPLEN) continue;
    for (; msua>lsua && *msua==0;) msua--;
    if (*msua==0 && msua==lsua) break;
    } // outer loop

  // all of the original operand in acc has been covered at this point
  // quotient now has at least DECPMAX+2 digits
  // *msua is now non-0 if inexact and sticky bits
  // lsuq is one below the last uint of the quotient
  lsuq++;                          // set -> true lsu of quo
  if (*msua) *lsuq|=1;             // apply sticky bit

  // quo now holds the (unrounded) quotient in base-billion; one
  // base-billion 'digit' per uInt.
  #if DECTRACE
  printf("DivQuo:");
  for (uq=msuq; uq>=lsuq; uq--) printf(" %09ld", (LI)*uq);
  printf("\n");
  #endif

  // Now convert to BCD for rounding and cleanup, starting from the
  // most significant end [offset by one into bcdacc to leave room
  // for a possible carry digit if rounding for REMNEAR is needed]
  for (uq=msuq, ub=bcdacc+1; uq>=lsuq; uq--, ub+=9) {
    uInt top, mid, rem;                 // work
    if (*uq==0) {                       // no split needed
      UBFROMUI(ub, 0);                  // clear 9 BCD8s
      UBFROMUI(ub+4, 0);                // ..
      *(ub+8)=0;                        // ..
      continue;
      }
    // *uq is non-zero -- split the base-billion digit into
    // hi, mid, and low three-digits
    #define divsplit9 1000000           // divisor
    #define divsplit6 1000              // divisor
    // The splitting is done by simple divides and remainders,
    // assuming the compiler will optimize these [GCC does]
    top=*uq/divsplit9;
    rem=*uq%divsplit9;
    mid=rem/divsplit6;
    rem=rem%divsplit6;
    // lay out the nine BCD digits (plus one unwanted byte)
    UBFROMUI(ub,   UBTOUI(&BIN2BCD8[top*4]));
    UBFROMUI(ub+3, UBTOUI(&BIN2BCD8[mid*4]));
    UBFROMUI(ub+6, UBTOUI(&BIN2BCD8[rem*4]));
    } // BCD conversion loop
  ub--;                                 // -> lsu

  // complete the bcdnum; quodigits is correct, so the position of
  // the first non-zero is known
  num.msd=bcdacc+1+(msuq-lsuq+1)*9-quodigits;
  num.lsd=ub;

  // make exponent adjustments, etc
  if (lsua<acc+DIVACCLEN-DIVOPLEN) {    // used extra digits
    num.exponent-=(Int)((acc+DIVACCLEN-DIVOPLEN-lsua)*9);
    // if the result was exact then there may be up to 8 extra
    // trailing zeros in the overflowed quotient final unit
    if (*msua==0) {
      for (; *ub==0;) ub--;             // drop zeros
      num.exponent+=(Int)(num.lsd-ub);  // and adjust exponent
      num.lsd=ub;
      }
    } // adjustment needed

  #if DIVCOUNT
  if (divcount>maxcount) {              // new high-water nark
    maxcount=divcount;
    printf("DivNewMaxCount: %ld\n", (LI)maxcount);
    }
  #endif

  if (op&DIVIDE) return decFinalize(result, &num, set); // all done

  // Is DIVIDEINT or a remainder; there is more to do -- first form
  // the integer (this is done 'after the fact', unlike as in
  // decNumber, so as not to tax DIVIDE)

  // The first non-zero digit will be in the first 9 digits, known
  // from quodigits and num.msd, so there is always space for DECPMAX
  // digits

  length=(Int)(num.lsd-num.msd+1);
  //printf("Length exp: %ld %ld\n", (LI)length, (LI)num.exponent);

  if (length+num.exponent>DECPMAX) { // cannot fit
    decFloatZero(result);
    DFWORD(result, 0)=DECFLOAT_qNaN;
    set->status|=DEC_Division_impossible;
    return result;
    }

  if (num.exponent>=0) {           // already an int, or need pad zeros
    for (ub=num.lsd+1; ub<=num.lsd+num.exponent; ub++) *ub=0;
    num.lsd+=num.exponent;
    }
   else {                          // too long: round or truncate needed
    Int drop=-num.exponent;
    if (!(op&REMNEAR)) {           // simple truncate
      num.lsd-=drop;
      if (num.lsd<num.msd) {       // truncated all
        num.lsd=num.msd;           // make 0
        *num.lsd=0;                // .. [sign still relevant]
        }
      }
     else {                        // round to nearest even [sigh]
      // round-to-nearest, in-place; msd is at or to right of bcdacc+1
      // (this is a special case of Quantize -- q.v. for commentary)
      uByte *roundat;              // -> re-round digit
      uByte reround;               // reround value
      *(num.msd-1)=0;              // in case of left carry, or make 0
      if (drop<length) roundat=num.lsd-drop+1;
       else if (drop==length) roundat=num.msd;
       else roundat=num.msd-1;     // [-> 0]
      reround=*roundat;
      for (ub=roundat+1; ub<=num.lsd; ub++) {
        if (*ub!=0) {
          reround=DECSTICKYTAB[reround];
          break;
          }
        } // check stickies
      if (roundat>num.msd) num.lsd=roundat-1;
       else {
        num.msd--;                           // use the 0 ..
        num.lsd=num.msd;                     // .. at the new MSD place
        }
      if (reround!=0) {                      // discarding non-zero
        uInt bump=0;
        // rounding is DEC_ROUND_HALF_EVEN always
        if (reround>5) bump=1;               // >0.5 goes up
         else if (reround==5)                // exactly 0.5000 ..
          bump=*(num.lsd) & 0x01;            // .. up iff [new] lsd is odd
        if (bump!=0) {                       // need increment
          // increment the coefficient; this might end up with 1000...
          ub=num.lsd;
          for (; UBTOUI(ub-3)==0x09090909; ub-=4) UBFROMUI(ub-3, 0);
          for (; *ub==9; ub--) *ub=0;        // at most 3 more
          *ub+=1;
          if (ub<num.msd) num.msd--;         // carried
          } // bump needed
        } // reround!=0
      } // remnear
    } // round or truncate needed
  num.exponent=0;                            // all paths
  //decShowNum(&num, "int");

  if (op&DIVIDEINT) return decFinalize(result, &num, set); // all done

  // Have a remainder to calculate
  decFinalize(&quotient, &num, set);         // lay out the integer so far
  DFWORD(&quotient, 0)^=DECFLOAT_Sign;       // negate it
  sign=DFWORD(dfl, 0);                       // save sign of dfl
  decFloatFMA(result, &quotient, dfr, dfl, set);
  if (!DFISZERO(result)) return result;
  // if the result is zero the sign shall be sign of dfl
  DFWORD(&quotient, 0)=sign;                 // construct decFloat of sign
  return decFloatCopySign(result, result, &quotient);
  } // decDivide

/* ------------------------------------------------------------------ */
/* decFiniteMultiply -- multiply two finite decFloats                 */
/*                                                                    */
/*   num    gets the result of multiplying dfl and dfr                */
/*   bcdacc .. with the coefficient in this array                     */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*                                                                    */
/* This effects the multiplication of two decFloats, both known to be */
/* finite, leaving the result in a bcdnum ready for decFinalize (for  */
/* use in Multiply) or in a following addition (FMA).                 */
/*                                                                    */
/* bcdacc must have space for at least DECPMAX9*18+1 bytes.           */
/* No error is possible and no status is set.                         */
/* ------------------------------------------------------------------ */
// This routine has two separate implementations of the core
// multiplication; both using base-billion.  One uses only 32-bit
// variables (Ints and uInts) or smaller; the other uses uLongs (for
// multiplication and addition only).  Both implementations cover
// both arithmetic sizes (DOUBLE and QUAD) in order to allow timing
// comparisons.  In any one compilation only one implementation for
// each size can be used, and if DECUSE64 is 0 then use of the 32-bit
// version is forced.
//
// Historical note: an earlier version of this code also supported the
// 256-bit format and has been preserved.  That is somewhat trickier
// during lazy carry splitting because the initial quotient estimate
// (est) can exceed 32 bits.

#define MULTBASE  ((uInt)BILLION)  // the base used for multiply
#define MULOPLEN  DECPMAX9         // operand length ('digits' base 10**9)
#define MULACCLEN (MULOPLEN*2)              // accumulator length (ditto)
#define LEADZEROS (MULACCLEN*9 - DECPMAX*2) // leading zeros always

// Assertions: exponent not too large and MULACCLEN is a multiple of 4
#if DECEMAXD>9
  #error Exponent may overflow when doubled for Multiply
#endif
#if MULACCLEN!=(MULACCLEN/4)*4
  // This assumption is used below only for initialization
  #error MULACCLEN is not a multiple of 4
#endif

static void decFiniteMultiply(bcdnum *num, uByte *bcdacc,
                              const decFloat *dfl, const decFloat *dfr) {
  uInt   bufl[MULOPLEN];           // left  coefficient (base-billion)
  uInt   bufr[MULOPLEN];           // right coefficient (base-billion)
  uInt   *ui, *uj;                 // work
  uByte  *ub;                      // ..
  uInt   uiwork;                   // for macros

  #if DECUSE64
  uLong  accl[MULACCLEN];          // lazy accumulator (base-billion+)
  uLong  *pl;                      // work -> lazy accumulator
  uInt   acc[MULACCLEN];           // coefficent in base-billion ..
  #else
  uInt   acc[MULACCLEN*2];         // accumulator in base-billion ..
  #endif
  uInt   *pa;                      // work -> accumulator
  //printf("Base10**9: OpLen=%d MulAcclen=%d\n", OPLEN, MULACCLEN);

  /* Calculate sign and exponent */
  num->sign=(DFWORD(dfl, 0)^DFWORD(dfr, 0)) & DECFLOAT_Sign;
  num->exponent=GETEXPUN(dfl)+GETEXPUN(dfr); // [see assertion above]

  /* Extract the coefficients and prepare the accumulator */
  // the coefficients of the operands are decoded into base-billion
  // numbers in uInt arrays (bufl and bufr, LSD at offset 0) of the
  // appropriate size.
  GETCOEFFBILL(dfl, bufl);
  GETCOEFFBILL(dfr, bufr);
  #if DECTRACE && 0
    printf("CoeffbL:");
    for (ui=bufl+MULOPLEN-1; ui>=bufl; ui--) printf(" %08lx", (LI)*ui);
    printf("\n");
    printf("CoeffbR:");
    for (uj=bufr+MULOPLEN-1; uj>=bufr; uj--) printf(" %08lx", (LI)*uj);
    printf("\n");
  #endif

  // start the 64-bit/32-bit differing paths...
#if DECUSE64

  // zero the accumulator
  #if MULACCLEN==4
    accl[0]=0; accl[1]=0; accl[2]=0; accl[3]=0;
  #else                                      // use a loop
    // MULACCLEN is a multiple of four, asserted above
    for (pl=accl; pl<accl+MULACCLEN; pl+=4) {
      *pl=0; *(pl+1)=0; *(pl+2)=0; *(pl+3)=0;// [reduce overhead]
      } // pl
  #endif

  /* Effect the multiplication */
  // The multiplcation proceeds using MFC's lazy-carry resolution
  // algorithm from decNumber.  First, the multiplication is
  // effected, allowing accumulation of the partial products (which
  // are in base-billion at each column position) into 64 bits
  // without resolving back to base=billion after each addition.
  // These 64-bit numbers (which may contain up to 19 decimal digits)
  // are then split using the Clark & Cowlishaw algorithm (see below).
  // [Testing for 0 in the inner loop is not really a 'win']
  for (ui=bufr; ui<bufr+MULOPLEN; ui++) { // over each item in rhs
    if (*ui==0) continue;                 // product cannot affect result
    pl=accl+(ui-bufr);                    // where to add the lhs
    for (uj=bufl; uj<bufl+MULOPLEN; uj++, pl++) { // over each item in lhs
      // if (*uj==0) continue;            // product cannot affect result
      *pl+=((uLong)*ui)*(*uj);
      } // uj
    } // ui

  // The 64-bit carries must now be resolved; this means that a
  // quotient/remainder has to be calculated for base-billion (1E+9).
  // For this, Clark & Cowlishaw's quotient estimation approach (also
  // used in decNumber) is needed, because 64-bit divide is generally
  // extremely slow on 32-bit machines, and may be slower than this
  // approach even on 64-bit machines.  This algorithm splits X
  // using:
  //
  //   magic=2**(A+B)/1E+9;   // 'magic number'
  //   hop=X/2**A;            // high order part of X (by shift)
  //   est=magic*hop/2**B     // quotient estimate (may be low by 1)
  //
  // A and B are quite constrained; hop and magic must fit in 32 bits,
  // and 2**(A+B) must be as large as possible (which is 2**61 if
  // magic is to fit).  Further, maxX increases with the length of
  // the operands (and hence the number of partial products
  // accumulated); maxX is OPLEN*(10**18), which is up to 19 digits.
  //
  // It can be shown that when OPLEN is 2 then the maximum error in
  // the estimated quotient is <1, but for larger maximum x the
  // maximum error is above 1 so a correction that is >1 may be
  // needed.  Values of A and B are chosen to satisfy the constraints
  // just mentioned while minimizing the maximum error (and hence the
  // maximum correction), as shown in the following table:
  //
  //   Type    OPLEN   A   B     maxX    maxError  maxCorrection
  //   ---------------------------------------------------------
  //   DOUBLE    2    29  32  <2*10**18    0.63       1
  //   QUAD      4    30  31  <4*10**18    1.17       2
  //
  // In the OPLEN==2 case there is most choice, but the value for B
  // of 32 has a big advantage as then the calculation of the
  // estimate requires no shifting; the compiler can extract the high
  // word directly after multiplying magic*hop.
  #define MULMAGIC 2305843009U          // 2**61/10**9  [both cases]
  #if DOUBLE
    #define MULSHIFTA 29
    #define MULSHIFTB 32
  #elif QUAD
    #define MULSHIFTA 30
    #define MULSHIFTB 31
  #else
    #error Unexpected type
  #endif

  #if DECTRACE
  printf("MulAccl:");
  for (pl=accl+MULACCLEN-1; pl>=accl; pl--)
    printf(" %08lx:%08lx", (LI)(*pl>>32), (LI)(*pl&0xffffffff));
  printf("\n");
  #endif

  for (pl=accl, pa=acc; pl<accl+MULACCLEN; pl++, pa++) { // each column position
    uInt lo, hop;                       // work
    uInt est;                           // cannot exceed 4E+9
    if (*pl>=MULTBASE) {
      // *pl holds a binary number which needs to be split
      hop=(uInt)(*pl>>MULSHIFTA);
      est=(uInt)(((uLong)hop*MULMAGIC)>>MULSHIFTB);
      // the estimate is now in est; now calculate hi:lo-est*10**9;
      // happily the top word of the result is irrelevant because it
      // will always be zero so this needs only one multiplication
      lo=(uInt)(*pl-((uLong)est*MULTBASE));  // low word of result
      // If QUAD, the correction here could be +2
      if (lo>=MULTBASE) {
        lo-=MULTBASE;                   // correct by +1
        est++;
        #if QUAD
        // may need to correct by +2
        if (lo>=MULTBASE) {
          lo-=MULTBASE;
          est++;
          }
        #endif
        }
      // finally place lo as the new coefficient 'digit' and add est to
      // the next place up [this is safe because this path is never
      // taken on the final iteration as *pl will fit]
      *pa=lo;
      *(pl+1)+=est;
      } // *pl needed split
     else {                             // *pl<MULTBASE
      *pa=(uInt)*pl;                    // just copy across
      }
    } // pl loop

#else  // 32-bit
  for (pa=acc;; pa+=4) {                     // zero the accumulator
    *pa=0; *(pa+1)=0; *(pa+2)=0; *(pa+3)=0;  // [reduce overhead]
    if (pa==acc+MULACCLEN*2-4) break;        // multiple of 4 asserted
    } // pa

  /* Effect the multiplication */
  // uLongs are not available (and in particular, there is no uLong
  // divide) but it is still possible to use MFC's lazy-carry
  // resolution algorithm from decNumber.  First, the multiplication
  // is effected, allowing accumulation of the partial products
  // (which are in base-billion at each column position) into 64 bits
  // [with the high-order 32 bits in each position being held at
  // offset +ACCLEN from the low-order 32 bits in the accumulator].
  // These 64-bit numbers (which may contain up to 19 decimal digits)
  // are then split using the Clark & Cowlishaw algorithm (see
  // below).
  for (ui=bufr;; ui++) {                // over each item in rhs
    uInt hi, lo;                        // words of exact multiply result
    pa=acc+(ui-bufr);                   // where to add the lhs
    for (uj=bufl;; uj++, pa++) {        // over each item in lhs
      LONGMUL32HI(hi, *ui, *uj);        // calculate product of digits
      lo=(*ui)*(*uj);                   // ..
      *pa+=lo;                          // accumulate low bits and ..
      *(pa+MULACCLEN)+=hi+(*pa<lo);     // .. high bits with any carry
      if (uj==bufl+MULOPLEN-1) break;
      }
    if (ui==bufr+MULOPLEN-1) break;
    }

  // The 64-bit carries must now be resolved; this means that a
  // quotient/remainder has to be calculated for base-billion (1E+9).
  // For this, Clark & Cowlishaw's quotient estimation approach (also
  // used in decNumber) is needed, because 64-bit divide is generally
  // extremely slow on 32-bit machines.  This algorithm splits X
  // using:
  //
  //   magic=2**(A+B)/1E+9;   // 'magic number'
  //   hop=X/2**A;            // high order part of X (by shift)
  //   est=magic*hop/2**B     // quotient estimate (may be low by 1)
  //
  // A and B are quite constrained; hop and magic must fit in 32 bits,
  // and 2**(A+B) must be as large as possible (which is 2**61 if
  // magic is to fit).  Further, maxX increases with the length of
  // the operands (and hence the number of partial products
  // accumulated); maxX is OPLEN*(10**18), which is up to 19 digits.
  //
  // It can be shown that when OPLEN is 2 then the maximum error in
  // the estimated quotient is <1, but for larger maximum x the
  // maximum error is above 1 so a correction that is >1 may be
  // needed.  Values of A and B are chosen to satisfy the constraints
  // just mentioned while minimizing the maximum error (and hence the
  // maximum correction), as shown in the following table:
  //
  //   Type    OPLEN   A   B     maxX    maxError  maxCorrection
  //   ---------------------------------------------------------
  //   DOUBLE    2    29  32  <2*10**18    0.63       1
  //   QUAD      4    30  31  <4*10**18    1.17       2
  //
  // In the OPLEN==2 case there is most choice, but the value for B
  // of 32 has a big advantage as then the calculation of the
  // estimate requires no shifting; the high word is simply
  // calculated from multiplying magic*hop.
  #define MULMAGIC 2305843009U          // 2**61/10**9  [both cases]
  #if DOUBLE
    #define MULSHIFTA 29
    #define MULSHIFTB 32
  #elif QUAD
    #define MULSHIFTA 30
    #define MULSHIFTB 31
  #else
    #error Unexpected type
  #endif

  #if DECTRACE
  printf("MulHiLo:");
  for (pa=acc+MULACCLEN-1; pa>=acc; pa--)
    printf(" %08lx:%08lx", (LI)*(pa+MULACCLEN), (LI)*pa);
  printf("\n");
  #endif

  for (pa=acc;; pa++) {                 // each low uInt
    uInt hi, lo;                        // words of exact multiply result
    uInt hop, estlo;                    // work
    #if QUAD
    uInt esthi;                         // ..
    #endif

    lo=*pa;
    hi=*(pa+MULACCLEN);                 // top 32 bits
    // hi and lo now hold a binary number which needs to be split

    #if DOUBLE
      hop=(hi<<3)+(lo>>MULSHIFTA);      // hi:lo/2**29
      LONGMUL32HI(estlo, hop, MULMAGIC);// only need the high word
      // [MULSHIFTB is 32, so estlo can be used directly]
      // the estimate is now in estlo; now calculate hi:lo-est*10**9;
      // happily the top word of the result is irrelevant because it
      // will always be zero so this needs only one multiplication
      lo-=(estlo*MULTBASE);
      // esthi=0;                       // high word is ignored below
      // the correction here will be at most +1; do it
      if (lo>=MULTBASE) {
        lo-=MULTBASE;
        estlo++;
        }
    #elif QUAD
      hop=(hi<<2)+(lo>>MULSHIFTA);      // hi:lo/2**30
      LONGMUL32HI(esthi, hop, MULMAGIC);// shift will be 31 ..
      estlo=hop*MULMAGIC;               // .. so low word needed
      estlo=(esthi<<1)+(estlo>>MULSHIFTB); // [just the top bit]
      // esthi=0;                       // high word is ignored below
      lo-=(estlo*MULTBASE);             // as above
      // the correction here could be +1 or +2
      if (lo>=MULTBASE) {
        lo-=MULTBASE;
        estlo++;
        }
      if (lo>=MULTBASE) {
        lo-=MULTBASE;
        estlo++;
        }
    #else
      #error Unexpected type
    #endif

    // finally place lo as the new accumulator digit and add est to
    // the next place up; this latter add could cause a carry of 1
    // to the high word of the next place
    *pa=lo;
    *(pa+1)+=estlo;
    // esthi is always 0 for DOUBLE and QUAD so this is skipped
    // *(pa+1+MULACCLEN)+=esthi;
    if (*(pa+1)<estlo) *(pa+1+MULACCLEN)+=1; // carry
    if (pa==acc+MULACCLEN-2) break;          // [MULACCLEN-1 will never need split]
    } // pa loop
#endif

  // At this point, whether using the 64-bit or the 32-bit paths, the
  // accumulator now holds the (unrounded) result in base-billion;
  // one base-billion 'digit' per uInt.
  #if DECTRACE
  printf("MultAcc:");
  for (pa=acc+MULACCLEN-1; pa>=acc; pa--) printf(" %09ld", (LI)*pa);
  printf("\n");
  #endif

  // Now convert to BCD for rounding and cleanup, starting from the
  // most significant end
  pa=acc+MULACCLEN-1;
  if (*pa!=0) num->msd=bcdacc+LEADZEROS;// drop known lead zeros
   else {                               // >=1 word of leading zeros
    num->msd=bcdacc;                    // known leading zeros are gone
    pa--;                               // skip first word ..
    for (; *pa==0; pa--) if (pa==acc) break; // .. and any more leading 0s
    }
  for (ub=bcdacc;; pa--, ub+=9) {
    if (*pa!=0) {                       // split(s) needed
      uInt top, mid, rem;               // work
      // *pa is non-zero -- split the base-billion acc digit into
      // hi, mid, and low three-digits
      #define mulsplit9 1000000         // divisor
      #define mulsplit6 1000            // divisor
      // The splitting is done by simple divides and remainders,
      // assuming the compiler will optimize these where useful
      // [GCC does]
      top=*pa/mulsplit9;
      rem=*pa%mulsplit9;
      mid=rem/mulsplit6;
      rem=rem%mulsplit6;
      // lay out the nine BCD digits (plus one unwanted byte)
      UBFROMUI(ub,   UBTOUI(&BIN2BCD8[top*4]));
      UBFROMUI(ub+3, UBTOUI(&BIN2BCD8[mid*4]));
      UBFROMUI(ub+6, UBTOUI(&BIN2BCD8[rem*4]));
      }
     else {                             // *pa==0
      UBFROMUI(ub, 0);                  // clear 9 BCD8s
      UBFROMUI(ub+4, 0);                // ..
      *(ub+8)=0;                        // ..
      }
    if (pa==acc) break;
    } // BCD conversion loop

  num->lsd=ub+8;                        // complete the bcdnum ..

  #if DECTRACE
  decShowNum(num, "postmult");
  decFloatShow(dfl, "dfl");
  decFloatShow(dfr, "dfr");
  #endif
  return;
  } // decFiniteMultiply

/* ------------------------------------------------------------------ */
/* decFloatAbs -- absolute value, heeding NaNs, etc.                  */
/*                                                                    */
/*   result gets the canonicalized df with sign 0                     */
/*   df     is the decFloat to abs                                    */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This has the same effect as decFloatPlus unless df is negative,    */
/* in which case it has the same effect as decFloatMinus.  The        */
/* effect is also the same as decFloatCopyAbs except that NaNs are    */
/* handled normally (the sign of a NaN is not affected, and an sNaN   */
/* will signal) and the result will be canonical.                     */
/* ------------------------------------------------------------------ */
decFloat * decFloatAbs(decFloat *result, const decFloat *df,
                       decContext *set) {
  if (DFISNAN(df)) return decNaNs(result, df, NULL, set);
  decCanonical(result, df);             // copy and check
  DFBYTE(result, 0)&=~0x80;             // zero sign bit
  return result;
  } // decFloatAbs

/* ------------------------------------------------------------------ */
/* decFloatAdd -- add two decFloats                                   */
/*                                                                    */
/*   result gets the result of adding dfl and dfr:                    */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
#if QUAD
// Table for testing MSDs for fastpath elimination; returns the MSD of
// a decDouble or decQuad (top 6 bits tested) ignoring the sign.
// Infinities return -32 and NaNs return -128 so that summing the two
// MSDs also allows rapid tests for the Specials (see code below).
const Int DECTESTMSD[64]={
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5,   6,    7,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 9, 8, 9, -32, -128,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5,   6,    7,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 9, 8, 9, -32, -128};
#else
// The table for testing MSDs is shared between the modules
extern const Int DECTESTMSD[64];
#endif

decFloat * decFloatAdd(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  bcdnum num;                      // for final conversion
  Int    bexpl, bexpr;             // left and right biased exponents
  uByte  *ub, *us, *ut;            // work
  uInt   uiwork;                   // for macros
  #if QUAD
  uShort uswork;                   // ..
  #endif

  uInt sourhil, sourhir;           // top words from source decFloats
                                   // [valid only through end of
                                   // fastpath code -- before swap]
  uInt diffsign;                   // non-zero if signs differ
  uInt carry;                      // carry: 0 or 1 before add loop
  Int  overlap;                    // coefficient overlap (if full)
  Int  summ;                       // sum of the MSDs
  // the following buffers hold coefficients with various alignments
  // (see commentary and diagrams below)
  uByte acc[4+2+DECPMAX*3+8];
  uByte buf[4+2+DECPMAX*2];
  uByte *umsd, *ulsd;              // local MSD and LSD pointers

  #if DECLITEND
    #define CARRYPAT 0x01000000    // carry=1 pattern
  #else
    #define CARRYPAT 0x00000001    // carry=1 pattern
  #endif

  /* Start decoding the arguments */
  // The initial exponents are placed into the opposite Ints to
  // that which might be expected; there are two sets of data to
  // keep track of (each decFloat and the corresponding exponent),
  // and this scheme means that at the swap point (after comparing
  // exponents) only one pair of words needs to be swapped
  // whichever path is taken (thereby minimising worst-case path).
  // The calculated exponents will be nonsense when the arguments are
  // Special, but are not used in that path
  sourhil=DFWORD(dfl, 0);          // LHS top word
  summ=DECTESTMSD[sourhil>>26];    // get first MSD for testing
  bexpr=DECCOMBEXP[sourhil>>26];   // get exponent high bits (in place)
  bexpr+=GETECON(dfl);             // .. + continuation

  sourhir=DFWORD(dfr, 0);          // RHS top word
  summ+=DECTESTMSD[sourhir>>26];   // sum MSDs for testing
  bexpl=DECCOMBEXP[sourhir>>26];
  bexpl+=GETECON(dfr);

  // here bexpr has biased exponent from lhs, and vice versa

  diffsign=(sourhil^sourhir)&DECFLOAT_Sign;

  // now determine whether to take a fast path or the full-function
  // slow path.  The slow path must be taken when:
  //   -- both numbers are finite, and:
  //         the exponents are different, or
  //         the signs are different, or
  //         the sum of the MSDs is >8 (hence might overflow)
  // specialness and the sum of the MSDs can be tested at once using
  // the summ value just calculated, so the test for specials is no
  // longer on the worst-case path (as of 3.60)

  if (summ<=8) {                   // MSD+MSD is good, or there is a special
    if (summ<0) {                  // there is a special
      // Inf+Inf would give -64; Inf+finite is -32 or higher
      if (summ<-64) return decNaNs(result, dfl, dfr, set);  // one or two NaNs
      // two infinities with different signs is invalid
      if (summ==-64 && diffsign) return decInvalid(result, set);
      if (DFISINF(dfl)) return decInfinity(result, dfl);    // LHS is infinite
      return decInfinity(result, dfr);                      // RHS must be Inf
      }
    // Here when both arguments are finite; fast path is possible
    // (currently only for aligned and same-sign)
    if (bexpr==bexpl && !diffsign) {
      uInt tac[DECLETS+1];              // base-1000 coefficient
      uInt encode;                      // work

      // Get one coefficient as base-1000 and add the other
      GETCOEFFTHOU(dfl, tac);           // least-significant goes to [0]
      ADDCOEFFTHOU(dfr, tac);
      // here the sum of the MSDs (plus any carry) will be <10 due to
      // the fastpath test earlier

      // construct the result; low word is the same for both formats
      encode =BIN2DPD[tac[0]];
      encode|=BIN2DPD[tac[1]]<<10;
      encode|=BIN2DPD[tac[2]]<<20;
      encode|=BIN2DPD[tac[3]]<<30;
      DFWORD(result, (DECBYTES/4)-1)=encode;

      // collect next two declets (all that remains, for Double)
      encode =BIN2DPD[tac[3]]>>2;
      encode|=BIN2DPD[tac[4]]<<8;

      #if QUAD
      // complete and lay out middling words
      encode|=BIN2DPD[tac[5]]<<18;
      encode|=BIN2DPD[tac[6]]<<28;
      DFWORD(result, 2)=encode;

      encode =BIN2DPD[tac[6]]>>4;
      encode|=BIN2DPD[tac[7]]<<6;
      encode|=BIN2DPD[tac[8]]<<16;
      encode|=BIN2DPD[tac[9]]<<26;
      DFWORD(result, 1)=encode;

      // and final two declets
      encode =BIN2DPD[tac[9]]>>6;
      encode|=BIN2DPD[tac[10]]<<4;
      #endif

      // add exponent continuation and sign (from either argument)
      encode|=sourhil & (ECONMASK | DECFLOAT_Sign);

      // create lookup index = MSD + top two bits of biased exponent <<4
      tac[DECLETS]|=(bexpl>>DECECONL)<<4;
      encode|=DECCOMBFROM[tac[DECLETS]]; // add constructed combination field
      DFWORD(result, 0)=encode;          // complete

      // decFloatShow(result, ">");
      return result;
      } // fast path OK
    // drop through to slow path
    } // low sum or Special(s)

  /* Slow path required -- arguments are finite and might overflow,   */
  /* or require alignment, or might have different signs              */

  // now swap either exponents or argument pointers
  if (bexpl<=bexpr) {
    // original left is bigger
    Int bexpswap=bexpl;
    bexpl=bexpr;
    bexpr=bexpswap;
    // printf("left bigger\n");
    }
   else {
    const decFloat *dfswap=dfl;
    dfl=dfr;
    dfr=dfswap;
    // printf("right bigger\n");
    }
  // [here dfl and bexpl refer to the datum with the larger exponent,
  // of if the exponents are equal then the original LHS argument]

  // if lhs is zero then result will be the rhs (now known to have
  // the smaller exponent), which also may need to be tested for zero
  // for the weird IEEE 754 sign rules
  if (DFISZERO(dfl)) {
    decCanonical(result, dfr);               // clean copy
    // "When the sum of two operands with opposite signs is
    // exactly zero, the sign of that sum shall be '+' in all
    // rounding modes except round toward -Infinity, in which
    // mode that sign shall be '-'."
    if (diffsign && DFISZERO(result)) {
      DFWORD(result, 0)&=~DECFLOAT_Sign;     // assume sign 0
      if (set->round==DEC_ROUND_FLOOR) DFWORD(result, 0)|=DECFLOAT_Sign;
      }
    return result;
    } // numfl is zero
  // [here, LHS is non-zero; code below assumes that]

  // Coefficients layout during the calculations to follow:
  //
  //       Overlap case:
  //       +------------------------------------------------+
  // acc:  |0000|      coeffa      | tail B |               |
  //       +------------------------------------------------+
  // buf:  |0000| pad0s |      coeffb       |               |
  //       +------------------------------------------------+
  //
  //       Touching coefficients or gap:
  //       +------------------------------------------------+
  // acc:  |0000|      coeffa      | gap |      coeffb      |
  //       +------------------------------------------------+
  //       [buf not used or needed; gap clamped to Pmax]

  // lay out lhs coefficient into accumulator; this starts at acc+4
  // for decDouble or acc+6 for decQuad so the LSD is word-
  // aligned; the top word gap is there only in case a carry digit
  // is prefixed after the add -- it does not need to be zeroed
  #if DOUBLE
    #define COFF 4                      // offset into acc
  #elif QUAD
    UBFROMUS(acc+4, 0);                 // prefix 00
    #define COFF 6                      // offset into acc
  #endif

  GETCOEFF(dfl, acc+COFF);              // decode from decFloat
  ulsd=acc+COFF+DECPMAX-1;
  umsd=acc+4;                           // [having this here avoids

  #if DECTRACE
  {bcdnum tum;
  tum.msd=umsd;
  tum.lsd=ulsd;
  tum.exponent=bexpl-DECBIAS;
  tum.sign=DFWORD(dfl, 0) & DECFLOAT_Sign;
  decShowNum(&tum, "dflx");}
  #endif

  // if signs differ, take ten's complement of lhs (here the
  // coefficient is subtracted from all-nines; the 1 is added during
  // the later add cycle -- zeros to the right do not matter because
  // the complement of zero is zero); these are fixed-length inverts
  // where the lsd is known to be at a 4-byte boundary (so no borrow
  // possible)
  carry=0;                              // assume no carry
  if (diffsign) {
    carry=CARRYPAT;                     // for +1 during add
    UBFROMUI(acc+ 4, 0x09090909-UBTOUI(acc+ 4));
    UBFROMUI(acc+ 8, 0x09090909-UBTOUI(acc+ 8));
    UBFROMUI(acc+12, 0x09090909-UBTOUI(acc+12));
    UBFROMUI(acc+16, 0x09090909-UBTOUI(acc+16));
    #if QUAD
    UBFROMUI(acc+20, 0x09090909-UBTOUI(acc+20));
    UBFROMUI(acc+24, 0x09090909-UBTOUI(acc+24));
    UBFROMUI(acc+28, 0x09090909-UBTOUI(acc+28));
    UBFROMUI(acc+32, 0x09090909-UBTOUI(acc+32));
    UBFROMUI(acc+36, 0x09090909-UBTOUI(acc+36));
    #endif
    } // diffsign

  // now process the rhs coefficient; if it cannot overlap lhs then
  // it can be put straight into acc (with an appropriate gap, if
  // needed) because no actual addition will be needed (except
  // possibly to complete ten's complement)
  overlap=DECPMAX-(bexpl-bexpr);
  #if DECTRACE
  printf("exps: %ld %ld\n", (LI)(bexpl-DECBIAS), (LI)(bexpr-DECBIAS));
  printf("Overlap=%ld carry=%08lx\n", (LI)overlap, (LI)carry);
  #endif

  if (overlap<=0) {                     // no overlap possible
    uInt gap;                           // local work
    // since a full addition is not needed, a ten's complement
    // calculation started above may need to be completed
    if (carry) {
      for (ub=ulsd; *ub==9; ub--) *ub=0;
      *ub+=1;
      carry=0;                          // taken care of
      }
    // up to DECPMAX-1 digits of the final result can extend down
    // below the LSD of the lhs, so if the gap is >DECPMAX then the
    // rhs will be simply sticky bits.  In this case the gap is
    // clamped to DECPMAX and the exponent adjusted to suit [this is
    // safe because the lhs is non-zero].
    gap=-overlap;
    if (gap>DECPMAX) {
      bexpr+=gap-1;
      gap=DECPMAX;
      }
    ub=ulsd+gap+1;                      // where MSD will go
    // Fill the gap with 0s; note that there is no addition to do
    ut=acc+COFF+DECPMAX;                // start of gap
    for (; ut<ub; ut+=4) UBFROMUI(ut, 0); // mind the gap
    if (overlap<-DECPMAX) {             // gap was > DECPMAX
      *ub=(uByte)(!DFISZERO(dfr));      // make sticky digit
      }
     else {                             // need full coefficient
      GETCOEFF(dfr, ub);                // decode from decFloat
      ub+=DECPMAX-1;                    // new LSD...
      }
    ulsd=ub;                            // save new LSD
    } // no overlap possible

   else {                               // overlap>0
    // coefficients overlap (perhaps completely, although also
    // perhaps only where zeros)
    if (overlap==DECPMAX) {             // aligned
      ub=buf+COFF;                      // where msd will go
      #if QUAD
      UBFROMUS(buf+4, 0);               // clear quad's 00
      #endif
      GETCOEFF(dfr, ub);                // decode from decFloat
      }
     else {                             // unaligned
      ub=buf+COFF+DECPMAX-overlap;      // where MSD will go
      // Fill the prefix gap with 0s; 8 will cover most common
      // unalignments, so start with direct assignments (a loop is
      // then used for any remaining -- the loop (and the one in a
      // moment) is not then on the critical path because the number
      // of additions is reduced by (at least) two in this case)
      UBFROMUI(buf+4, 0);               // [clears decQuad 00 too]
      UBFROMUI(buf+8, 0);
      if (ub>buf+12) {
        ut=buf+12;                      // start any remaining
        for (; ut<ub; ut+=4) UBFROMUI(ut, 0); // fill them
        }
      GETCOEFF(dfr, ub);                // decode from decFloat

      // now move tail of rhs across to main acc; again use direct
      // copies for 8 digits-worth
      UBFROMUI(acc+COFF+DECPMAX,   UBTOUI(buf+COFF+DECPMAX));
      UBFROMUI(acc+COFF+DECPMAX+4, UBTOUI(buf+COFF+DECPMAX+4));
      if (buf+COFF+DECPMAX+8<ub+DECPMAX) {
        us=buf+COFF+DECPMAX+8;          // source
        ut=acc+COFF+DECPMAX+8;          // target
        for (; us<ub+DECPMAX; us+=4, ut+=4) UBFROMUI(ut, UBTOUI(us));
        }
      } // unaligned

    ulsd=acc+(ub-buf+DECPMAX-1);        // update LSD pointer

    // Now do the add of the non-tail; this is all nicely aligned,
    // and is over a multiple of four digits (because for Quad two
    // zero digits were added on the left); words in both acc and
    // buf (buf especially) will often be zero
    // [byte-by-byte add, here, is about 15% slower total effect than
    // the by-fours]

    // Now effect the add; this is harder on a little-endian
    // machine as the inter-digit carry cannot use the usual BCD
    // addition trick because the bytes are loaded in the wrong order
    // [this loop could be unrolled, but probably scarcely worth it]

    ut=acc+COFF+DECPMAX-4;              // target LSW (acc)
    us=buf+COFF+DECPMAX-4;              // source LSW (buf, to add to acc)

    #if !DECLITEND
    for (; ut>=acc+4; ut-=4, us-=4) {   // big-endian add loop
      // bcd8 add
      carry+=UBTOUI(us);                // rhs + carry
      if (carry==0) continue;           // no-op
      carry+=UBTOUI(ut);                // lhs
      // Big-endian BCD adjust (uses internal carry)
      carry+=0x76f6f6f6;                // note top nibble not all bits
      // apply BCD adjust and save
      UBFROMUI(ut, (carry & 0x0f0f0f0f) - ((carry & 0x60606060)>>4));
      carry>>=31;                       // true carry was at far left
      } // add loop
    #else
    for (; ut>=acc+4; ut-=4, us-=4) {   // little-endian add loop
      // bcd8 add
      carry+=UBTOUI(us);                // rhs + carry
      if (carry==0) continue;           // no-op [common if unaligned]
      carry+=UBTOUI(ut);                // lhs
      // Little-endian BCD adjust; inter-digit carry must be manual
      // because the lsb from the array will be in the most-significant
      // byte of carry
      carry+=0x76767676;                // note no inter-byte carries
      carry+=(carry & 0x80000000)>>15;
      carry+=(carry & 0x00800000)>>15;
      carry+=(carry & 0x00008000)>>15;
      carry-=(carry & 0x60606060)>>4;   // BCD adjust back
      UBFROMUI(ut, carry & 0x0f0f0f0f); // clear debris and save
      // here, final carry-out bit is at 0x00000080; move it ready
      // for next word-add (i.e., to 0x01000000)
      carry=(carry & 0x00000080)<<17;
      } // add loop
    #endif

    #if DECTRACE
    {bcdnum tum;
    printf("Add done, carry=%08lx, diffsign=%ld\n", (LI)carry, (LI)diffsign);
    tum.msd=umsd;  // acc+4;
    tum.lsd=ulsd;
    tum.exponent=0;
    tum.sign=0;
    decShowNum(&tum, "dfadd");}
    #endif
    } // overlap possible

  // ordering here is a little strange in order to have slowest path
  // first in GCC asm listing
  if (diffsign) {                  // subtraction
    if (!carry) {                  // no carry out means RHS<LHS
      // borrowed -- take ten's complement
      // sign is lhs sign
      num.sign=DFWORD(dfl, 0) & DECFLOAT_Sign;

      // invert the coefficient first by fours, then add one; space
      // at the end of the buffer ensures the by-fours is always
      // safe, but lsd+1 must be cleared to prevent a borrow
      // if big-endian
      #if !DECLITEND
      *(ulsd+1)=0;
      #endif
      // there are always at least four coefficient words
      UBFROMUI(umsd,    0x09090909-UBTOUI(umsd));
      UBFROMUI(umsd+4,  0x09090909-UBTOUI(umsd+4));
      UBFROMUI(umsd+8,  0x09090909-UBTOUI(umsd+8));
      UBFROMUI(umsd+12, 0x09090909-UBTOUI(umsd+12));
      #if DOUBLE
        #define BNEXT 16
      #elif QUAD
        UBFROMUI(umsd+16, 0x09090909-UBTOUI(umsd+16));
        UBFROMUI(umsd+20, 0x09090909-UBTOUI(umsd+20));
        UBFROMUI(umsd+24, 0x09090909-UBTOUI(umsd+24));
        UBFROMUI(umsd+28, 0x09090909-UBTOUI(umsd+28));
        UBFROMUI(umsd+32, 0x09090909-UBTOUI(umsd+32));
        #define BNEXT 36
      #endif
      if (ulsd>=umsd+BNEXT) {           // unaligned
        // eight will handle most unaligments for Double; 16 for Quad
        UBFROMUI(umsd+BNEXT,   0x09090909-UBTOUI(umsd+BNEXT));
        UBFROMUI(umsd+BNEXT+4, 0x09090909-UBTOUI(umsd+BNEXT+4));
        #if DOUBLE
        #define BNEXTY (BNEXT+8)
        #elif QUAD
        UBFROMUI(umsd+BNEXT+8,  0x09090909-UBTOUI(umsd+BNEXT+8));
        UBFROMUI(umsd+BNEXT+12, 0x09090909-UBTOUI(umsd+BNEXT+12));
        #define BNEXTY (BNEXT+16)
        #endif
        if (ulsd>=umsd+BNEXTY) {        // very unaligned
          ut=umsd+BNEXTY;               // -> continue
          for (;;ut+=4) {
            UBFROMUI(ut, 0x09090909-UBTOUI(ut)); // invert four digits
            if (ut>=ulsd-3) break;      // all done
            }
          }
        }
      // complete the ten's complement by adding 1
      for (ub=ulsd; *ub==9; ub--) *ub=0;
      *ub+=1;
      } // borrowed

     else {                        // carry out means RHS>=LHS
      num.sign=DFWORD(dfr, 0) & DECFLOAT_Sign;
      // all done except for the special IEEE 754 exact-zero-result
      // rule (see above); while testing for zero, strip leading
      // zeros (which will save decFinalize doing it) (this is in
      // diffsign path, so carry impossible and true umsd is
      // acc+COFF)

      // Check the initial coefficient area using the fast macro;
      // this will often be all that needs to be done (as on the
      // worst-case path when the subtraction was aligned and
      // full-length)
      if (ISCOEFFZERO(acc+COFF)) {
        umsd=acc+COFF+DECPMAX-1;   // so far, so zero
        if (ulsd>umsd) {           // more to check
          umsd++;                  // to align after checked area
          for (; UBTOUI(umsd)==0 && umsd+3<ulsd;) umsd+=4;
          for (; *umsd==0 && umsd<ulsd;) umsd++;
          }
        if (*umsd==0) {            // must be true zero (and diffsign)
          num.sign=0;              // assume +
          if (set->round==DEC_ROUND_FLOOR) num.sign=DECFLOAT_Sign;
          }
        }
      // [else was not zero, might still have leading zeros]
      } // subtraction gave positive result
    } // diffsign

   else { // same-sign addition
    num.sign=DFWORD(dfl, 0)&DECFLOAT_Sign;
    #if DOUBLE
    if (carry) {                   // only possible with decDouble
      *(acc+3)=1;                  // [Quad has leading 00]
      umsd=acc+3;
      }
    #endif
    } // same sign

  num.msd=umsd;                    // set MSD ..
  num.lsd=ulsd;                    // .. and LSD
  num.exponent=bexpr-DECBIAS;      // set exponent to smaller, unbiassed

  #if DECTRACE
  decFloatShow(dfl, "dfl");
  decFloatShow(dfr, "dfr");
  decShowNum(&num, "postadd");
  #endif
  return decFinalize(result, &num, set); // round, check, and lay out
  } // decFloatAdd

/* ------------------------------------------------------------------ */
/* decFloatAnd -- logical digitwise AND of two decFloats              */
/*                                                                    */
/*   result gets the result of ANDing dfl and dfr                     */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result, which will be canonical with sign=0              */
/*                                                                    */
/* The operands must be positive, finite with exponent q=0, and       */
/* comprise just zeros and ones; if not, Invalid operation results.   */
/* ------------------------------------------------------------------ */
decFloat * decFloatAnd(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  if (!DFISUINT01(dfl) || !DFISUINT01(dfr)
   || !DFISCC01(dfl)   || !DFISCC01(dfr)) return decInvalid(result, set);
  // the operands are positive finite integers (q=0) with just 0s and 1s
  #if DOUBLE
   DFWORD(result, 0)=ZEROWORD
                   |((DFWORD(dfl, 0) & DFWORD(dfr, 0))&0x04009124);
   DFWORD(result, 1)=(DFWORD(dfl, 1) & DFWORD(dfr, 1))&0x49124491;
  #elif QUAD
   DFWORD(result, 0)=ZEROWORD
                   |((DFWORD(dfl, 0) & DFWORD(dfr, 0))&0x04000912);
   DFWORD(result, 1)=(DFWORD(dfl, 1) & DFWORD(dfr, 1))&0x44912449;
   DFWORD(result, 2)=(DFWORD(dfl, 2) & DFWORD(dfr, 2))&0x12449124;
   DFWORD(result, 3)=(DFWORD(dfl, 3) & DFWORD(dfr, 3))&0x49124491;
  #endif
  return result;
  } // decFloatAnd

/* ------------------------------------------------------------------ */
/* decFloatCanonical -- copy a decFloat, making canonical             */
/*                                                                    */
/*   result gets the canonicalized df                                 */
/*   df     is the decFloat to copy and make canonical                */
/*   returns result                                                   */
/*                                                                    */
/* This works on specials, too; no error or exception is possible.    */
/* ------------------------------------------------------------------ */
decFloat * decFloatCanonical(decFloat *result, const decFloat *df) {
  return decCanonical(result, df);
  } // decFloatCanonical

/* ------------------------------------------------------------------ */
/* decFloatClass -- return the class of a decFloat                    */
/*                                                                    */
/*   df is the decFloat to test                                       */
/*   returns the decClass that df falls into                          */
/* ------------------------------------------------------------------ */
enum decClass decFloatClass(const decFloat *df) {
  Int exp;                         // exponent
  if (DFISSPECIAL(df)) {
    if (DFISQNAN(df)) return DEC_CLASS_QNAN;
    if (DFISSNAN(df)) return DEC_CLASS_SNAN;
    // must be an infinity
    if (DFISSIGNED(df)) return DEC_CLASS_NEG_INF;
    return DEC_CLASS_POS_INF;
    }
  if (DFISZERO(df)) {              // quite common
    if (DFISSIGNED(df)) return DEC_CLASS_NEG_ZERO;
    return DEC_CLASS_POS_ZERO;
    }
  // is finite and non-zero; similar code to decFloatIsNormal, here
  // [this could be speeded up slightly by in-lining decFloatDigits]
  exp=GETEXPUN(df)                 // get unbiased exponent ..
     +decFloatDigits(df)-1;        // .. and make adjusted exponent
  if (exp>=DECEMIN) {              // is normal
    if (DFISSIGNED(df)) return DEC_CLASS_NEG_NORMAL;
    return DEC_CLASS_POS_NORMAL;
    }
  // is subnormal
  if (DFISSIGNED(df)) return DEC_CLASS_NEG_SUBNORMAL;
  return DEC_CLASS_POS_SUBNORMAL;
  } // decFloatClass

/* ------------------------------------------------------------------ */
/* decFloatClassString -- return the class of a decFloat as a string  */
/*                                                                    */
/*   df is the decFloat to test                                       */
/*   returns a constant string describing the class df falls into     */
/* ------------------------------------------------------------------ */
const char *decFloatClassString(const decFloat *df) {
  enum decClass eclass=decFloatClass(df);
  if (eclass==DEC_CLASS_POS_NORMAL)    return DEC_ClassString_PN;
  if (eclass==DEC_CLASS_NEG_NORMAL)    return DEC_ClassString_NN;
  if (eclass==DEC_CLASS_POS_ZERO)      return DEC_ClassString_PZ;
  if (eclass==DEC_CLASS_NEG_ZERO)      return DEC_ClassString_NZ;
  if (eclass==DEC_CLASS_POS_SUBNORMAL) return DEC_ClassString_PS;
  if (eclass==DEC_CLASS_NEG_SUBNORMAL) return DEC_ClassString_NS;
  if (eclass==DEC_CLASS_POS_INF)       return DEC_ClassString_PI;
  if (eclass==DEC_CLASS_NEG_INF)       return DEC_ClassString_NI;
  if (eclass==DEC_CLASS_QNAN)          return DEC_ClassString_QN;
  if (eclass==DEC_CLASS_SNAN)          return DEC_ClassString_SN;
  return DEC_ClassString_UN;           // Unknown
  } // decFloatClassString

/* ------------------------------------------------------------------ */
/* decFloatCompare -- compare two decFloats; quiet NaNs allowed       */
/*                                                                    */
/*   result gets the result of comparing dfl and dfr                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result, which may be -1, 0, 1, or NaN (Unordered)        */
/* ------------------------------------------------------------------ */
decFloat * decFloatCompare(decFloat *result,
                           const decFloat *dfl, const decFloat *dfr,
                           decContext *set) {
  Int comp;                                  // work
  // NaNs are handled as usual
  if (DFISNAN(dfl) || DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
  // numeric comparison needed
  comp=decNumCompare(dfl, dfr, 0);
  decFloatZero(result);
  if (comp==0) return result;
  DFBYTE(result, DECBYTES-1)=0x01;      // LSD=1
  if (comp<0) DFBYTE(result, 0)|=0x80;  // set sign bit
  return result;
  } // decFloatCompare

/* ------------------------------------------------------------------ */
/* decFloatCompareSignal -- compare two decFloats; all NaNs signal    */
/*                                                                    */
/*   result gets the result of comparing dfl and dfr                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result, which may be -1, 0, 1, or NaN (Unordered)        */
/* ------------------------------------------------------------------ */
decFloat * decFloatCompareSignal(decFloat *result,
                                 const decFloat *dfl, const decFloat *dfr,
                                 decContext *set) {
  Int comp;                                  // work
  // NaNs are handled as usual, except that all NaNs signal
  if (DFISNAN(dfl) || DFISNAN(dfr)) {
    set->status|=DEC_Invalid_operation;
    return decNaNs(result, dfl, dfr, set);
    }
  // numeric comparison needed
  comp=decNumCompare(dfl, dfr, 0);
  decFloatZero(result);
  if (comp==0) return result;
  DFBYTE(result, DECBYTES-1)=0x01;      // LSD=1
  if (comp<0) DFBYTE(result, 0)|=0x80;  // set sign bit
  return result;
  } // decFloatCompareSignal

/* ------------------------------------------------------------------ */
/* decFloatCompareTotal -- compare two decFloats with total ordering  */
/*                                                                    */
/*   result gets the result of comparing dfl and dfr                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   returns result, which may be -1, 0, or 1                         */
/* ------------------------------------------------------------------ */
decFloat * decFloatCompareTotal(decFloat *result,
                                const decFloat *dfl, const decFloat *dfr) {
  Int  comp;                                 // work
  uInt uiwork;                               // for macros
  #if QUAD
  uShort uswork;                             // ..
  #endif
  if (DFISNAN(dfl) || DFISNAN(dfr)) {
    Int nanl, nanr;                          // work
    // morph NaNs to +/- 1 or 2, leave numbers as 0
    nanl=DFISSNAN(dfl)+DFISQNAN(dfl)*2;      // quiet > signalling
    if (DFISSIGNED(dfl)) nanl=-nanl;
    nanr=DFISSNAN(dfr)+DFISQNAN(dfr)*2;
    if (DFISSIGNED(dfr)) nanr=-nanr;
    if (nanl>nanr) comp=+1;
     else if (nanl<nanr) comp=-1;
     else { // NaNs are the same type and sign .. must compare payload
      // buffers need +2 for QUAD
      uByte bufl[DECPMAX+4];                 // for LHS coefficient + foot
      uByte bufr[DECPMAX+4];                 // for RHS coefficient + foot
      uByte *ub, *uc;                        // work
      Int sigl;                              // signum of LHS
      sigl=(DFISSIGNED(dfl) ? -1 : +1);

      // decode the coefficients
      // (shift both right two if Quad to make a multiple of four)
      #if QUAD
        UBFROMUS(bufl, 0);
        UBFROMUS(bufr, 0);
      #endif
      GETCOEFF(dfl, bufl+QUAD*2);            // decode from decFloat
      GETCOEFF(dfr, bufr+QUAD*2);            // ..
      // all multiples of four, here
      comp=0;                                // assume equal
      for (ub=bufl, uc=bufr; ub<bufl+DECPMAX+QUAD*2; ub+=4, uc+=4) {
        uInt ui=UBTOUI(ub);
        if (ui==UBTOUI(uc)) continue; // so far so same
        // about to find a winner; go by bytes in case little-endian
        for (;; ub++, uc++) {
          if (*ub==*uc) continue;
          if (*ub>*uc) comp=sigl;            // difference found
           else comp=-sigl;                  // ..
           break;
          }
        }
      } // same NaN type and sign
    }
   else {
    // numeric comparison needed
    comp=decNumCompare(dfl, dfr, 1);    // total ordering
    }
  decFloatZero(result);
  if (comp==0) return result;
  DFBYTE(result, DECBYTES-1)=0x01;      // LSD=1
  if (comp<0) DFBYTE(result, 0)|=0x80;  // set sign bit
  return result;
  } // decFloatCompareTotal

/* ------------------------------------------------------------------ */
/* decFloatCompareTotalMag -- compare magnitudes with total ordering  */
/*                                                                    */
/*   result gets the result of comparing abs(dfl) and abs(dfr)        */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   returns result, which may be -1, 0, or 1                         */
/* ------------------------------------------------------------------ */
decFloat * decFloatCompareTotalMag(decFloat *result,
                                const decFloat *dfl, const decFloat *dfr) {
  decFloat a, b;                        // for copy if needed
  // copy and redirect signed operand(s)
  if (DFISSIGNED(dfl)) {
    decFloatCopyAbs(&a, dfl);
    dfl=&a;
    }
  if (DFISSIGNED(dfr)) {
    decFloatCopyAbs(&b, dfr);
    dfr=&b;
    }
  return decFloatCompareTotal(result, dfl, dfr);
  } // decFloatCompareTotalMag

/* ------------------------------------------------------------------ */
/* decFloatCopy -- copy a decFloat as-is                              */
/*                                                                    */
/*   result gets the copy of dfl                                      */
/*   dfl    is the decFloat to copy                                   */
/*   returns result                                                   */
/*                                                                    */
/* This is a bitwise operation; no errors or exceptions are possible. */
/* ------------------------------------------------------------------ */
decFloat * decFloatCopy(decFloat *result, const decFloat *dfl) {
  if (dfl!=result) *result=*dfl;             // copy needed
  return result;
  } // decFloatCopy

/* ------------------------------------------------------------------ */
/* decFloatCopyAbs -- copy a decFloat as-is and set sign bit to 0     */
/*                                                                    */
/*   result gets the copy of dfl with sign bit 0                      */
/*   dfl    is the decFloat to copy                                   */
/*   returns result                                                   */
/*                                                                    */
/* This is a bitwise operation; no errors or exceptions are possible. */
/* ------------------------------------------------------------------ */
decFloat * decFloatCopyAbs(decFloat *result, const decFloat *dfl) {
  if (dfl!=result) *result=*dfl;        // copy needed
  DFBYTE(result, 0)&=~0x80;             // zero sign bit
  return result;
  } // decFloatCopyAbs

/* ------------------------------------------------------------------ */
/* decFloatCopyNegate -- copy a decFloat as-is with inverted sign bit */
/*                                                                    */
/*   result gets the copy of dfl with sign bit inverted               */
/*   dfl    is the decFloat to copy                                   */
/*   returns result                                                   */
/*                                                                    */
/* This is a bitwise operation; no errors or exceptions are possible. */
/* ------------------------------------------------------------------ */
decFloat * decFloatCopyNegate(decFloat *result, const decFloat *dfl) {
  if (dfl!=result) *result=*dfl;        // copy needed
  DFBYTE(result, 0)^=0x80;              // invert sign bit
  return result;
  } // decFloatCopyNegate

/* ------------------------------------------------------------------ */
/* decFloatCopySign -- copy a decFloat with the sign of another       */
/*                                                                    */
/*   result gets the result of copying dfl with the sign of dfr       */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   returns result                                                   */
/*                                                                    */
/* This is a bitwise operation; no errors or exceptions are possible. */
/* ------------------------------------------------------------------ */
decFloat * decFloatCopySign(decFloat *result,
                            const decFloat *dfl, const decFloat *dfr) {
  uByte sign=(uByte)(DFBYTE(dfr, 0)&0x80);   // save sign bit
  if (dfl!=result) *result=*dfl;             // copy needed
  DFBYTE(result, 0)&=~0x80;                  // clear sign ..
  DFBYTE(result, 0)=(uByte)(DFBYTE(result, 0)|sign); // .. and set saved
  return result;
  } // decFloatCopySign

/* ------------------------------------------------------------------ */
/* decFloatDigits -- return the number of digits in a decFloat        */
/*                                                                    */
/*   df is the decFloat to investigate                                */
/*   returns the number of significant digits in the decFloat; a      */
/*     zero coefficient returns 1 as does an infinity (a NaN returns  */
/*     the number of digits in the payload)                           */
/* ------------------------------------------------------------------ */
// private macro to extract a declet according to provided formula
// (form), and if it is non-zero then return the calculated digits
// depending on the declet number (n), where n=0 for the most
// significant declet; uses uInt dpd for work
#define dpdlenchk(n, form)  dpd=(form)&0x3ff;     \
  if (dpd) return (DECPMAX-1-3*(n))-(3-DPD2BCD8[dpd*4+3])
// next one is used when it is known that the declet must be
// non-zero, or is the final zero declet
#define dpdlendun(n, form)  dpd=(form)&0x3ff;     \
  if (dpd==0) return 1;                           \
  return (DECPMAX-1-3*(n))-(3-DPD2BCD8[dpd*4+3])

uInt decFloatDigits(const decFloat *df) {
  uInt dpd;                        // work
  uInt sourhi=DFWORD(df, 0);       // top word from source decFloat
  #if QUAD
  uInt sourmh, sourml;
  #endif
  uInt sourlo;

  if (DFISINF(df)) return 1;
  // A NaN effectively has an MSD of 0; otherwise if non-zero MSD
  // then the coefficient is full-length
  if (!DFISNAN(df) && DECCOMBMSD[sourhi>>26]) return DECPMAX;

  #if DOUBLE
    if (sourhi&0x0003ffff) {     // ends in first
      dpdlenchk(0, sourhi>>8);
      sourlo=DFWORD(df, 1);
      dpdlendun(1, (sourhi<<2) | (sourlo>>30));
      } // [cannot drop through]
    sourlo=DFWORD(df, 1);  // sourhi not involved now
    if (sourlo&0xfff00000) {     // in one of first two
      dpdlenchk(1, sourlo>>30);  // very rare
      dpdlendun(2, sourlo>>20);
      } // [cannot drop through]
    dpdlenchk(3, sourlo>>10);
    dpdlendun(4, sourlo);
    // [cannot drop through]

  #elif QUAD
    if (sourhi&0x00003fff) {     // ends in first
      dpdlenchk(0, sourhi>>4);
      sourmh=DFWORD(df, 1);
      dpdlendun(1, ((sourhi)<<6) | (sourmh>>26));
      } // [cannot drop through]
    sourmh=DFWORD(df, 1);
    if (sourmh) {
      dpdlenchk(1, sourmh>>26);
      dpdlenchk(2, sourmh>>16);
      dpdlenchk(3, sourmh>>6);
      sourml=DFWORD(df, 2);
      dpdlendun(4, ((sourmh)<<4) | (sourml>>28));
      } // [cannot drop through]
    sourml=DFWORD(df, 2);
    if (sourml) {
      dpdlenchk(4, sourml>>28);
      dpdlenchk(5, sourml>>18);
      dpdlenchk(6, sourml>>8);
      sourlo=DFWORD(df, 3);
      dpdlendun(7, ((sourml)<<2) | (sourlo>>30));
      } // [cannot drop through]
    sourlo=DFWORD(df, 3);
    if (sourlo&0xfff00000) {     // in one of first two
      dpdlenchk(7, sourlo>>30);  // very rare
      dpdlendun(8, sourlo>>20);
      } // [cannot drop through]
    dpdlenchk(9, sourlo>>10);
    dpdlendun(10, sourlo);
    // [cannot drop through]
  #endif
  } // decFloatDigits

/* ------------------------------------------------------------------ */
/* decFloatDivide -- divide a decFloat by another                     */
/*                                                                    */
/*   result gets the result of dividing dfl by dfr:                   */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
// This is just a wrapper.
decFloat * decFloatDivide(decFloat *result,
                          const decFloat *dfl, const decFloat *dfr,
                          decContext *set) {
  return decDivide(result, dfl, dfr, set, DIVIDE);
  } // decFloatDivide

/* ------------------------------------------------------------------ */
/* decFloatDivideInteger -- integer divide a decFloat by another      */
/*                                                                    */
/*   result gets the result of dividing dfl by dfr:                   */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatDivideInteger(decFloat *result,
                             const decFloat *dfl, const decFloat *dfr,
                             decContext *set) {
  return decDivide(result, dfl, dfr, set, DIVIDEINT);
  } // decFloatDivideInteger

/* ------------------------------------------------------------------ */
/* decFloatFMA -- multiply and add three decFloats, fused             */
/*                                                                    */
/*   result gets the result of (dfl*dfr)+dff with a single rounding   */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   dff    is the final decFloat (fhs)                               */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatFMA(decFloat *result, const decFloat *dfl,
                       const decFloat *dfr, const decFloat *dff,
                       decContext *set) {

  // The accumulator has the bytes needed for FiniteMultiply, plus
  // one byte to the left in case of carry, plus DECPMAX+2 to the
  // right for the final addition (up to full fhs + round & sticky)
  #define FMALEN (ROUNDUP4(1+ (DECPMAX9*18+1) +DECPMAX+2))
  uByte  acc[FMALEN];              // for multiplied coefficient in BCD
                                   // .. and for final result
  bcdnum mul;                      // for multiplication result
  bcdnum fin;                      // for final operand, expanded
  uByte  coe[ROUNDUP4(DECPMAX)];   // dff coefficient in BCD
  bcdnum *hi, *lo;                 // bcdnum with higher/lower exponent
  uInt   diffsign;                 // non-zero if signs differ
  uInt   hipad;                    // pad digit for hi if needed
  Int    padding;                  // excess exponent
  uInt   carry;                    // +1 for ten's complement and during add
  uByte  *ub, *uh, *ul;            // work
  uInt   uiwork;                   // for macros

  // handle all the special values [any special operand leads to a
  // special result]
  if (DFISSPECIAL(dfl) || DFISSPECIAL(dfr) || DFISSPECIAL(dff)) {
    decFloat proxy;                // multiplication result proxy
    // NaNs are handled as usual, giving priority to sNaNs
    if (DFISSNAN(dfl) || DFISSNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    if (DFISSNAN(dff)) return decNaNs(result, dff, NULL, set);
    if (DFISNAN(dfl) || DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    if (DFISNAN(dff)) return decNaNs(result, dff, NULL, set);
    // One or more of the three is infinite
    // infinity times zero is bad
    decFloatZero(&proxy);
    if (DFISINF(dfl)) {
      if (DFISZERO(dfr)) return decInvalid(result, set);
      decInfinity(&proxy, &proxy);
      }
     else if (DFISINF(dfr)) {
      if (DFISZERO(dfl)) return decInvalid(result, set);
      decInfinity(&proxy, &proxy);
      }
    // compute sign of multiplication and place in proxy
    DFWORD(&proxy, 0)|=(DFWORD(dfl, 0)^DFWORD(dfr, 0))&DECFLOAT_Sign;
    if (!DFISINF(dff)) return decFloatCopy(result, &proxy);
    // dff is Infinite
    if (!DFISINF(&proxy)) return decInfinity(result, dff);
    // both sides of addition are infinite; different sign is bad
    if ((DFWORD(dff, 0)&DECFLOAT_Sign)!=(DFWORD(&proxy, 0)&DECFLOAT_Sign))
      return decInvalid(result, set);
    return decFloatCopy(result, &proxy);
    }

  /* Here when all operands are finite */

  // First multiply dfl*dfr
  decFiniteMultiply(&mul, acc+1, dfl, dfr);
  // The multiply is complete, exact and unbounded, and described in
  // mul with the coefficient held in acc[1...]

  // now add in dff; the algorithm is essentially the same as
  // decFloatAdd, but the code is different because the code there
  // is highly optimized for adding two numbers of the same size
  fin.exponent=GETEXPUN(dff);           // get dff exponent and sign
  fin.sign=DFWORD(dff, 0)&DECFLOAT_Sign;
  diffsign=mul.sign^fin.sign;           // note if signs differ
  fin.msd=coe;
  fin.lsd=coe+DECPMAX-1;
  GETCOEFF(dff, coe);                   // extract the coefficient

  // now set hi and lo so that hi points to whichever of mul and fin
  // has the higher exponent and lo points to the other [don't care,
  // if the same].  One coefficient will be in acc, the other in coe.
  if (mul.exponent>=fin.exponent) {
    hi=&mul;
    lo=&fin;
    }
   else {
    hi=&fin;
    lo=&mul;
    }

  // remove leading zeros on both operands; this will save time later
  // and make testing for zero trivial (tests are safe because acc
  // and coe are rounded up to uInts)
  for (; UBTOUI(hi->msd)==0 && hi->msd+3<hi->lsd;) hi->msd+=4;
  for (; *hi->msd==0 && hi->msd<hi->lsd;) hi->msd++;
  for (; UBTOUI(lo->msd)==0 && lo->msd+3<lo->lsd;) lo->msd+=4;
  for (; *lo->msd==0 && lo->msd<lo->lsd;) lo->msd++;

  // if hi is zero then result will be lo (which has the smaller
  // exponent), which also may need to be tested for zero for the
  // weird IEEE 754 sign rules
  if (*hi->msd==0) {                         // hi is zero
    // "When the sum of two operands with opposite signs is
    // exactly zero, the sign of that sum shall be '+' in all
    // rounding modes except round toward -Infinity, in which
    // mode that sign shall be '-'."
    if (diffsign) {
      if (*lo->msd==0) {                     // lo is zero
        lo->sign=0;
        if (set->round==DEC_ROUND_FLOOR) lo->sign=DECFLOAT_Sign;
        } // diffsign && lo=0
      } // diffsign
    return decFinalize(result, lo, set);     // may need clamping
    } // numfl is zero
  // [here, both are minimal length and hi is non-zero]
  // (if lo is zero then padding with zeros may be needed, below)

  // if signs differ, take the ten's complement of hi (zeros to the
  // right do not matter because the complement of zero is zero); the
  // +1 is done later, as part of the addition, inserted at the
  // correct digit
  hipad=0;
  carry=0;
  if (diffsign) {
    hipad=9;
    carry=1;
    // exactly the correct number of digits must be inverted
    for (uh=hi->msd; uh<hi->lsd-3; uh+=4) UBFROMUI(uh, 0x09090909-UBTOUI(uh));
    for (; uh<=hi->lsd; uh++) *uh=(uByte)(0x09-*uh);
    }

  // ready to add; note that hi has no leading zeros so gap
  // calculation does not have to be as pessimistic as in decFloatAdd
  // (this is much more like the arbitrary-precision algorithm in
  // Rexx and decNumber)

  // padding is the number of zeros that would need to be added to hi
  // for its lsd to be aligned with the lsd of lo
  padding=hi->exponent-lo->exponent;
  // printf("FMA pad %ld\n", (LI)padding);

  // the result of the addition will be built into the accumulator,
  // starting from the far right; this could be either hi or lo, and
  // will be aligned
  ub=acc+FMALEN-1;                 // where lsd of result will go
  ul=lo->lsd;                      // lsd of rhs

  if (padding!=0) {                // unaligned
    // if the msd of lo is more than DECPMAX+2 digits to the right of
    // the original msd of hi then it can be reduced to a single
    // digit at the right place, as it stays clear of hi digits
    // [it must be DECPMAX+2 because during a subtraction the msd
    // could become 0 after a borrow from 1.000 to 0.9999...]

    Int hilen=(Int)(hi->lsd-hi->msd+1); // length of hi
    Int lolen=(Int)(lo->lsd-lo->msd+1); // and of lo

    if (hilen+padding-lolen > DECPMAX+2) {   // can reduce lo to single
      // make sure it is virtually at least DECPMAX from hi->msd, at
      // least to right of hi->lsd (in case of destructive subtract),
      // and separated by at least two digits from either of those
      // (the tricky DOUBLE case is when hi is a 1 that will become a
      // 0.9999... by subtraction:
      //   hi:   1                                   E+16
      //   lo:    .................1000000000000000  E-16
      // which for the addition pads to:
      //   hi:   1000000000000000000                 E-16
      //   lo:    .................1000000000000000  E-16
      Int newexp=MINI(hi->exponent, hi->exponent+hilen-DECPMAX)-3;

      // printf("FMA reduce: %ld\n", (LI)reduce);
      lo->lsd=lo->msd;                       // to single digit [maybe 0]
      lo->exponent=newexp;                   // new lowest exponent
      padding=hi->exponent-lo->exponent;     // recalculate
      ul=lo->lsd;                            // .. and repoint
      }

    // padding is still > 0, but will fit in acc (less leading carry slot)
    #if DECCHECK
      if (padding<=0) printf("FMA low padding: %ld\n", (LI)padding);
      if (hilen+padding+1>FMALEN)
        printf("FMA excess hilen+padding: %ld+%ld \n", (LI)hilen, (LI)padding);
      // printf("FMA padding: %ld\n", (LI)padding);
    #endif

    // padding digits can now be set in the result; one or more of
    // these will come from lo; others will be zeros in the gap
    for (; ul-3>=lo->msd && padding>3; padding-=4, ul-=4, ub-=4) {
      UBFROMUI(ub-3, UBTOUI(ul-3));          // [cannot overlap]
      }
    for (; ul>=lo->msd && padding>0; padding--, ul--, ub--) *ub=*ul;
    for (;padding>0; padding--, ub--) *ub=0; // mind the gap
    }

  // addition now complete to the right of the rightmost digit of hi
  uh=hi->lsd;

  // dow do the add from hi->lsd to the left
  // [bytewise, because either operand can run out at any time]
  // carry was set up depending on ten's complement above
  // first assume both operands have some digits
  for (;; ub--) {
    if (uh<hi->msd || ul<lo->msd) break;
    *ub=(uByte)(carry+(*uh--)+(*ul--));
    carry=0;
    if (*ub<10) continue;
    *ub-=10;
    carry=1;
    } // both loop

  if (ul<lo->msd) {           // to left of lo
    for (;; ub--) {
      if (uh<hi->msd) break;
      *ub=(uByte)(carry+(*uh--));  // [+0]
      carry=0;
      if (*ub<10) continue;
      *ub-=10;
      carry=1;
      } // hi loop
    }
   else {                     // to left of hi
    for (;; ub--) {
      if (ul<lo->msd) break;
      *ub=(uByte)(carry+hipad+(*ul--));
      carry=0;
      if (*ub<10) continue;
      *ub-=10;
      carry=1;
      } // lo loop
    }

  // addition complete -- now handle carry, borrow, etc.
  // use lo to set up the num (its exponent is already correct, and
  // sign usually is)
  lo->msd=ub+1;
  lo->lsd=acc+FMALEN-1;
  // decShowNum(lo, "lo");
  if (!diffsign) {                 // same-sign addition
    if (carry) {                   // carry out
      *ub=1;                       // place the 1 ..
      lo->msd--;                   // .. and update
      }
    } // same sign
   else {                          // signs differed (subtraction)
    if (!carry) {                  // no carry out means hi<lo
      // borrowed -- take ten's complement of the right digits
      lo->sign=hi->sign;           // sign is lhs sign
      for (ul=lo->msd; ul<lo->lsd-3; ul+=4) UBFROMUI(ul, 0x09090909-UBTOUI(ul));
      for (; ul<=lo->lsd; ul++) *ul=(uByte)(0x09-*ul); // [leaves ul at lsd+1]
      // complete the ten's complement by adding 1 [cannot overrun]
      for (ul--; *ul==9; ul--) *ul=0;
      *ul+=1;
      } // borrowed
     else {                        // carry out means hi>=lo
      // sign to use is lo->sign
      // all done except for the special IEEE 754 exact-zero-result
      // rule (see above); while testing for zero, strip leading
      // zeros (which will save decFinalize doing it)
      for (; UBTOUI(lo->msd)==0 && lo->msd+3<lo->lsd;) lo->msd+=4;
      for (; *lo->msd==0 && lo->msd<lo->lsd;) lo->msd++;
      if (*lo->msd==0) {           // must be true zero (and diffsign)
        lo->sign=0;                // assume +
        if (set->round==DEC_ROUND_FLOOR) lo->sign=DECFLOAT_Sign;
        }
      // [else was not zero, might still have leading zeros]
      } // subtraction gave positive result
    } // diffsign

  #if DECCHECK
  // assert no left underrun
  if (lo->msd<acc) {
    printf("FMA underrun by %ld \n", (LI)(acc-lo->msd));
    }
  #endif

  return decFinalize(result, lo, set);  // round, check, and lay out
  } // decFloatFMA

/* ------------------------------------------------------------------ */
/* decFloatFromInt -- initialise a decFloat from an Int               */
/*                                                                    */
/*   result gets the converted Int                                    */
/*   n      is the Int to convert                                     */
/*   returns result                                                   */
/*                                                                    */
/* The result is Exact; no errors or exceptions are possible.         */
/* ------------------------------------------------------------------ */
decFloat * decFloatFromInt32(decFloat *result, Int n) {
  uInt u=(uInt)n;                       // copy as bits
  uInt encode;                          // work
  DFWORD(result, 0)=ZEROWORD;           // always
  #if QUAD
    DFWORD(result, 1)=0;
    DFWORD(result, 2)=0;
  #endif
  if (n<0) {                            // handle -n with care
    // [This can be done without the test, but is then slightly slower]
    u=(~u)+1;
    DFWORD(result, 0)|=DECFLOAT_Sign;
    }
  // Since the maximum value of u now is 2**31, only the low word of
  // result is affected
  encode=BIN2DPD[u%1000];
  u/=1000;
  encode|=BIN2DPD[u%1000]<<10;
  u/=1000;
  encode|=BIN2DPD[u%1000]<<20;
  u/=1000;                              // now 0, 1, or 2
  encode|=u<<30;
  DFWORD(result, DECWORDS-1)=encode;
  return result;
  } // decFloatFromInt32

/* ------------------------------------------------------------------ */
/* decFloatFromUInt -- initialise a decFloat from a uInt              */
/*                                                                    */
/*   result gets the converted uInt                                   */
/*   n      is the uInt to convert                                    */
/*   returns result                                                   */
/*                                                                    */
/* The result is Exact; no errors or exceptions are possible.         */
/* ------------------------------------------------------------------ */
decFloat * decFloatFromUInt32(decFloat *result, uInt u) {
  uInt encode;                          // work
  DFWORD(result, 0)=ZEROWORD;           // always
  #if QUAD
    DFWORD(result, 1)=0;
    DFWORD(result, 2)=0;
  #endif
  encode=BIN2DPD[u%1000];
  u/=1000;
  encode|=BIN2DPD[u%1000]<<10;
  u/=1000;
  encode|=BIN2DPD[u%1000]<<20;
  u/=1000;                              // now 0 -> 4
  encode|=u<<30;
  DFWORD(result, DECWORDS-1)=encode;
  DFWORD(result, DECWORDS-2)|=u>>2;     // rarely non-zero
  return result;
  } // decFloatFromUInt32

/* ------------------------------------------------------------------ */
/* decFloatInvert -- logical digitwise INVERT of a decFloat           */
/*                                                                    */
/*   result gets the result of INVERTing df                           */
/*   df     is the decFloat to invert                                 */
/*   set    is the context                                            */
/*   returns result, which will be canonical with sign=0              */
/*                                                                    */
/* The operand must be positive, finite with exponent q=0, and        */
/* comprise just zeros and ones; if not, Invalid operation results.   */
/* ------------------------------------------------------------------ */
decFloat * decFloatInvert(decFloat *result, const decFloat *df,
                          decContext *set) {
  uInt sourhi=DFWORD(df, 0);            // top word of dfs

  if (!DFISUINT01(df) || !DFISCC01(df)) return decInvalid(result, set);
  // the operand is a finite integer (q=0)
  #if DOUBLE
   DFWORD(result, 0)=ZEROWORD|((~sourhi)&0x04009124);
   DFWORD(result, 1)=(~DFWORD(df, 1))   &0x49124491;
  #elif QUAD
   DFWORD(result, 0)=ZEROWORD|((~sourhi)&0x04000912);
   DFWORD(result, 1)=(~DFWORD(df, 1))   &0x44912449;
   DFWORD(result, 2)=(~DFWORD(df, 2))   &0x12449124;
   DFWORD(result, 3)=(~DFWORD(df, 3))   &0x49124491;
  #endif
  return result;
  } // decFloatInvert

/* ------------------------------------------------------------------ */
/* decFloatIs -- decFloat tests (IsSigned, etc.)                      */
/*                                                                    */
/*   df is the decFloat to test                                       */
/*   returns 0 or 1 in a uInt                                         */
/*                                                                    */
/* Many of these could be macros, but having them as real functions   */
/* is a little cleaner (and they can be referred to here by the       */
/* generic names)                                                     */
/* ------------------------------------------------------------------ */
uInt decFloatIsCanonical(const decFloat *df) {
  if (DFISSPECIAL(df)) {
    if (DFISINF(df)) {
      if (DFWORD(df, 0)&ECONMASK) return 0;  // exponent continuation
      if (!DFISCCZERO(df)) return 0;         // coefficient continuation
      return 1;
      }
    // is a NaN
    if (DFWORD(df, 0)&ECONNANMASK) return 0; // exponent continuation
    if (DFISCCZERO(df)) return 1;            // coefficient continuation
    // drop through to check payload
    }
  { // declare block
  #if DOUBLE
    uInt sourhi=DFWORD(df, 0);
    uInt sourlo=DFWORD(df, 1);
    if (CANONDPDOFF(sourhi, 8)
     && CANONDPDTWO(sourhi, sourlo, 30)
     && CANONDPDOFF(sourlo, 20)
     && CANONDPDOFF(sourlo, 10)
     && CANONDPDOFF(sourlo, 0)) return 1;
  #elif QUAD
    uInt sourhi=DFWORD(df, 0);
    uInt sourmh=DFWORD(df, 1);
    uInt sourml=DFWORD(df, 2);
    uInt sourlo=DFWORD(df, 3);
    if (CANONDPDOFF(sourhi, 4)
     && CANONDPDTWO(sourhi, sourmh, 26)
     && CANONDPDOFF(sourmh, 16)
     && CANONDPDOFF(sourmh, 6)
     && CANONDPDTWO(sourmh, sourml, 28)
     && CANONDPDOFF(sourml, 18)
     && CANONDPDOFF(sourml, 8)
     && CANONDPDTWO(sourml, sourlo, 30)
     && CANONDPDOFF(sourlo, 20)
     && CANONDPDOFF(sourlo, 10)
     && CANONDPDOFF(sourlo, 0)) return 1;
  #endif
  } // block
  return 0;    // a declet is non-canonical
  }

uInt decFloatIsFinite(const decFloat *df) {
  return !DFISSPECIAL(df);
  }
uInt decFloatIsInfinite(const decFloat *df) {
  return DFISINF(df);
  }
uInt decFloatIsInteger(const decFloat *df) {
  return DFISINT(df);
  }
uInt decFloatIsLogical(const decFloat *df) {
  return DFISUINT01(df) & DFISCC01(df);
  }
uInt decFloatIsNaN(const decFloat *df) {
  return DFISNAN(df);
  }
uInt decFloatIsNegative(const decFloat *df) {
  return DFISSIGNED(df) && !DFISZERO(df) && !DFISNAN(df);
  }
uInt decFloatIsNormal(const decFloat *df) {
  Int exp;                         // exponent
  if (DFISSPECIAL(df)) return 0;
  if (DFISZERO(df)) return 0;
  // is finite and non-zero
  exp=GETEXPUN(df)                 // get unbiased exponent ..
     +decFloatDigits(df)-1;        // .. and make adjusted exponent
  return (exp>=DECEMIN);           // < DECEMIN is subnormal
  }
uInt decFloatIsPositive(const decFloat *df) {
  return !DFISSIGNED(df) && !DFISZERO(df) && !DFISNAN(df);
  }
uInt decFloatIsSignaling(const decFloat *df) {
  return DFISSNAN(df);
  }
uInt decFloatIsSignalling(const decFloat *df) {
  return DFISSNAN(df);
  }
uInt decFloatIsSigned(const decFloat *df) {
  return DFISSIGNED(df);
  }
uInt decFloatIsSubnormal(const decFloat *df) {
  if (DFISSPECIAL(df)) return 0;
  // is finite
  if (decFloatIsNormal(df)) return 0;
  // it is <Nmin, but could be zero
  if (DFISZERO(df)) return 0;
  return 1;                                  // is subnormal
  }
uInt decFloatIsZero(const decFloat *df) {
  return DFISZERO(df);
  } // decFloatIs...

/* ------------------------------------------------------------------ */
/* decFloatLogB -- return adjusted exponent, by 754 rules             */
/*                                                                    */
/*   result gets the adjusted exponent as an integer, or a NaN etc.   */
/*   df     is the decFloat to be examined                            */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* Notable cases:                                                     */
/*   A<0 -> Use |A|                                                   */
/*   A=0 -> -Infinity (Division by zero)                              */
/*   A=Infinite -> +Infinity (Exact)                                  */
/*   A=1 exactly -> 0 (Exact)                                         */
/*   NaNs are propagated as usual                                     */
/* ------------------------------------------------------------------ */
decFloat * decFloatLogB(decFloat *result, const decFloat *df,
                        decContext *set) {
  Int ae;                                    // adjusted exponent
  if (DFISNAN(df)) return decNaNs(result, df, NULL, set);
  if (DFISINF(df)) {
    DFWORD(result, 0)=0;                     // need +ve
    return decInfinity(result, result);      // canonical +Infinity
    }
  if (DFISZERO(df)) {
    set->status|=DEC_Division_by_zero;       // as per 754
    DFWORD(result, 0)=DECFLOAT_Sign;         // make negative
    return decInfinity(result, result);      // canonical -Infinity
    }
  ae=GETEXPUN(df)                       // get unbiased exponent ..
    +decFloatDigits(df)-1;              // .. and make adjusted exponent
  // ae has limited range (3 digits for DOUBLE and 4 for QUAD), so
  // it is worth using a special case of decFloatFromInt32
  DFWORD(result, 0)=ZEROWORD;           // always
  if (ae<0) {
    DFWORD(result, 0)|=DECFLOAT_Sign;   // -0 so far
    ae=-ae;
    }
  #if DOUBLE
    DFWORD(result, 1)=BIN2DPD[ae];      // a single declet
  #elif QUAD
    DFWORD(result, 1)=0;
    DFWORD(result, 2)=0;
    DFWORD(result, 3)=(ae/1000)<<10;    // is <10, so need no DPD encode
    DFWORD(result, 3)|=BIN2DPD[ae%1000];
  #endif
  return result;
  } // decFloatLogB

/* ------------------------------------------------------------------ */
/* decFloatMax -- return maxnum of two operands                       */
/*                                                                    */
/*   result gets the chosen decFloat                                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* If just one operand is a quiet NaN it is ignored.                  */
/* ------------------------------------------------------------------ */
decFloat * decFloatMax(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  Int comp;
  if (DFISNAN(dfl)) {
    // sNaN or both NaNs leads to normal NaN processing
    if (DFISNAN(dfr) || DFISSNAN(dfl)) return decNaNs(result, dfl, dfr, set);
    return decCanonical(result, dfr);        // RHS is numeric
    }
  if (DFISNAN(dfr)) {
    // sNaN leads to normal NaN processing (both NaN handled above)
    if (DFISSNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    return decCanonical(result, dfl);        // LHS is numeric
    }
  // Both operands are numeric; numeric comparison needed -- use
  // total order for a well-defined choice (and +0 > -0)
  comp=decNumCompare(dfl, dfr, 1);
  if (comp>=0) return decCanonical(result, dfl);
  return decCanonical(result, dfr);
  } // decFloatMax

/* ------------------------------------------------------------------ */
/* decFloatMaxMag -- return maxnummag of two operands                 */
/*                                                                    */
/*   result gets the chosen decFloat                                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* Returns according to the magnitude comparisons if both numeric and */
/* unequal, otherwise returns maxnum                                  */
/* ------------------------------------------------------------------ */
decFloat * decFloatMaxMag(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  Int comp;
  decFloat absl, absr;
  if (DFISNAN(dfl) || DFISNAN(dfr)) return decFloatMax(result, dfl, dfr, set);

  decFloatCopyAbs(&absl, dfl);
  decFloatCopyAbs(&absr, dfr);
  comp=decNumCompare(&absl, &absr, 0);
  if (comp>0) return decCanonical(result, dfl);
  if (comp<0) return decCanonical(result, dfr);
  return decFloatMax(result, dfl, dfr, set);
  } // decFloatMaxMag

/* ------------------------------------------------------------------ */
/* decFloatMin -- return minnum of two operands                       */
/*                                                                    */
/*   result gets the chosen decFloat                                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* If just one operand is a quiet NaN it is ignored.                  */
/* ------------------------------------------------------------------ */
decFloat * decFloatMin(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  Int comp;
  if (DFISNAN(dfl)) {
    // sNaN or both NaNs leads to normal NaN processing
    if (DFISNAN(dfr) || DFISSNAN(dfl)) return decNaNs(result, dfl, dfr, set);
    return decCanonical(result, dfr);        // RHS is numeric
    }
  if (DFISNAN(dfr)) {
    // sNaN leads to normal NaN processing (both NaN handled above)
    if (DFISSNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    return decCanonical(result, dfl);        // LHS is numeric
    }
  // Both operands are numeric; numeric comparison needed -- use
  // total order for a well-defined choice (and +0 > -0)
  comp=decNumCompare(dfl, dfr, 1);
  if (comp<=0) return decCanonical(result, dfl);
  return decCanonical(result, dfr);
  } // decFloatMin

/* ------------------------------------------------------------------ */
/* decFloatMinMag -- return minnummag of two operands                 */
/*                                                                    */
/*   result gets the chosen decFloat                                  */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* Returns according to the magnitude comparisons if both numeric and */
/* unequal, otherwise returns minnum                                  */
/* ------------------------------------------------------------------ */
decFloat * decFloatMinMag(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  Int comp;
  decFloat absl, absr;
  if (DFISNAN(dfl) || DFISNAN(dfr)) return decFloatMin(result, dfl, dfr, set);

  decFloatCopyAbs(&absl, dfl);
  decFloatCopyAbs(&absr, dfr);
  comp=decNumCompare(&absl, &absr, 0);
  if (comp<0) return decCanonical(result, dfl);
  if (comp>0) return decCanonical(result, dfr);
  return decFloatMin(result, dfl, dfr, set);
  } // decFloatMinMag

/* ------------------------------------------------------------------ */
/* decFloatMinus -- negate value, heeding NaNs, etc.                  */
/*                                                                    */
/*   result gets the canonicalized 0-df                               */
/*   df     is the decFloat to minus                                  */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This has the same effect as 0-df where the exponent of the zero is */
/* the same as that of df (if df is finite).                          */
/* The effect is also the same as decFloatCopyNegate except that NaNs */
/* are handled normally (the sign of a NaN is not affected, and an    */
/* sNaN will signal), the result is canonical, and zero gets sign 0.  */
/* ------------------------------------------------------------------ */
decFloat * decFloatMinus(decFloat *result, const decFloat *df,
                         decContext *set) {
  if (DFISNAN(df)) return decNaNs(result, df, NULL, set);
  decCanonical(result, df);                       // copy and check
  if (DFISZERO(df)) DFBYTE(result, 0)&=~0x80;     // turn off sign bit
   else DFBYTE(result, 0)^=0x80;                  // flip sign bit
  return result;
  } // decFloatMinus

/* ------------------------------------------------------------------ */
/* decFloatMultiply -- multiply two decFloats                         */
/*                                                                    */
/*   result gets the result of multiplying dfl and dfr:               */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatMultiply(decFloat *result,
                            const decFloat *dfl, const decFloat *dfr,
                            decContext *set) {
  bcdnum num;                      // for final conversion
  uByte  bcdacc[DECPMAX9*18+1];    // for coefficent in BCD

  if (DFISSPECIAL(dfl) || DFISSPECIAL(dfr)) { // either is special?
    // NaNs are handled as usual
    if (DFISNAN(dfl) || DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    // infinity times zero is bad
    if (DFISINF(dfl) && DFISZERO(dfr)) return decInvalid(result, set);
    if (DFISINF(dfr) && DFISZERO(dfl)) return decInvalid(result, set);
    // both infinite; return canonical infinity with computed sign
    DFWORD(result, 0)=DFWORD(dfl, 0)^DFWORD(dfr, 0); // compute sign
    return decInfinity(result, result);
    }

  /* Here when both operands are finite */
  decFiniteMultiply(&num, bcdacc, dfl, dfr);
  return decFinalize(result, &num, set); // round, check, and lay out
  } // decFloatMultiply

/* ------------------------------------------------------------------ */
/* decFloatNextMinus -- next towards -Infinity                        */
/*                                                                    */
/*   result gets the next lesser decFloat                             */
/*   dfl    is the decFloat to start with                             */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This is 754 nextdown; Invalid is the only status possible (from    */
/* an sNaN).                                                          */
/* ------------------------------------------------------------------ */
decFloat * decFloatNextMinus(decFloat *result, const decFloat *dfl,
                             decContext *set) {
  decFloat delta;                       // tiny increment
  uInt savestat;                        // saves status
  enum rounding saveround;              // .. and mode

  // +Infinity is the special case
  if (DFISINF(dfl) && !DFISSIGNED(dfl)) {
    DFSETNMAX(result);
    return result;                      // [no status to set]
    }
  // other cases are effected by sutracting a tiny delta -- this
  // should be done in a wider format as the delta is unrepresentable
  // here (but can be done with normal add if the sign of zero is
  // treated carefully, because no Inexactitude is interesting);
  // rounding to -Infinity then pushes the result to next below
  decFloatZero(&delta);                 // set up tiny delta
  DFWORD(&delta, DECWORDS-1)=1;         // coefficient=1
  DFWORD(&delta, 0)=DECFLOAT_Sign;      // Sign=1 + biased exponent=0
  // set up for the directional round
  saveround=set->round;                 // save mode
  set->round=DEC_ROUND_FLOOR;           // .. round towards -Infinity
  savestat=set->status;                 // save status
  decFloatAdd(result, dfl, &delta, set);
  // Add rules mess up the sign when going from +Ntiny to 0
  if (DFISZERO(result)) DFWORD(result, 0)^=DECFLOAT_Sign; // correct
  set->status&=DEC_Invalid_operation;   // preserve only sNaN status
  set->status|=savestat;                // restore pending flags
  set->round=saveround;                 // .. and mode
  return result;
  } // decFloatNextMinus

/* ------------------------------------------------------------------ */
/* decFloatNextPlus -- next towards +Infinity                         */
/*                                                                    */
/*   result gets the next larger decFloat                             */
/*   dfl    is the decFloat to start with                             */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This is 754 nextup; Invalid is the only status possible (from      */
/* an sNaN).                                                          */
/* ------------------------------------------------------------------ */
decFloat * decFloatNextPlus(decFloat *result, const decFloat *dfl,
                            decContext *set) {
  uInt savestat;                        // saves status
  enum rounding saveround;              // .. and mode
  decFloat delta;                       // tiny increment

  // -Infinity is the special case
  if (DFISINF(dfl) && DFISSIGNED(dfl)) {
    DFSETNMAX(result);
    DFWORD(result, 0)|=DECFLOAT_Sign;   // make negative
    return result;                      // [no status to set]
    }
  // other cases are effected by sutracting a tiny delta -- this
  // should be done in a wider format as the delta is unrepresentable
  // here (but can be done with normal add if the sign of zero is
  // treated carefully, because no Inexactitude is interesting);
  // rounding to +Infinity then pushes the result to next above
  decFloatZero(&delta);                 // set up tiny delta
  DFWORD(&delta, DECWORDS-1)=1;         // coefficient=1
  DFWORD(&delta, 0)=0;                  // Sign=0 + biased exponent=0
  // set up for the directional round
  saveround=set->round;                 // save mode
  set->round=DEC_ROUND_CEILING;         // .. round towards +Infinity
  savestat=set->status;                 // save status
  decFloatAdd(result, dfl, &delta, set);
  // Add rules mess up the sign when going from -Ntiny to -0
  if (DFISZERO(result)) DFWORD(result, 0)^=DECFLOAT_Sign; // correct
  set->status&=DEC_Invalid_operation;   // preserve only sNaN status
  set->status|=savestat;                // restore pending flags
  set->round=saveround;                 // .. and mode
  return result;
  } // decFloatNextPlus

/* ------------------------------------------------------------------ */
/* decFloatNextToward -- next towards a decFloat                      */
/*                                                                    */
/*   result gets the next decFloat                                    */
/*   dfl    is the decFloat to start with                             */
/*   dfr    is the decFloat to move toward                            */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This is 754-1985 nextafter, as modified during revision (dropped   */
/* from 754-2008); status may be set unless the result is a normal    */
/* number.                                                            */
/* ------------------------------------------------------------------ */
decFloat * decFloatNextToward(decFloat *result,
                              const decFloat *dfl, const decFloat *dfr,
                              decContext *set) {
  decFloat delta;                       // tiny increment or decrement
  decFloat pointone;                    // 1e-1
  uInt  savestat;                       // saves status
  enum  rounding saveround;             // .. and mode
  uInt  deltatop;                       // top word for delta
  Int   comp;                           // work

  if (DFISNAN(dfl) || DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
  // Both are numeric, so Invalid no longer a possibility
  comp=decNumCompare(dfl, dfr, 0);
  if (comp==0) return decFloatCopySign(result, dfl, dfr); // equal
  // unequal; do NextPlus or NextMinus but with different status rules

  if (comp<0) { // lhs<rhs, do NextPlus, see above for commentary
    if (DFISINF(dfl) && DFISSIGNED(dfl)) {   // -Infinity special case
      DFSETNMAX(result);
      DFWORD(result, 0)|=DECFLOAT_Sign;
      return result;
      }
    saveround=set->round;                    // save mode
    set->round=DEC_ROUND_CEILING;            // .. round towards +Infinity
    deltatop=0;                              // positive delta
    }
   else { // lhs>rhs, do NextMinus, see above for commentary
    if (DFISINF(dfl) && !DFISSIGNED(dfl)) {  // +Infinity special case
      DFSETNMAX(result);
      return result;
      }
    saveround=set->round;                    // save mode
    set->round=DEC_ROUND_FLOOR;              // .. round towards -Infinity
    deltatop=DECFLOAT_Sign;                  // negative delta
    }
  savestat=set->status;                      // save status
  // Here, Inexact is needed where appropriate (and hence Underflow,
  // etc.).  Therefore the tiny delta which is otherwise
  // unrepresentable (see NextPlus and NextMinus) is constructed
  // using the multiplication of FMA.
  decFloatZero(&delta);                 // set up tiny delta
  DFWORD(&delta, DECWORDS-1)=1;         // coefficient=1
  DFWORD(&delta, 0)=deltatop;           // Sign + biased exponent=0
  decFloatFromString(&pointone, "1E-1", set); // set up multiplier
  decFloatFMA(result, &delta, &pointone, dfl, set);
  // [Delta is truly tiny, so no need to correct sign of zero]
  // use new status unless the result is normal
  if (decFloatIsNormal(result)) set->status=savestat; // else goes forward
  set->round=saveround;                 // restore mode
  return result;
  } // decFloatNextToward

/* ------------------------------------------------------------------ */
/* decFloatOr -- logical digitwise OR of two decFloats                */
/*                                                                    */
/*   result gets the result of ORing dfl and dfr                      */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result, which will be canonical with sign=0              */
/*                                                                    */
/* The operands must be positive, finite with exponent q=0, and       */
/* comprise just zeros and ones; if not, Invalid operation results.   */
/* ------------------------------------------------------------------ */
decFloat * decFloatOr(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  if (!DFISUINT01(dfl) || !DFISUINT01(dfr)
   || !DFISCC01(dfl)   || !DFISCC01(dfr)) return decInvalid(result, set);
  // the operands are positive finite integers (q=0) with just 0s and 1s
  #if DOUBLE
   DFWORD(result, 0)=ZEROWORD
                   |((DFWORD(dfl, 0) | DFWORD(dfr, 0))&0x04009124);
   DFWORD(result, 1)=(DFWORD(dfl, 1) | DFWORD(dfr, 1))&0x49124491;
  #elif QUAD
   DFWORD(result, 0)=ZEROWORD
                   |((DFWORD(dfl, 0) | DFWORD(dfr, 0))&0x04000912);
   DFWORD(result, 1)=(DFWORD(dfl, 1) | DFWORD(dfr, 1))&0x44912449;
   DFWORD(result, 2)=(DFWORD(dfl, 2) | DFWORD(dfr, 2))&0x12449124;
   DFWORD(result, 3)=(DFWORD(dfl, 3) | DFWORD(dfr, 3))&0x49124491;
  #endif
  return result;
  } // decFloatOr

/* ------------------------------------------------------------------ */
/* decFloatPlus -- add value to 0, heeding NaNs, etc.                 */
/*                                                                    */
/*   result gets the canonicalized 0+df                               */
/*   df     is the decFloat to plus                                   */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This has the same effect as 0+df where the exponent of the zero is */
/* the same as that of df (if df is finite).                          */
/* The effect is also the same as decFloatCopy except that NaNs       */
/* are handled normally (the sign of a NaN is not affected, and an    */
/* sNaN will signal), the result is canonical, and zero gets sign 0.  */
/* ------------------------------------------------------------------ */
decFloat * decFloatPlus(decFloat *result, const decFloat *df,
                        decContext *set) {
  if (DFISNAN(df)) return decNaNs(result, df, NULL, set);
  decCanonical(result, df);                       // copy and check
  if (DFISZERO(df)) DFBYTE(result, 0)&=~0x80;     // turn off sign bit
  return result;
  } // decFloatPlus

/* ------------------------------------------------------------------ */
/* decFloatQuantize -- quantize a decFloat                            */
/*                                                                    */
/*   result gets the result of quantizing dfl to match dfr            */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs), which sets the exponent     */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* Unless there is an error or the result is infinite, the exponent   */
/* of result is guaranteed to be the same as that of dfr.             */
/* ------------------------------------------------------------------ */
decFloat * decFloatQuantize(decFloat *result,
                            const decFloat *dfl, const decFloat *dfr,
                            decContext *set) {
  Int   explb, exprb;         // left and right biased exponents
  uByte *ulsd;                // local LSD pointer
  uByte *ub, *uc;             // work
  Int   drop;                 // ..
  uInt  dpd;                  // ..
  uInt  encode;               // encoding accumulator
  uInt  sourhil, sourhir;     // top words from source decFloats
  uInt  uiwork;               // for macros
  #if QUAD
  uShort uswork;              // ..
  #endif
  // the following buffer holds the coefficient for manipulation
  uByte buf[4+DECPMAX*3+2*QUAD];   // + space for zeros to left or right
  #if DECTRACE
  bcdnum num;                      // for trace displays
  #endif

  /* Start decoding the arguments */
  sourhil=DFWORD(dfl, 0);          // LHS top word
  explb=DECCOMBEXP[sourhil>>26];   // get exponent high bits (in place)
  sourhir=DFWORD(dfr, 0);          // RHS top word
  exprb=DECCOMBEXP[sourhir>>26];

  if (EXPISSPECIAL(explb | exprb)) { // either is special?
    // NaNs are handled as usual
    if (DFISNAN(dfl) || DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
    // one infinity but not both is bad
    if (DFISINF(dfl)!=DFISINF(dfr)) return decInvalid(result, set);
    // both infinite; return canonical infinity with sign of LHS
    return decInfinity(result, dfl);
    }

  /* Here when both arguments are finite */
  // complete extraction of the exponents [no need to unbias]
  explb+=GETECON(dfl);             // + continuation
  exprb+=GETECON(dfr);             // ..

  // calculate the number of digits to drop from the coefficient
  drop=exprb-explb;                // 0 if nothing to do
  if (drop==0) return decCanonical(result, dfl); // return canonical

  // the coefficient is needed; lay it out into buf, offset so zeros
  // can be added before or after as needed -- an extra heading is
  // added so can safely pad Quad DECPMAX-1 zeros to the left by
  // fours
  #define BUFOFF (buf+4+DECPMAX)
  GETCOEFF(dfl, BUFOFF);           // decode from decFloat
  // [now the msd is at BUFOFF and the lsd is at BUFOFF+DECPMAX-1]

  #if DECTRACE
  num.msd=BUFOFF;
  num.lsd=BUFOFF+DECPMAX-1;
  num.exponent=explb-DECBIAS;
  num.sign=sourhil & DECFLOAT_Sign;
  decShowNum(&num, "dfl");
  #endif

  if (drop>0) {                         // [most common case]
    // (this code is very similar to that in decFloatFinalize, but
    // has many differences so is duplicated here -- so any changes
    // may need to be made there, too)
    uByte *roundat;                          // -> re-round digit
    uByte reround;                           // reround value
    // printf("Rounding; drop=%ld\n", (LI)drop);

    // there is at least one zero needed to the left, in all but one
    // exceptional (all-nines) case, so place four zeros now; this is
    // needed almost always and makes rounding all-nines by fours safe
    UBFROMUI(BUFOFF-4, 0);

    // Three cases here:
    //   1. new LSD is in coefficient (almost always)
    //   2. new LSD is digit to left of coefficient (so MSD is
    //      round-for-reround digit)
    //   3. new LSD is to left of case 2 (whole coefficient is sticky)
    // Note that leading zeros can safely be treated as useful digits

    // [duplicate check-stickies code to save a test]
    // [by-digit check for stickies as runs of zeros are rare]
    if (drop<DECPMAX) {                      // NB lengths not addresses
      roundat=BUFOFF+DECPMAX-drop;
      reround=*roundat;
      for (ub=roundat+1; ub<BUFOFF+DECPMAX; ub++) {
        if (*ub!=0) {                        // non-zero to be discarded
          reround=DECSTICKYTAB[reround];     // apply sticky bit
          break;                             // [remainder don't-care]
          }
        } // check stickies
      ulsd=roundat-1;                        // set LSD
      }
     else {                                  // edge case
      if (drop==DECPMAX) {
        roundat=BUFOFF;
        reround=*roundat;
        }
       else {
        roundat=BUFOFF-1;
        reround=0;
        }
      for (ub=roundat+1; ub<BUFOFF+DECPMAX; ub++) {
        if (*ub!=0) {                        // non-zero to be discarded
          reround=DECSTICKYTAB[reround];     // apply sticky bit
          break;                             // [remainder don't-care]
          }
        } // check stickies
      *BUFOFF=0;                             // make a coefficient of 0
      ulsd=BUFOFF;                           // .. at the MSD place
      }

    if (reround!=0) {                        // discarding non-zero
      uInt bump=0;
      set->status|=DEC_Inexact;

      // next decide whether to increment the coefficient
      if (set->round==DEC_ROUND_HALF_EVEN) { // fastpath slowest case
        if (reround>5) bump=1;               // >0.5 goes up
         else if (reround==5)                // exactly 0.5000 ..
          bump=*ulsd & 0x01;                 // .. up iff [new] lsd is odd
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
          if (!(sourhil&DECFLOAT_Sign) && reround>0) bump=1;
          break;} // r-c
        case DEC_ROUND_FLOOR: {
          // same as _UP for negative numbers, and as _DOWN for positive
          // [negative reround cannot occur on 0]
          if (sourhil&DECFLOAT_Sign && reround>0) bump=1;
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

      if (bump!=0) {                         // need increment
        // increment the coefficient; this could give 1000... (after
        // the all nines case)
        ub=ulsd;
        for (; UBTOUI(ub-3)==0x09090909; ub-=4) UBFROMUI(ub-3, 0);
        // now at most 3 digits left to non-9 (usually just the one)
        for (; *ub==9; ub--) *ub=0;
        *ub+=1;
        // [the all-nines case will have carried one digit to the
        // left of the original MSD -- just where it is needed]
        } // bump needed
      } // inexact rounding

    // now clear zeros to the left so exactly DECPMAX digits will be
    // available in the coefficent -- the first word to the left was
    // cleared earlier for safe carry; now add any more needed
    if (drop>4) {
      UBFROMUI(BUFOFF-8, 0);                 // must be at least 5
      for (uc=BUFOFF-12; uc>ulsd-DECPMAX-3; uc-=4) UBFROMUI(uc, 0);
      }
    } // need round (drop>0)

   else { // drop<0; padding with -drop digits is needed
    // This is the case where an error can occur if the padded
    // coefficient will not fit; checking for this can be done in the
    // same loop as padding for zeros if the no-hope and zero cases
    // are checked first
    if (-drop>DECPMAX-1) {                   // cannot fit unless 0
      if (!ISCOEFFZERO(BUFOFF)) return decInvalid(result, set);
      // a zero can have any exponent; just drop through and use it
      ulsd=BUFOFF+DECPMAX-1;
      }
     else { // padding will fit (but may still be too long)
      // final-word mask depends on endianess
      #if DECLITEND
      static const uInt dmask[]={0, 0x000000ff, 0x0000ffff, 0x00ffffff};
      #else
      static const uInt dmask[]={0, 0xff000000, 0xffff0000, 0xffffff00};
      #endif
      // note that here zeros to the right are added by fours, so in
      // the Quad case this could write 36 zeros if the coefficient has
      // fewer than three significant digits (hence the +2*QUAD for buf)
      for (uc=BUFOFF+DECPMAX;; uc+=4) {
        UBFROMUI(uc, 0);
        if (UBTOUI(uc-DECPMAX)!=0) {              // could be bad
          // if all four digits should be zero, definitely bad
          if (uc<=BUFOFF+DECPMAX+(-drop)-4)
            return decInvalid(result, set);
          // must be a 1- to 3-digit sequence; check more carefully
          if ((UBTOUI(uc-DECPMAX)&dmask[(-drop)%4])!=0)
            return decInvalid(result, set);
          break;    // no need for loop end test
          }
        if (uc>=BUFOFF+DECPMAX+(-drop)-4) break;  // done
        }
      ulsd=BUFOFF+DECPMAX+(-drop)-1;
      } // pad and check leading zeros
    } // drop<0

  #if DECTRACE
  num.msd=ulsd-DECPMAX+1;
  num.lsd=ulsd;
  num.exponent=explb-DECBIAS;
  num.sign=sourhil & DECFLOAT_Sign;
  decShowNum(&num, "res");
  #endif

  /*------------------------------------------------------------------*/
  /* At this point the result is DECPMAX digits, ending at ulsd, so   */
  /* fits the encoding exactly; there is no possibility of error      */
  /*------------------------------------------------------------------*/
  encode=((exprb>>DECECONL)<<4) + *(ulsd-DECPMAX+1); // make index
  encode=DECCOMBFROM[encode];                // indexed by (0-2)*16+msd
  // the exponent continuation can be extracted from the original RHS
  encode|=sourhir & ECONMASK;
  encode|=sourhil&DECFLOAT_Sign;             // add the sign from LHS

  // finally encode the coefficient
  // private macro to encode a declet; this version can be used
  // because all coefficient digits exist
  #define getDPD3q(dpd, n) ub=ulsd-(3*(n))-2;                   \
    dpd=BCD2DPD[(*ub*256)+(*(ub+1)*16)+*(ub+2)];

  #if DOUBLE
    getDPD3q(dpd, 4); encode|=dpd<<8;
    getDPD3q(dpd, 3); encode|=dpd>>2;
    DFWORD(result, 0)=encode;
    encode=dpd<<30;
    getDPD3q(dpd, 2); encode|=dpd<<20;
    getDPD3q(dpd, 1); encode|=dpd<<10;
    getDPD3q(dpd, 0); encode|=dpd;
    DFWORD(result, 1)=encode;

  #elif QUAD
    getDPD3q(dpd,10); encode|=dpd<<4;
    getDPD3q(dpd, 9); encode|=dpd>>6;
    DFWORD(result, 0)=encode;
    encode=dpd<<26;
    getDPD3q(dpd, 8); encode|=dpd<<16;
    getDPD3q(dpd, 7); encode|=dpd<<6;
    getDPD3q(dpd, 6); encode|=dpd>>4;
    DFWORD(result, 1)=encode;
    encode=dpd<<28;
    getDPD3q(dpd, 5); encode|=dpd<<18;
    getDPD3q(dpd, 4); encode|=dpd<<8;
    getDPD3q(dpd, 3); encode|=dpd>>2;
    DFWORD(result, 2)=encode;
    encode=dpd<<30;
    getDPD3q(dpd, 2); encode|=dpd<<20;
    getDPD3q(dpd, 1); encode|=dpd<<10;
    getDPD3q(dpd, 0); encode|=dpd;
    DFWORD(result, 3)=encode;
  #endif
  return result;
  } // decFloatQuantize

/* ------------------------------------------------------------------ */
/* decFloatReduce -- reduce finite coefficient to minimum length      */
/*                                                                    */
/*   result gets the reduced decFloat                                 */
/*   df     is the source decFloat                                    */
/*   set    is the context                                            */
/*   returns result, which will be canonical                          */
/*                                                                    */
/* This removes all possible trailing zeros from the coefficient;     */
/* some may remain when the number is very close to Nmax.             */
/* Special values are unchanged and no status is set unless df=sNaN.  */
/* Reduced zero has an exponent q=0.                                  */
/* ------------------------------------------------------------------ */
decFloat * decFloatReduce(decFloat *result, const decFloat *df,
                          decContext *set) {
  bcdnum num;                           // work
  uByte buf[DECPMAX], *ub;              // coefficient and pointer
  if (df!=result) *result=*df;          // copy, if needed
  if (DFISNAN(df)) return decNaNs(result, df, NULL, set);   // sNaN
  // zeros and infinites propagate too
  if (DFISINF(df)) return decInfinity(result, df);     // canonical
  if (DFISZERO(df)) {
    uInt sign=DFWORD(df, 0)&DECFLOAT_Sign;
    decFloatZero(result);
    DFWORD(result, 0)|=sign;
    return result;                      // exponent dropped, sign OK
    }
  // non-zero finite
  GETCOEFF(df, buf);
  ub=buf+DECPMAX-1;                     // -> lsd
  if (*ub) return result;               // no trailing zeros
  for (ub--; *ub==0;) ub--;             // terminates because non-zero
  // *ub is the first non-zero from the right
  num.sign=DFWORD(df, 0)&DECFLOAT_Sign; // set up number...
  num.exponent=GETEXPUN(df)+(Int)(buf+DECPMAX-1-ub); // adjusted exponent
  num.msd=buf;
  num.lsd=ub;
  return decFinalize(result, &num, set);
  } // decFloatReduce

/* ------------------------------------------------------------------ */
/* decFloatRemainder -- integer divide and return remainder           */
/*                                                                    */
/*   result gets the remainder of dividing dfl by dfr:                */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatRemainder(decFloat *result,
                             const decFloat *dfl, const decFloat *dfr,
                             decContext *set) {
  return decDivide(result, dfl, dfr, set, REMAINDER);
  } // decFloatRemainder

/* ------------------------------------------------------------------ */
/* decFloatRemainderNear -- integer divide to nearest and remainder   */
/*                                                                    */
/*   result gets the remainder of dividing dfl by dfr:                */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This is the IEEE remainder, where the nearest integer is used.     */
/* ------------------------------------------------------------------ */
decFloat * decFloatRemainderNear(decFloat *result,
                             const decFloat *dfl, const decFloat *dfr,
                             decContext *set) {
  return decDivide(result, dfl, dfr, set, REMNEAR);
  } // decFloatRemainderNear

/* ------------------------------------------------------------------ */
/* decFloatRotate -- rotate the coefficient of a decFloat left/right  */
/*                                                                    */
/*   result gets the result of rotating dfl                           */
/*   dfl    is the source decFloat to rotate                          */
/*   dfr    is the count of digits to rotate, an integer (with q=0)   */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* The digits of the coefficient of dfl are rotated to the left (if   */
/* dfr is positive) or to the right (if dfr is negative) without      */
/* adjusting the exponent or the sign of dfl.                         */
/*                                                                    */
/* dfr must be in the range -DECPMAX through +DECPMAX.                */
/* NaNs are propagated as usual.  An infinite dfl is unaffected (but  */
/* dfr must be valid).  No status is set unless dfr is invalid or an  */
/* operand is an sNaN.  The result is canonical.                      */
/* ------------------------------------------------------------------ */
#define PHALF (ROUNDUP(DECPMAX/2, 4))   // half length, rounded up
decFloat * decFloatRotate(decFloat *result,
                         const decFloat *dfl, const decFloat *dfr,
                         decContext *set) {
  Int rotate;                           // dfr as an Int
  uByte buf[DECPMAX+PHALF];             // coefficient + half
  uInt digits, savestat;                // work
  bcdnum num;                           // ..
  uByte *ub;                            // ..

  if (DFISNAN(dfl)||DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
  if (!DFISINT(dfr)) return decInvalid(result, set);
  digits=decFloatDigits(dfr);                    // calculate digits
  if (digits>2) return decInvalid(result, set);  // definitely out of range
  rotate=DPD2BIN[DFWORD(dfr, DECWORDS-1)&0x3ff]; // is in bottom declet
  if (rotate>DECPMAX) return decInvalid(result, set); // too big
  // [from here on no error or status change is possible]
  if (DFISINF(dfl)) return decInfinity(result, dfl);  // canonical
  // handle no-rotate cases
  if (rotate==0 || rotate==DECPMAX) return decCanonical(result, dfl);
  // a real rotate is needed: 0 < rotate < DECPMAX
  // reduce the rotation to no more than half to reduce copying later
  // (for QUAD in fact half + 2 digits)
  if (DFISSIGNED(dfr)) rotate=-rotate;
  if (abs(rotate)>PHALF) {
    if (rotate<0) rotate=DECPMAX+rotate;
     else rotate=rotate-DECPMAX;
    }
  // now lay out the coefficient, leaving room to the right or the
  // left depending on the direction of rotation
  ub=buf;
  if (rotate<0) ub+=PHALF;    // rotate right, so space to left
  GETCOEFF(dfl, ub);
  // copy half the digits to left or right, and set num.msd
  if (rotate<0) {
    memcpy(buf, buf+DECPMAX, PHALF);
    num.msd=buf+PHALF+rotate;
    }
   else {
    memcpy(buf+DECPMAX, buf, PHALF);
    num.msd=buf+rotate;
    }
  // fill in rest of num
  num.lsd=num.msd+DECPMAX-1;
  num.sign=DFWORD(dfl, 0)&DECFLOAT_Sign;
  num.exponent=GETEXPUN(dfl);
  savestat=set->status;                 // record
  decFinalize(result, &num, set);
  set->status=savestat;                 // restore
  return result;
  } // decFloatRotate

/* ------------------------------------------------------------------ */
/* decFloatSameQuantum -- test decFloats for same quantum             */
/*                                                                    */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   returns 1 if the operands have the same quantum, 0 otherwise     */
/*                                                                    */
/* No error is possible and no status results.                        */
/* ------------------------------------------------------------------ */
uInt decFloatSameQuantum(const decFloat *dfl, const decFloat *dfr) {
  if (DFISSPECIAL(dfl) || DFISSPECIAL(dfr)) {
    if (DFISNAN(dfl) && DFISNAN(dfr)) return 1;
    if (DFISINF(dfl) && DFISINF(dfr)) return 1;
    return 0;  // any other special mixture gives false
    }
  if (GETEXP(dfl)==GETEXP(dfr)) return 1; // biased exponents match
  return 0;
  } // decFloatSameQuantum

/* ------------------------------------------------------------------ */
/* decFloatScaleB -- multiply by a power of 10, as per 754            */
/*                                                                    */
/*   result gets the result of the operation                          */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs), am integer (with q=0)       */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* This computes result=dfl x 10**dfr where dfr is an integer in the  */
/* range +/-2*(emax+pmax), typically resulting from LogB.             */
/* Underflow and Overflow (with Inexact) may occur.  NaNs propagate   */
/* as usual.                                                          */
/* ------------------------------------------------------------------ */
#define SCALEBMAX 2*(DECEMAX+DECPMAX)   // D=800, Q=12356
decFloat * decFloatScaleB(decFloat *result,
                          const decFloat *dfl, const decFloat *dfr,
                          decContext *set) {
  uInt digits;                          // work
  Int  expr;                            // dfr as an Int

  if (DFISNAN(dfl)||DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
  if (!DFISINT(dfr)) return decInvalid(result, set);
  digits=decFloatDigits(dfr);                // calculate digits

  #if DOUBLE
  if (digits>3) return decInvalid(result, set);   // definitely out of range
  expr=DPD2BIN[DFWORD(dfr, 1)&0x3ff];             // must be in bottom declet
  #elif QUAD
  if (digits>5) return decInvalid(result, set);   // definitely out of range
  expr=DPD2BIN[DFWORD(dfr, 3)&0x3ff]              // in bottom 2 declets ..
      +DPD2BIN[(DFWORD(dfr, 3)>>10)&0x3ff]*1000;  // ..
  #endif
  if (expr>SCALEBMAX) return decInvalid(result, set);  // oops
  // [from now on no error possible]
  if (DFISINF(dfl)) return decInfinity(result, dfl);   // canonical
  if (DFISSIGNED(dfr)) expr=-expr;
  // dfl is finite and expr is valid
  *result=*dfl;                              // copy to target
  return decFloatSetExponent(result, set, GETEXPUN(result)+expr);
  } // decFloatScaleB

/* ------------------------------------------------------------------ */
/* decFloatShift -- shift the coefficient of a decFloat left or right */
/*                                                                    */
/*   result gets the result of shifting dfl                           */
/*   dfl    is the source decFloat to shift                           */
/*   dfr    is the count of digits to shift, an integer (with q=0)    */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* The digits of the coefficient of dfl are shifted to the left (if   */
/* dfr is positive) or to the right (if dfr is negative) without      */
/* adjusting the exponent or the sign of dfl.                         */
/*                                                                    */
/* dfr must be in the range -DECPMAX through +DECPMAX.                */
/* NaNs are propagated as usual.  An infinite dfl is unaffected (but  */
/* dfr must be valid).  No status is set unless dfr is invalid or an  */
/* operand is an sNaN.  The result is canonical.                      */
/* ------------------------------------------------------------------ */
decFloat * decFloatShift(decFloat *result,
                         const decFloat *dfl, const decFloat *dfr,
                         decContext *set) {
  Int    shift;                         // dfr as an Int
  uByte  buf[DECPMAX*2];                // coefficient + padding
  uInt   digits, savestat;              // work
  bcdnum num;                           // ..
  uInt   uiwork;                        // for macros

  if (DFISNAN(dfl)||DFISNAN(dfr)) return decNaNs(result, dfl, dfr, set);
  if (!DFISINT(dfr)) return decInvalid(result, set);
  digits=decFloatDigits(dfr);                     // calculate digits
  if (digits>2) return decInvalid(result, set);   // definitely out of range
  shift=DPD2BIN[DFWORD(dfr, DECWORDS-1)&0x3ff];   // is in bottom declet
  if (shift>DECPMAX) return decInvalid(result, set);   // too big
  // [from here on no error or status change is possible]

  if (DFISINF(dfl)) return decInfinity(result, dfl); // canonical
  // handle no-shift and all-shift (clear to zero) cases
  if (shift==0) return decCanonical(result, dfl);
  if (shift==DECPMAX) {                      // zero with sign
    uByte sign=(uByte)(DFBYTE(dfl, 0)&0x80); // save sign bit
    decFloatZero(result);                    // make +0
    DFBYTE(result, 0)=(uByte)(DFBYTE(result, 0)|sign); // and set sign
    // [cannot safely use CopySign]
    return result;
    }
  // a real shift is needed: 0 < shift < DECPMAX
  num.sign=DFWORD(dfl, 0)&DECFLOAT_Sign;
  num.exponent=GETEXPUN(dfl);
  num.msd=buf;
  GETCOEFF(dfl, buf);
  if (DFISSIGNED(dfr)) { // shift right
    // edge cases are taken care of, so this is easy
    num.lsd=buf+DECPMAX-shift-1;
    }
   else { // shift left -- zero padding needed to right
    UBFROMUI(buf+DECPMAX, 0);           // 8 will handle most cases
    UBFROMUI(buf+DECPMAX+4, 0);         // ..
    if (shift>8) memset(buf+DECPMAX+8, 0, 8+QUAD*18); // all other cases
    num.msd+=shift;
    num.lsd=num.msd+DECPMAX-1;
    }
  savestat=set->status;                 // record
  decFinalize(result, &num, set);
  set->status=savestat;                 // restore
  return result;
  } // decFloatShift

/* ------------------------------------------------------------------ */
/* decFloatSubtract -- subtract a decFloat from another               */
/*                                                                    */
/*   result gets the result of subtracting dfr from dfl:              */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatSubtract(decFloat *result,
                            const decFloat *dfl, const decFloat *dfr,
                            decContext *set) {
  decFloat temp;
  // NaNs must propagate without sign change
  if (DFISNAN(dfr)) return decFloatAdd(result, dfl, dfr, set);
  temp=*dfr;                                   // make a copy
  DFBYTE(&temp, 0)^=0x80;                      // flip sign
  return decFloatAdd(result, dfl, &temp, set); // and add to the lhs
  } // decFloatSubtract

/* ------------------------------------------------------------------ */
/* decFloatToInt -- round to 32-bit binary integer (4 flavours)       */
/*                                                                    */
/*   df    is the decFloat to round                                   */
/*   set   is the context                                             */
/*   round is the rounding mode to use                                */
/*   returns a uInt or an Int, rounded according to the name          */
/*                                                                    */
/* Invalid will always be signaled if df is a NaN, is Infinite, or is */
/* outside the range of the target; Inexact will not be signaled for  */
/* simple rounding unless 'Exact' appears in the name.                */
/* ------------------------------------------------------------------ */
uInt decFloatToUInt32(const decFloat *df, decContext *set,
                      enum rounding round) {
  return decToInt32(df, set, round, 0, 1);}

uInt decFloatToUInt32Exact(const decFloat *df, decContext *set,
                           enum rounding round) {
  return decToInt32(df, set, round, 1, 1);}

Int decFloatToInt32(const decFloat *df, decContext *set,
                    enum rounding round) {
  return (Int)decToInt32(df, set, round, 0, 0);}

Int decFloatToInt32Exact(const decFloat *df, decContext *set,
                         enum rounding round) {
  return (Int)decToInt32(df, set, round, 1, 0);}

/* ------------------------------------------------------------------ */
/* decFloatToIntegral -- round to integral value (two flavours)       */
/*                                                                    */
/*   result gets the result                                           */
/*   df     is the decFloat to round                                  */
/*   set    is the context                                            */
/*   round  is the rounding mode to use                               */
/*   returns result                                                   */
/*                                                                    */
/* No exceptions, even Inexact, are raised except for sNaN input, or  */
/* if 'Exact' appears in the name.                                    */
/* ------------------------------------------------------------------ */
decFloat * decFloatToIntegralValue(decFloat *result, const decFloat *df,
                                   decContext *set, enum rounding round) {
  return decToIntegral(result, df, set, round, 0);}

decFloat * decFloatToIntegralExact(decFloat *result, const decFloat *df,
                                   decContext *set) {
  return decToIntegral(result, df, set, set->round, 1);}

/* ------------------------------------------------------------------ */
/* decFloatXor -- logical digitwise XOR of two decFloats              */
/*                                                                    */
/*   result gets the result of XORing dfl and dfr                     */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs)                              */
/*   set    is the context                                            */
/*   returns result, which will be canonical with sign=0              */
/*                                                                    */
/* The operands must be positive, finite with exponent q=0, and       */
/* comprise just zeros and ones; if not, Invalid operation results.   */
/* ------------------------------------------------------------------ */
decFloat * decFloatXor(decFloat *result,
                       const decFloat *dfl, const decFloat *dfr,
                       decContext *set) {
  if (!DFISUINT01(dfl) || !DFISUINT01(dfr)
   || !DFISCC01(dfl)   || !DFISCC01(dfr)) return decInvalid(result, set);
  // the operands are positive finite integers (q=0) with just 0s and 1s
  #if DOUBLE
   DFWORD(result, 0)=ZEROWORD
                   |((DFWORD(dfl, 0) ^ DFWORD(dfr, 0))&0x04009124);
   DFWORD(result, 1)=(DFWORD(dfl, 1) ^ DFWORD(dfr, 1))&0x49124491;
  #elif QUAD
   DFWORD(result, 0)=ZEROWORD
                   |((DFWORD(dfl, 0) ^ DFWORD(dfr, 0))&0x04000912);
   DFWORD(result, 1)=(DFWORD(dfl, 1) ^ DFWORD(dfr, 1))&0x44912449;
   DFWORD(result, 2)=(DFWORD(dfl, 2) ^ DFWORD(dfr, 2))&0x12449124;
   DFWORD(result, 3)=(DFWORD(dfl, 3) ^ DFWORD(dfr, 3))&0x49124491;
  #endif
  return result;
  } // decFloatXor

/* ------------------------------------------------------------------ */
/* decInvalid -- set Invalid_operation result                         */
/*                                                                    */
/*   result gets a canonical NaN                                      */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* status has Invalid_operation added                                 */
/* ------------------------------------------------------------------ */
static decFloat *decInvalid(decFloat *result, decContext *set) {
  decFloatZero(result);
  DFWORD(result, 0)=DECFLOAT_qNaN;
  set->status|=DEC_Invalid_operation;
  return result;
  } // decInvalid

/* ------------------------------------------------------------------ */
/* decInfinity -- set canonical Infinity with sign from a decFloat    */
/*                                                                    */
/*   result gets a canonical Infinity                                 */
/*   df     is source decFloat (only the sign is used)                */
/*   returns result                                                   */
/*                                                                    */
/* df may be the same as result                                       */
/* ------------------------------------------------------------------ */
static decFloat *decInfinity(decFloat *result, const decFloat *df) {
  uInt sign=DFWORD(df, 0);         // save source signword
  decFloatZero(result);            // clear everything
  DFWORD(result, 0)=DECFLOAT_Inf | (sign & DECFLOAT_Sign);
  return result;
  } // decInfinity

/* ------------------------------------------------------------------ */
/* decNaNs -- handle NaN argument(s)                                  */
/*                                                                    */
/*   result gets the result of handling dfl and dfr, one or both of   */
/*          which is a NaN                                            */
/*   dfl    is the first decFloat (lhs)                               */
/*   dfr    is the second decFloat (rhs) -- may be NULL for a single- */
/*          operand operation                                         */
/*   set    is the context                                            */
/*   returns result                                                   */
/*                                                                    */
/* Called when one or both operands is a NaN, and propagates the      */
/* appropriate result to res.  When an sNaN is found, it is changed   */
/* to a qNaN and Invalid operation is set.                            */
/* ------------------------------------------------------------------ */
static decFloat *decNaNs(decFloat *result,
                         const decFloat *dfl, const decFloat *dfr,
                         decContext *set) {
  // handle sNaNs first
  if (dfr!=NULL && DFISSNAN(dfr) && !DFISSNAN(dfl)) dfl=dfr; // use RHS
  if (DFISSNAN(dfl)) {
    decCanonical(result, dfl);          // propagate canonical sNaN
    DFWORD(result, 0)&=~(DECFLOAT_qNaN ^ DECFLOAT_sNaN); // quiet
    set->status|=DEC_Invalid_operation;
    return result;
    }
  // one or both is a quiet NaN
  if (!DFISNAN(dfl)) dfl=dfr;           // RHS must be NaN, use it
  return decCanonical(result, dfl);     // propagate canonical qNaN
  } // decNaNs

/* ------------------------------------------------------------------ */
/* decNumCompare -- numeric comparison of two decFloats               */
/*                                                                    */
/*   dfl    is the left-hand decFloat, which is not a NaN             */
/*   dfr    is the right-hand decFloat, which is not a NaN            */
/*   tot    is 1 for total order compare, 0 for simple numeric        */
/*   returns -1, 0, or +1 for dfl<dfr, dfl=dfr, dfl>dfr               */
/*                                                                    */
/* No error is possible; status and mode are unchanged.               */
/* ------------------------------------------------------------------ */
static Int decNumCompare(const decFloat *dfl, const decFloat *dfr, Flag tot) {
  Int   sigl, sigr;                     // LHS and RHS non-0 signums
  Int   shift;                          // shift needed to align operands
  uByte *ub, *uc;                       // work
  uInt  uiwork;                         // for macros
  // buffers +2 if Quad (36 digits), need double plus 4 for safe padding
  uByte bufl[DECPMAX*2+QUAD*2+4];       // for LHS coefficient + padding
  uByte bufr[DECPMAX*2+QUAD*2+4];       // for RHS coefficient + padding

  sigl=1;
  if (DFISSIGNED(dfl)) {
    if (!DFISSIGNED(dfr)) {             // -LHS +RHS
      if (DFISZERO(dfl) && DFISZERO(dfr) && !tot) return 0;
      return -1;                        // RHS wins
      }
    sigl=-1;
    }
  if (DFISSIGNED(dfr)) {
    if (!DFISSIGNED(dfl)) {             // +LHS -RHS
      if (DFISZERO(dfl) && DFISZERO(dfr) && !tot) return 0;
      return +1;                        // LHS wins
      }
    }

  // signs are the same; operand(s) could be zero
  sigr=-sigl;                           // sign to return if abs(RHS) wins

  if (DFISINF(dfl)) {
    if (DFISINF(dfr)) return 0;         // both infinite & same sign
    return sigl;                        // inf > n
    }
  if (DFISINF(dfr)) return sigr;        // n < inf [dfl is finite]

  // here, both are same sign and finite; calculate their offset
  shift=GETEXP(dfl)-GETEXP(dfr);        // [0 means aligned]
  // [bias can be ignored -- the absolute exponent is not relevant]

  if (DFISZERO(dfl)) {
    if (!DFISZERO(dfr)) return sigr;    // LHS=0, RHS!=0
    // both are zero, return 0 if both same exponent or numeric compare
    if (shift==0 || !tot) return 0;
    if (shift>0) return sigl;
    return sigr;                        // [shift<0]
    }
   else {                               // LHS!=0
    if (DFISZERO(dfr)) return sigl;     // LHS!=0, RHS=0
    }
  // both are known to be non-zero at this point

  // if the exponents are so different that the coefficients do not
  // overlap (by even one digit) then a full comparison is not needed
  if (abs(shift)>=DECPMAX) {            // no overlap
    // coefficients are known to be non-zero
    if (shift>0) return sigl;
    return sigr;                        // [shift<0]
    }

  // decode the coefficients
  // (shift both right two if Quad to make a multiple of four)
  #if QUAD
    UBFROMUI(bufl, 0);
    UBFROMUI(bufr, 0);
  #endif
  GETCOEFF(dfl, bufl+QUAD*2);           // decode from decFloat
  GETCOEFF(dfr, bufr+QUAD*2);           // ..
  if (shift==0) {                       // aligned; common and easy
    // all multiples of four, here
    for (ub=bufl, uc=bufr; ub<bufl+DECPMAX+QUAD*2; ub+=4, uc+=4) {
      uInt ui=UBTOUI(ub);
      if (ui==UBTOUI(uc)) continue;     // so far so same
      // about to find a winner; go by bytes in case little-endian
      for (;; ub++, uc++) {
        if (*ub>*uc) return sigl;       // difference found
        if (*ub<*uc) return sigr;       // ..
        }
      }
    } // aligned
   else if (shift>0) {                  // lhs to left
    ub=bufl;                            // RHS pointer
    // pad bufl so right-aligned; most shifts will fit in 8
    UBFROMUI(bufl+DECPMAX+QUAD*2, 0);   // add eight zeros
    UBFROMUI(bufl+DECPMAX+QUAD*2+4, 0); // ..
    if (shift>8) {
      // more than eight; fill the rest, and also worth doing the
      // lead-in by fours
      uByte *up;                        // work
      uByte *upend=bufl+DECPMAX+QUAD*2+shift;
      for (up=bufl+DECPMAX+QUAD*2+8; up<upend; up+=4) UBFROMUI(up, 0);
      // [pads up to 36 in all for Quad]
      for (;; ub+=4) {
        if (UBTOUI(ub)!=0) return sigl;
        if (ub+4>bufl+shift-4) break;
        }
      }
    // check remaining leading digits
    for (; ub<bufl+shift; ub++) if (*ub!=0) return sigl;
    // now start the overlapped part; bufl has been padded, so the
    // comparison can go for the full length of bufr, which is a
    // multiple of 4 bytes
    for (uc=bufr; ; uc+=4, ub+=4) {
      uInt ui=UBTOUI(ub);
      if (ui!=UBTOUI(uc)) {             // mismatch found
        for (;; uc++, ub++) {           // check from left [little-endian?]
          if (*ub>*uc) return sigl;     // difference found
          if (*ub<*uc) return sigr;     // ..
          }
        } // mismatch
      if (uc==bufr+QUAD*2+DECPMAX-4) break; // all checked
      }
    } // shift>0

   else { // shift<0) .. RHS is to left of LHS; mirror shift>0
    uc=bufr;                            // RHS pointer
    // pad bufr so right-aligned; most shifts will fit in 8
    UBFROMUI(bufr+DECPMAX+QUAD*2, 0);   // add eight zeros
    UBFROMUI(bufr+DECPMAX+QUAD*2+4, 0); // ..
    if (shift<-8) {
      // more than eight; fill the rest, and also worth doing the
      // lead-in by fours
      uByte *up;                        // work
      uByte *upend=bufr+DECPMAX+QUAD*2-shift;
      for (up=bufr+DECPMAX+QUAD*2+8; up<upend; up+=4) UBFROMUI(up, 0);
      // [pads up to 36 in all for Quad]
      for (;; uc+=4) {
        if (UBTOUI(uc)!=0) return sigr;
        if (uc+4>bufr-shift-4) break;
        }
      }
    // check remaining leading digits
    for (; uc<bufr-shift; uc++) if (*uc!=0) return sigr;
    // now start the overlapped part; bufr has been padded, so the
    // comparison can go for the full length of bufl, which is a
    // multiple of 4 bytes
    for (ub=bufl; ; ub+=4, uc+=4) {
      uInt ui=UBTOUI(ub);
      if (ui!=UBTOUI(uc)) {             // mismatch found
        for (;; ub++, uc++) {           // check from left [little-endian?]
          if (*ub>*uc) return sigl;     // difference found
          if (*ub<*uc) return sigr;     // ..
          }
        } // mismatch
      if (ub==bufl+QUAD*2+DECPMAX-4) break; // all checked
      }
    } // shift<0

  // Here when compare equal
  if (!tot) return 0;                   // numerically equal
  // total ordering .. exponent matters
  if (shift>0) return sigl;             // total order by exponent
  if (shift<0) return sigr;             // ..
  return 0;
  } // decNumCompare

/* ------------------------------------------------------------------ */
/* decToInt32 -- local routine to effect ToInteger conversions        */
/*                                                                    */
/*   df     is the decFloat to convert                                */
/*   set    is the context                                            */
/*   rmode  is the rounding mode to use                               */
/*   exact  is 1 if Inexact should be signalled                       */
/*   unsign is 1 if the result a uInt, 0 if an Int (cast to uInt)     */
/*   returns 32-bit result as a uInt                                  */
/*                                                                    */
/* Invalid is set is df is a NaN, is infinite, or is out-of-range; in */
/* these cases 0 is returned.                                         */
/* ------------------------------------------------------------------ */
static uInt decToInt32(const decFloat *df, decContext *set,
                       enum rounding rmode, Flag exact, Flag unsign) {
  Int  exp;                        // exponent
  uInt sourhi, sourpen, sourlo;    // top word from source decFloat ..
  uInt hi, lo;                     // .. penultimate, least, etc.
  decFloat zero, result;           // work
  Int  i;                          // ..

  /* Start decoding the argument */
  sourhi=DFWORD(df, 0);                 // top word
  exp=DECCOMBEXP[sourhi>>26];           // get exponent high bits (in place)
  if (EXPISSPECIAL(exp)) {              // is special?
    set->status|=DEC_Invalid_operation; // signal
    return 0;
    }

  /* Here when the argument is finite */
  if (GETEXPUN(df)==0) result=*df;      // already a true integer
   else {                               // need to round to integer
    enum rounding saveround;            // saver
    uInt savestatus;                    // ..
    saveround=set->round;               // save rounding mode ..
    savestatus=set->status;             // .. and status
    set->round=rmode;                   // set mode
    decFloatZero(&zero);                // make 0E+0
    set->status=0;                      // clear
    decFloatQuantize(&result, df, &zero, set); // [this may fail]
    set->round=saveround;               // restore rounding mode ..
    if (exact) set->status|=savestatus; // include Inexact
     else set->status=savestatus;       // .. or just original status
    }

  // only the last four declets of the coefficient can contain
  // non-zero; check for others (and also NaN or Infinity from the
  // Quantize) first (see DFISZERO for explanation):
  // decFloatShow(&result, "sofar");
  #if DOUBLE
  if ((DFWORD(&result, 0)&0x1c03ff00)!=0
   || (DFWORD(&result, 0)&0x60000000)==0x60000000) {
  #elif QUAD
  if ((DFWORD(&result, 2)&0xffffff00)!=0
   ||  DFWORD(&result, 1)!=0
   || (DFWORD(&result, 0)&0x1c003fff)!=0
   || (DFWORD(&result, 0)&0x60000000)==0x60000000) {
  #endif
    set->status|=DEC_Invalid_operation; // Invalid or out of range
    return 0;
    }
  // get last twelve digits of the coefficent into hi & ho, base
  // 10**9 (see GETCOEFFBILL):
  sourlo=DFWORD(&result, DECWORDS-1);
  lo=DPD2BIN0[sourlo&0x3ff]
    +DPD2BINK[(sourlo>>10)&0x3ff]
    +DPD2BINM[(sourlo>>20)&0x3ff];
  sourpen=DFWORD(&result, DECWORDS-2);
  hi=DPD2BIN0[((sourpen<<2) | (sourlo>>30))&0x3ff];

  // according to request, check range carefully
  if (unsign) {
    if (hi>4 || (hi==4 && lo>294967295) || (hi+lo!=0 && DFISSIGNED(&result))) {
      set->status|=DEC_Invalid_operation; // out of range
      return 0;
      }
    return hi*BILLION+lo;
    }
  // signed
  if (hi>2 || (hi==2 && lo>147483647)) {
    // handle the usual edge case
    if (lo==147483648 && hi==2 && DFISSIGNED(&result)) return 0x80000000;
    set->status|=DEC_Invalid_operation; // truly out of range
    return 0;
    }
  i=hi*BILLION+lo;
  if (DFISSIGNED(&result)) i=-i;
  return (uInt)i;
  } // decToInt32

/* ------------------------------------------------------------------ */
/* decToIntegral -- local routine to effect ToIntegral value          */
/*                                                                    */
/*   result gets the result                                           */
/*   df     is the decFloat to round                                  */
/*   set    is the context                                            */
/*   rmode  is the rounding mode to use                               */
/*   exact  is 1 if Inexact should be signalled                       */
/*   returns result                                                   */
/* ------------------------------------------------------------------ */
static decFloat * decToIntegral(decFloat *result, const decFloat *df,
                                decContext *set, enum rounding rmode,
                                Flag exact) {
  Int  exp;                        // exponent
  uInt sourhi;                     // top word from source decFloat
  enum rounding saveround;         // saver
  uInt savestatus;                 // ..
  decFloat zero;                   // work

  /* Start decoding the argument */
  sourhi=DFWORD(df, 0);            // top word
  exp=DECCOMBEXP[sourhi>>26];      // get exponent high bits (in place)

  if (EXPISSPECIAL(exp)) {         // is special?
    // NaNs are handled as usual
    if (DFISNAN(df)) return decNaNs(result, df, NULL, set);
    // must be infinite; return canonical infinity with sign of df
    return decInfinity(result, df);
    }

  /* Here when the argument is finite */
  // complete extraction of the exponent
  exp+=GETECON(df)-DECBIAS;             // .. + continuation and unbias

  if (exp>=0) return decCanonical(result, df); // already integral

  saveround=set->round;                 // save rounding mode ..
  savestatus=set->status;               // .. and status
  set->round=rmode;                     // set mode
  decFloatZero(&zero);                  // make 0E+0
  decFloatQuantize(result, df, &zero, set); // 'integrate'; cannot fail
  set->round=saveround;                 // restore rounding mode ..
  if (!exact) set->status=savestatus;   // .. and status, unless exact
  return result;
  } // decToIntegral
