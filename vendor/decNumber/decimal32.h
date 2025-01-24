/* ------------------------------------------------------------------ */
/* Decimal 32-bit format module header                                */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2006.  All rights reserved.   */
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

#if !defined(DECIMAL32)
  #define DECIMAL32
  #define DEC32NAME     "decimal32"                   /* Short name   */
  #define DEC32FULLNAME "Decimal 32-bit Number"       /* Verbose name */
  #define DEC32AUTHOR   "Mike Cowlishaw"              /* Who to blame */

  /* parameters for decimal32s */
  #define DECIMAL32_Bytes  4            /* length                     */
  #define DECIMAL32_Pmax   7            /* maximum precision (digits) */
  #define DECIMAL32_Emax   96           /* maximum adjusted exponent  */
  #define DECIMAL32_Emin  -95           /* minimum adjusted exponent  */
  #define DECIMAL32_Bias   101          /* bias for the exponent      */
  #define DECIMAL32_String 15           /* maximum string length, +1  */
  #define DECIMAL32_EconL  6            /* exp. continuation length   */
  /* highest biased exponent (Elimit-1)                               */
  #define DECIMAL32_Ehigh  (DECIMAL32_Emax+DECIMAL32_Bias-DECIMAL32_Pmax+1)

  /* check enough digits, if pre-defined                              */
  #if defined(DECNUMDIGITS)
    #if (DECNUMDIGITS<DECIMAL32_Pmax)
      #error decimal32.h needs pre-defined DECNUMDIGITS>=7 for safe use
    #endif
  #endif

  #ifndef DECNUMDIGITS
    #define DECNUMDIGITS DECIMAL32_Pmax /* size if not already defined*/
  #endif
  #ifndef DECNUMBER
    #include "decNumber.h"              /* context and number library */
  #endif

  /* Decimal 32-bit type, accessible by bytes */
  typedef struct {
    uint8_t bytes[DECIMAL32_Bytes];     /* decimal32: 1, 5, 6, 20 bits*/
    } decimal32;

  /* special values [top byte excluding sign bit; last two bits are   */
  /* don't-care for Infinity on input, last bit don't-care for NaN]   */
  #if !defined(DECIMAL_NaN)
    #define DECIMAL_NaN     0x7c        /* 0 11111 00 NaN             */
    #define DECIMAL_sNaN    0x7e        /* 0 11111 10 sNaN            */
    #define DECIMAL_Inf     0x78        /* 0 11110 00 Infinity        */
  #endif

  /* ---------------------------------------------------------------- */
  /* Routines                                                         */
  /* ---------------------------------------------------------------- */
  /* String conversions                                               */
  decimal32 * decimal32FromString(decimal32 *, const char *, decContext *);
  char * decimal32ToString(const decimal32 *, char *);
  char * decimal32ToEngString(const decimal32 *, char *);

  /* decNumber conversions                                            */
  decimal32 * decimal32FromNumber(decimal32 *, const decNumber *,
                                  decContext *);
  decNumber * decimal32ToNumber(const decimal32 *, decNumber *);

  /* Format-dependent utilities                                       */
  uint32_t    decimal32IsCanonical(const decimal32 *);
  decimal32 * decimal32Canonical(decimal32 *, const decimal32 *);

#endif
