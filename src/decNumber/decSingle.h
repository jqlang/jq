/* ------------------------------------------------------------------ */
/* decSingle.h -- Decimal 32-bit format module header                 */
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

#if !defined(DECSINGLE)
  #define DECSINGLE

  #define DECSINGLENAME       "decSingle"             /* Short name   */
  #define DECSINGLETITLE      "Decimal 32-bit datum"  /* Verbose name */
  #define DECSINGLEAUTHOR     "Mike Cowlishaw"        /* Who to blame */

  /* parameters for decSingles */
  #define DECSINGLE_Bytes    4     /* length                          */
  #define DECSINGLE_Pmax     7     /* maximum precision (digits)      */
  #define DECSINGLE_Emin   -95     /* minimum adjusted exponent       */
  #define DECSINGLE_Emax    96     /* maximum adjusted exponent       */
  #define DECSINGLE_EmaxD    3     /* maximum exponent digits         */
  #define DECSINGLE_Bias   101     /* bias for the exponent           */
  #define DECSINGLE_String  16     /* maximum string length, +1       */
  #define DECSINGLE_EconL    6     /* exponent continuation length    */
  #define DECSINGLE_Declets  2     /* count of declets                */
  /* highest biased exponent (Elimit-1) */
  #define DECSINGLE_Ehigh (DECSINGLE_Emax + DECSINGLE_Bias - (DECSINGLE_Pmax-1))

  /* Required includes                                                */
  #include "decContext.h"
  #include "decQuad.h"
  #include "decDouble.h"

  /* The decSingle decimal 32-bit type, accessible by all sizes */
  typedef union {
    uint8_t   bytes[DECSINGLE_Bytes];   /* fields: 1, 5, 6, 20 bits */
    uint16_t shorts[DECSINGLE_Bytes/2];
    uint32_t  words[DECSINGLE_Bytes/4];
    } decSingle;

  /* ---------------------------------------------------------------- */
  /* Routines -- implemented as decFloat routines in common files     */
  /* ---------------------------------------------------------------- */

  /* Utilities (binary argument(s) or result, extractors, etc.) */
  extern decSingle * decSingleFromBCD(decSingle *, int32_t, const uint8_t *, int32_t);
  extern decSingle * decSingleFromPacked(decSingle *, int32_t, const uint8_t *);
  extern decSingle * decSingleFromPackedChecked(decSingle *, int32_t, const uint8_t *);
  extern decSingle * decSingleFromString(decSingle *, const char *, decContext *);
  extern decSingle * decSingleFromWider(decSingle *, const decDouble *, decContext *);
  extern int32_t     decSingleGetCoefficient(const decSingle *, uint8_t *);
  extern int32_t     decSingleGetExponent(const decSingle *);
  extern decSingle * decSingleSetCoefficient(decSingle *, const uint8_t *, int32_t);
  extern decSingle * decSingleSetExponent(decSingle *, decContext *, int32_t);
  extern void        decSingleShow(const decSingle *, const char *);
  extern int32_t     decSingleToBCD(const decSingle *, int32_t *, uint8_t *);
  extern char      * decSingleToEngString(const decSingle *, char *);
  extern int32_t     decSingleToPacked(const decSingle *, int32_t *, uint8_t *);
  extern char      * decSingleToString(const decSingle *, char *);
  extern decDouble * decSingleToWider(const decSingle *, decDouble *);
  extern decSingle * decSingleZero(decSingle *);

  /* (No Arithmetic routines for decSingle) */

  /* Non-computational */
  extern uint32_t     decSingleRadix(const decSingle *);
  extern const char * decSingleVersion(void);

  /* decNumber conversions; these are implemented as macros so as not  */
  /* to force a dependency on decimal32 and decNumber in decSingle.    */
  /* decSingleFromNumber returns a decimal32 * to avoid warnings.      */
  #define decSingleToNumber(dq, dn) decimal32ToNumber((decimal32 *)(dq), dn)
  #define decSingleFromNumber(dq, dn, set) decimal32FromNumber((decimal32 *)(dq), dn, set)

#endif
