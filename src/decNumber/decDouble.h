/* ------------------------------------------------------------------ */
/* decDouble.h -- Decimal 64-bit format module header                 */
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

#if !defined(DECDOUBLE)
  #define DECDOUBLE

  #define DECDOUBLENAME       "decimalDouble"         /* Short name   */
  #define DECDOUBLETITLE      "Decimal 64-bit datum"  /* Verbose name */
  #define DECDOUBLEAUTHOR     "Mike Cowlishaw"        /* Who to blame */

  /* parameters for decDoubles */
  #define DECDOUBLE_Bytes   8      /* length                          */
  #define DECDOUBLE_Pmax    16     /* maximum precision (digits)      */
  #define DECDOUBLE_Emin   -383    /* minimum adjusted exponent       */
  #define DECDOUBLE_Emax    384    /* maximum adjusted exponent       */
  #define DECDOUBLE_EmaxD   3      /* maximum exponent digits         */
  #define DECDOUBLE_Bias    398    /* bias for the exponent           */
  #define DECDOUBLE_String  25     /* maximum string length, +1       */
  #define DECDOUBLE_EconL   8      /* exponent continuation length    */
  #define DECDOUBLE_Declets 5      /* count of declets                */
  /* highest biased exponent (Elimit-1) */
  #define DECDOUBLE_Ehigh (DECDOUBLE_Emax + DECDOUBLE_Bias - (DECDOUBLE_Pmax-1))

  /* Required includes                                                */
  #include "decContext.h"
  #include "decQuad.h"

  /* The decDouble decimal 64-bit type, accessible by all sizes */
  typedef union {
    uint8_t   bytes[DECDOUBLE_Bytes];   /* fields: 1, 5, 8, 50 bits */
    uint16_t shorts[DECDOUBLE_Bytes/2];
    uint32_t  words[DECDOUBLE_Bytes/4];
    #if DECUSE64
    uint64_t  longs[DECDOUBLE_Bytes/8];
    #endif
    } decDouble;

  /* ---------------------------------------------------------------- */
  /* Routines -- implemented as decFloat routines in common files     */
  /* ---------------------------------------------------------------- */

  /* Utilities and conversions, extractors, etc.) */
  extern decDouble * decDoubleFromBCD(decDouble *, int32_t, const uint8_t *, int32_t);
  extern decDouble * decDoubleFromInt32(decDouble *, int32_t);
  extern decDouble * decDoubleFromPacked(decDouble *, int32_t, const uint8_t *);
  extern decDouble * decDoubleFromPackedChecked(decDouble *, int32_t, const uint8_t *);
  extern decDouble * decDoubleFromString(decDouble *, const char *, decContext *);
  extern decDouble * decDoubleFromUInt32(decDouble *, uint32_t);
  extern decDouble * decDoubleFromWider(decDouble *, const decQuad *, decContext *);
  extern int32_t     decDoubleGetCoefficient(const decDouble *, uint8_t *);
  extern int32_t     decDoubleGetExponent(const decDouble *);
  extern decDouble * decDoubleSetCoefficient(decDouble *, const uint8_t *, int32_t);
  extern decDouble * decDoubleSetExponent(decDouble *, decContext *, int32_t);
  extern void        decDoubleShow(const decDouble *, const char *);
  extern int32_t     decDoubleToBCD(const decDouble *, int32_t *, uint8_t *);
  extern char      * decDoubleToEngString(const decDouble *, char *);
  extern int32_t     decDoubleToInt32(const decDouble *, decContext *, enum rounding);
  extern int32_t     decDoubleToInt32Exact(const decDouble *, decContext *, enum rounding);
  extern int32_t     decDoubleToPacked(const decDouble *, int32_t *, uint8_t *);
  extern char      * decDoubleToString(const decDouble *, char *);
  extern uint32_t    decDoubleToUInt32(const decDouble *, decContext *, enum rounding);
  extern uint32_t    decDoubleToUInt32Exact(const decDouble *, decContext *, enum rounding);
  extern decQuad   * decDoubleToWider(const decDouble *, decQuad *);
  extern decDouble * decDoubleZero(decDouble *);

  /* Computational (result is a decDouble) */
  extern decDouble * decDoubleAbs(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleAdd(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleAnd(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleDivide(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleDivideInteger(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleFMA(decDouble *, const decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleInvert(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleLogB(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleMax(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleMaxMag(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleMin(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleMinMag(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleMinus(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleMultiply(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleNextMinus(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleNextPlus(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleNextToward(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleOr(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoublePlus(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleQuantize(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleReduce(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleRemainder(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleRemainderNear(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleRotate(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleScaleB(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleShift(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleSubtract(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleToIntegralValue(decDouble *, const decDouble *, decContext *, enum rounding);
  extern decDouble * decDoubleToIntegralExact(decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleXor(decDouble *, const decDouble *, const decDouble *, decContext *);

  /* Comparisons */
  extern decDouble * decDoubleCompare(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleCompareSignal(decDouble *, const decDouble *, const decDouble *, decContext *);
  extern decDouble * decDoubleCompareTotal(decDouble *, const decDouble *, const decDouble *);
  extern decDouble * decDoubleCompareTotalMag(decDouble *, const decDouble *, const decDouble *);

  /* Copies */
  extern decDouble * decDoubleCanonical(decDouble *, const decDouble *);
  extern decDouble * decDoubleCopy(decDouble *, const decDouble *);
  extern decDouble * decDoubleCopyAbs(decDouble *, const decDouble *);
  extern decDouble * decDoubleCopyNegate(decDouble *, const decDouble *);
  extern decDouble * decDoubleCopySign(decDouble *, const decDouble *, const decDouble *);

  /* Non-computational */
  extern enum decClass decDoubleClass(const decDouble *);
  extern const char *  decDoubleClassString(const decDouble *);
  extern uint32_t      decDoubleDigits(const decDouble *);
  extern uint32_t      decDoubleIsCanonical(const decDouble *);
  extern uint32_t      decDoubleIsFinite(const decDouble *);
  extern uint32_t      decDoubleIsInfinite(const decDouble *);
  extern uint32_t      decDoubleIsInteger(const decDouble *);
  extern uint32_t      decDoubleIsLogical(const decDouble *);
  extern uint32_t      decDoubleIsNaN(const decDouble *);
  extern uint32_t      decDoubleIsNegative(const decDouble *);
  extern uint32_t      decDoubleIsNormal(const decDouble *);
  extern uint32_t      decDoubleIsPositive(const decDouble *);
  extern uint32_t      decDoubleIsSignaling(const decDouble *);
  extern uint32_t      decDoubleIsSignalling(const decDouble *);
  extern uint32_t      decDoubleIsSigned(const decDouble *);
  extern uint32_t      decDoubleIsSubnormal(const decDouble *);
  extern uint32_t      decDoubleIsZero(const decDouble *);
  extern uint32_t      decDoubleRadix(const decDouble *);
  extern uint32_t      decDoubleSameQuantum(const decDouble *, const decDouble *);
  extern const char *  decDoubleVersion(void);

  /* decNumber conversions; these are implemented as macros so as not  */
  /* to force a dependency on decimal64 and decNumber in decDouble.    */
  /* decDoubleFromNumber returns a decimal64 * to avoid warnings.      */
  #define decDoubleToNumber(dq, dn) decimal64ToNumber((decimal64 *)(dq), dn)
  #define decDoubleFromNumber(dq, dn, set) decimal64FromNumber((decimal64 *)(dq), dn, set)

#endif
