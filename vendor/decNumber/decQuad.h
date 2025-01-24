/* ------------------------------------------------------------------ */
/* decQuad.h -- Decimal 128-bit format module header                  */
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
/* This include file is always included by decSingle and decDouble,   */
/* and therefore also holds useful constants used by all three.       */

#if !defined(DECQUAD)
  #define DECQUAD

  #define DECQUADNAME         "decimalQuad"           /* Short name   */
  #define DECQUADTITLE        "Decimal 128-bit datum" /* Verbose name */
  #define DECQUADAUTHOR       "Mike Cowlishaw"        /* Who to blame */

  /* parameters for decQuads */
  #define DECQUAD_Bytes    16      /* length                          */
  #define DECQUAD_Pmax     34      /* maximum precision (digits)      */
  #define DECQUAD_Emin  -6143      /* minimum adjusted exponent       */
  #define DECQUAD_Emax   6144      /* maximum adjusted exponent       */
  #define DECQUAD_EmaxD     4      /* maximum exponent digits         */
  #define DECQUAD_Bias   6176      /* bias for the exponent           */
  #define DECQUAD_String   43      /* maximum string length, +1       */
  #define DECQUAD_EconL    12      /* exponent continuation length    */
  #define DECQUAD_Declets  11      /* count of declets                */
  /* highest biased exponent (Elimit-1) */
  #define DECQUAD_Ehigh (DECQUAD_Emax + DECQUAD_Bias - (DECQUAD_Pmax-1))

  /* Required include                                                 */
  #include "decContext.h"

  /* The decQuad decimal 128-bit type, accessible by all sizes */
  typedef union {
    uint8_t   bytes[DECQUAD_Bytes];     /* fields: 1, 5, 12, 110 bits */
    uint16_t shorts[DECQUAD_Bytes/2];
    uint32_t  words[DECQUAD_Bytes/4];
    #if DECUSE64
    uint64_t  longs[DECQUAD_Bytes/8];
    #endif
    } decQuad;

  /* ---------------------------------------------------------------- */
  /* Shared constants                                                 */
  /* ---------------------------------------------------------------- */

  /* sign and special values [top 32-bits; last two bits are don't-care
     for Infinity on input, last bit don't-care for NaNs] */
  #define DECFLOAT_Sign  0x80000000     /* 1 00000 00 Sign */
  #define DECFLOAT_NaN   0x7c000000     /* 0 11111 00 NaN generic */
  #define DECFLOAT_qNaN  0x7c000000     /* 0 11111 00 qNaN */
  #define DECFLOAT_sNaN  0x7e000000     /* 0 11111 10 sNaN */
  #define DECFLOAT_Inf   0x78000000     /* 0 11110 00 Infinity */
  #define DECFLOAT_MinSp 0x78000000     /* minimum special value */
                                        /* [specials are all >=MinSp] */
  /* Sign nibble constants                                            */
  #if !defined(DECPPLUSALT)
    #define DECPPLUSALT  0x0A /* alternate plus  nibble               */
    #define DECPMINUSALT 0x0B /* alternate minus nibble               */
    #define DECPPLUS     0x0C /* preferred plus  nibble               */
    #define DECPMINUS    0x0D /* preferred minus nibble               */
    #define DECPPLUSALT2 0x0E /* alternate plus  nibble               */
    #define DECPUNSIGNED 0x0F /* alternate plus  nibble (unsigned)    */
  #endif

  /* ---------------------------------------------------------------- */
  /* Routines -- implemented as decFloat routines in common files     */
  /* ---------------------------------------------------------------- */

  /* Utilities and conversions, extractors, etc.) */
  extern decQuad * decQuadFromBCD(decQuad *, int32_t, const uint8_t *, int32_t);
  extern decQuad * decQuadFromInt32(decQuad *, int32_t);
  extern decQuad * decQuadFromPacked(decQuad *, int32_t, const uint8_t *);
  extern decQuad * decQuadFromPackedChecked(decQuad *, int32_t, const uint8_t *);
  extern decQuad * decQuadFromString(decQuad *, const char *, decContext *);
  extern decQuad * decQuadFromUInt32(decQuad *, uint32_t);
  extern int32_t   decQuadGetCoefficient(const decQuad *, uint8_t *);
  extern int32_t   decQuadGetExponent(const decQuad *);
  extern decQuad * decQuadSetCoefficient(decQuad *, const uint8_t *, int32_t);
  extern decQuad * decQuadSetExponent(decQuad *, decContext *, int32_t);
  extern void      decQuadShow(const decQuad *, const char *);
  extern int32_t   decQuadToBCD(const decQuad *, int32_t *, uint8_t *);
  extern char    * decQuadToEngString(const decQuad *, char *);
  extern int32_t   decQuadToInt32(const decQuad *, decContext *, enum rounding);
  extern int32_t   decQuadToInt32Exact(const decQuad *, decContext *, enum rounding);
  extern int32_t   decQuadToPacked(const decQuad *, int32_t *, uint8_t *);
  extern char    * decQuadToString(const decQuad *, char *);
  extern uint32_t  decQuadToUInt32(const decQuad *, decContext *, enum rounding);
  extern uint32_t  decQuadToUInt32Exact(const decQuad *, decContext *, enum rounding);
  extern decQuad * decQuadZero(decQuad *);

  /* Computational (result is a decQuad) */
  extern decQuad * decQuadAbs(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadAdd(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadAnd(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadDivide(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadDivideInteger(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadFMA(decQuad *, const decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadInvert(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadLogB(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadMax(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadMaxMag(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadMin(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadMinMag(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadMinus(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadMultiply(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadNextMinus(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadNextPlus(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadNextToward(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadOr(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadPlus(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadQuantize(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadReduce(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadRemainder(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadRemainderNear(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadRotate(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadScaleB(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadShift(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadSubtract(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadToIntegralValue(decQuad *, const decQuad *, decContext *, enum rounding);
  extern decQuad * decQuadToIntegralExact(decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadXor(decQuad *, const decQuad *, const decQuad *, decContext *);

  /* Comparisons */
  extern decQuad * decQuadCompare(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadCompareSignal(decQuad *, const decQuad *, const decQuad *, decContext *);
  extern decQuad * decQuadCompareTotal(decQuad *, const decQuad *, const decQuad *);
  extern decQuad * decQuadCompareTotalMag(decQuad *, const decQuad *, const decQuad *);

  /* Copies */
  extern decQuad * decQuadCanonical(decQuad *, const decQuad *);
  extern decQuad * decQuadCopy(decQuad *, const decQuad *);
  extern decQuad * decQuadCopyAbs(decQuad *, const decQuad *);
  extern decQuad * decQuadCopyNegate(decQuad *, const decQuad *);
  extern decQuad * decQuadCopySign(decQuad *, const decQuad *, const decQuad *);

  /* Non-computational */
  extern enum decClass decQuadClass(const decQuad *);
  extern const char *  decQuadClassString(const decQuad *);
  extern uint32_t      decQuadDigits(const decQuad *);
  extern uint32_t      decQuadIsCanonical(const decQuad *);
  extern uint32_t      decQuadIsFinite(const decQuad *);
  extern uint32_t      decQuadIsInteger(const decQuad *);
  extern uint32_t      decQuadIsLogical(const decQuad *);
  extern uint32_t      decQuadIsInfinite(const decQuad *);
  extern uint32_t      decQuadIsNaN(const decQuad *);
  extern uint32_t      decQuadIsNegative(const decQuad *);
  extern uint32_t      decQuadIsNormal(const decQuad *);
  extern uint32_t      decQuadIsPositive(const decQuad *);
  extern uint32_t      decQuadIsSignaling(const decQuad *);
  extern uint32_t      decQuadIsSignalling(const decQuad *);
  extern uint32_t      decQuadIsSigned(const decQuad *);
  extern uint32_t      decQuadIsSubnormal(const decQuad *);
  extern uint32_t      decQuadIsZero(const decQuad *);
  extern uint32_t      decQuadRadix(const decQuad *);
  extern uint32_t      decQuadSameQuantum(const decQuad *, const decQuad *);
  extern const char *  decQuadVersion(void);

  /* decNumber conversions; these are implemented as macros so as not  */
  /* to force a dependency on decimal128 and decNumber in decQuad.     */
  /* decQuadFromNumber returns a decimal128 * to avoid warnings.       */
  #define decQuadToNumber(dq, dn) decimal128ToNumber((decimal128 *)(dq), dn)
  #define decQuadFromNumber(dq, dn, set) decimal128FromNumber((decimal128 *)(dq), dn, set)

#endif
