/* ------------------------------------------------------------------ */
/* decQuad.c -- decQuad operations module                             */
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
/* This module comprises decQuad operations (including conversions)   */
/* ------------------------------------------------------------------ */


/* Constant mappings for shared code */
#define DECPMAX     DECQUAD_Pmax
#define DECEMIN     DECQUAD_Emin
#define DECEMAX     DECQUAD_Emax
#define DECEMAXD    DECQUAD_EmaxD
#define DECBYTES    DECQUAD_Bytes
#define DECSTRING   DECQUAD_String
#define DECECONL    DECQUAD_EconL
#define DECBIAS     DECQUAD_Bias
#define DECLETS     DECQUAD_Declets
#define DECQTINY   (-DECQUAD_Bias)

/* Type and function mappings for shared code */
#define decFloat                   decQuad        // Type name

// Utilities and conversions (binary results, extractors, etc.)
#define decFloatFromBCD            decQuadFromBCD
#define decFloatFromInt32          decQuadFromInt32
#define decFloatFromPacked         decQuadFromPacked
#define decFloatFromPackedChecked  decQuadFromPackedChecked
#define decFloatFromString         decQuadFromString
#define decFloatFromUInt32         decQuadFromUInt32
#define decFloatFromWider          decQuadFromWider
#define decFloatGetCoefficient     decQuadGetCoefficient
#define decFloatGetExponent        decQuadGetExponent
#define decFloatSetCoefficient     decQuadSetCoefficient
#define decFloatSetExponent        decQuadSetExponent
#define decFloatShow               decQuadShow
#define decFloatToBCD              decQuadToBCD
#define decFloatToEngString        decQuadToEngString
#define decFloatToInt32            decQuadToInt32
#define decFloatToInt32Exact       decQuadToInt32Exact
#define decFloatToPacked           decQuadToPacked
#define decFloatToString           decQuadToString
#define decFloatToUInt32           decQuadToUInt32
#define decFloatToUInt32Exact      decQuadToUInt32Exact
#define decFloatToWider            decQuadToWider
#define decFloatZero               decQuadZero

// Computational (result is a decFloat)
#define decFloatAbs                decQuadAbs
#define decFloatAdd                decQuadAdd
#define decFloatAnd                decQuadAnd
#define decFloatDivide             decQuadDivide
#define decFloatDivideInteger      decQuadDivideInteger
#define decFloatFMA                decQuadFMA
#define decFloatInvert             decQuadInvert
#define decFloatLogB               decQuadLogB
#define decFloatMax                decQuadMax
#define decFloatMaxMag             decQuadMaxMag
#define decFloatMin                decQuadMin
#define decFloatMinMag             decQuadMinMag
#define decFloatMinus              decQuadMinus
#define decFloatMultiply           decQuadMultiply
#define decFloatNextMinus          decQuadNextMinus
#define decFloatNextPlus           decQuadNextPlus
#define decFloatNextToward         decQuadNextToward
#define decFloatOr                 decQuadOr
#define decFloatPlus               decQuadPlus
#define decFloatQuantize           decQuadQuantize
#define decFloatReduce             decQuadReduce
#define decFloatRemainder          decQuadRemainder
#define decFloatRemainderNear      decQuadRemainderNear
#define decFloatRotate             decQuadRotate
#define decFloatScaleB             decQuadScaleB
#define decFloatShift              decQuadShift
#define decFloatSubtract           decQuadSubtract
#define decFloatToIntegralValue    decQuadToIntegralValue
#define decFloatToIntegralExact    decQuadToIntegralExact
#define decFloatXor                decQuadXor

// Comparisons
#define decFloatCompare            decQuadCompare
#define decFloatCompareSignal      decQuadCompareSignal
#define decFloatCompareTotal       decQuadCompareTotal
#define decFloatCompareTotalMag    decQuadCompareTotalMag

// Copies
#define decFloatCanonical          decQuadCanonical
#define decFloatCopy               decQuadCopy
#define decFloatCopyAbs            decQuadCopyAbs
#define decFloatCopyNegate         decQuadCopyNegate
#define decFloatCopySign           decQuadCopySign

// Non-computational
#define decFloatClass              decQuadClass
#define decFloatClassString        decQuadClassString
#define decFloatDigits             decQuadDigits
#define decFloatIsCanonical        decQuadIsCanonical
#define decFloatIsFinite           decQuadIsFinite
#define decFloatIsInfinite         decQuadIsInfinite
#define decFloatIsInteger          decQuadIsInteger
#define decFloatIsLogical          decQuadIsLogical
#define decFloatIsNaN              decQuadIsNaN
#define decFloatIsNegative         decQuadIsNegative
#define decFloatIsNormal           decQuadIsNormal
#define decFloatIsPositive         decQuadIsPositive
#define decFloatIsSignaling        decQuadIsSignaling
#define decFloatIsSignalling       decQuadIsSignalling
#define decFloatIsSigned           decQuadIsSigned
#define decFloatIsSubnormal        decQuadIsSubnormal
#define decFloatIsZero             decQuadIsZero
#define decFloatRadix              decQuadRadix
#define decFloatSameQuantum        decQuadSameQuantum
#define decFloatVersion            decQuadVersion

/* And now the code itself */
#include "decContext.h"       // public includes
#include "decQuad.h"          // ..
#include "decNumberLocal.h"   // local includes (need DECPMAX)
#include "decCommon.c"        // non-arithmetic decFloat routines
#include "decBasic.c"         // basic formats routines

