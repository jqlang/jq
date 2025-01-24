/* ------------------------------------------------------------------ */
/* decSingle.c -- decSingle operations module                         */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2008.  All rights reserved.   */
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
/* This module comprises decSingle operations (including conversions) */
/* ------------------------------------------------------------------ */

#include "decContext.h"       // public includes
#include "decSingle.h"        // public includes

/* Constant mappings for shared code */
#define DECPMAX     DECSINGLE_Pmax
#define DECEMIN     DECSINGLE_Emin
#define DECEMAX     DECSINGLE_Emax
#define DECEMAXD    DECSINGLE_EmaxD
#define DECBYTES    DECSINGLE_Bytes
#define DECSTRING   DECSINGLE_String
#define DECECONL    DECSINGLE_EconL
#define DECBIAS     DECSINGLE_Bias
#define DECLETS     DECSINGLE_Declets
#define DECQTINY    (-DECSINGLE_Bias)
// parameters of next-wider format
#define DECWBYTES   DECDOUBLE_Bytes
#define DECWPMAX    DECDOUBLE_Pmax
#define DECWECONL   DECDOUBLE_EconL
#define DECWBIAS    DECDOUBLE_Bias

/* Type and function mappings for shared code */
#define decFloat                   decSingle      // Type name
#define decFloatWider              decDouble      // Type name

// Utility (binary results, extractors, etc.)
#define decFloatFromBCD            decSingleFromBCD
#define decFloatFromPacked         decSingleFromPacked
#define decFloatFromPackedChecked  decSingleFromPackedChecked
#define decFloatFromString         decSingleFromString
#define decFloatFromWider          decSingleFromWider
#define decFloatGetCoefficient     decSingleGetCoefficient
#define decFloatGetExponent        decSingleGetExponent
#define decFloatSetCoefficient     decSingleSetCoefficient
#define decFloatSetExponent        decSingleSetExponent
#define decFloatShow               decSingleShow
#define decFloatToBCD              decSingleToBCD
#define decFloatToEngString        decSingleToEngString
#define decFloatToPacked           decSingleToPacked
#define decFloatToString           decSingleToString
#define decFloatToWider            decSingleToWider
#define decFloatZero               decSingleZero

// Non-computational
#define decFloatRadix              decSingleRadix
#define decFloatVersion            decSingleVersion

#include "decNumberLocal.h"   // local includes (need DECPMAX)
#include "decCommon.c"        // non-basic decFloat routines
// [Do not include decBasic.c for decimal32]

