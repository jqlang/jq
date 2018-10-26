/* ------------------------------------------------------------------ */
/* Packed Decimal conversion module                                   */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2002.  All rights reserved.   */
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
/* This module comprises the routines for Packed Decimal format       */
/* numbers.  Conversions are supplied to and from decNumber, which in */
/* turn supports:                                                     */
/*   conversions to and from string                                   */
/*   arithmetic routines                                              */
/*   utilities.                                                       */
/* Conversions from decNumber to and from densely packed decimal      */
/* formats are provided by the decimal32 through decimal128 modules.  */
/* ------------------------------------------------------------------ */

#include <string.h>           // for NULL
#include "decNumber.h"        // base number library
#include "decPacked.h"        // packed decimal
#include "decNumberLocal.h"   // decNumber local types, etc.

/* ------------------------------------------------------------------ */
/* decPackedFromNumber -- convert decNumber to BCD Packed Decimal     */
/*                                                                    */
/*   bcd    is the BCD bytes                                          */
/*   length is the length of the BCD array                            */
/*   scale  is the scale result                                       */
/*   dn     is the decNumber                                          */
/*   returns bcd, or NULL if error                                    */
/*                                                                    */
/* The number is converted to a BCD packed decimal byte array,        */
/* right aligned in the bcd array, whose length is indicated by the   */
/* second parameter.  The final 4-bit nibble in the array will be a   */
/* sign nibble, C (1100) for + and D (1101) for -.  Unused bytes and  */
/* nibbles to the left of the number are set to 0.                    */
/*                                                                    */
/* scale is set to the scale of the number (this is the exponent,     */
/* negated).  To force the number to a specified scale, first use the */
/* decNumberRescale routine, which will round and change the exponent */
/* as necessary.                                                      */
/*                                                                    */
/* If there is an error (that is, the decNumber has too many digits   */
/* to fit in length bytes, or it is a NaN or Infinity), NULL is       */
/* returned and the bcd and scale results are unchanged.  Otherwise   */
/* bcd is returned.                                                   */
/* ------------------------------------------------------------------ */
uByte * decPackedFromNumber(uByte *bcd, Int length, Int *scale,
                            const decNumber *dn) {
  const Unit *up=dn->lsu;     // Unit array pointer
  uByte obyte, *out;          // current output byte, and where it goes
  Int indigs=dn->digits;      // digits processed
  uInt cut=DECDPUN;           // downcounter per Unit
  uInt u=*up;                 // work
  uInt nib;                   // ..
  #if DECDPUN<=4
  uInt temp;                  // ..
  #endif

  if (dn->digits>length*2-1                  // too long ..
   ||(dn->bits & DECSPECIAL)) return NULL;   // .. or special -- hopeless

  if (dn->bits&DECNEG) obyte=DECPMINUS;      // set the sign ..
   else                obyte=DECPPLUS;
  *scale=-dn->exponent;                      // .. and scale

  // loop from lowest (rightmost) byte
  out=bcd+length-1;                          // -> final byte
  for (; out>=bcd; out--) {
    if (indigs>0) {
      if (cut==0) {
        up++;
        u=*up;
        cut=DECDPUN;
        }
      #if DECDPUN<=4
        temp=(u*6554)>>16;         // fast /10
        nib=u-X10(temp);
        u=temp;
      #else
        nib=u%10;                  // cannot use *6554 trick :-(
        u=u/10;
      #endif
      obyte|=(nib<<4);
      indigs--;
      cut--;
      }
    *out=obyte;
    obyte=0;                       // assume 0
    if (indigs>0) {
      if (cut==0) {
        up++;
        u=*up;
        cut=DECDPUN;
        }
      #if DECDPUN<=4
        temp=(u*6554)>>16;         // as above
        obyte=(uByte)(u-X10(temp));
        u=temp;
      #else
        obyte=(uByte)(u%10);
        u=u/10;
      #endif
      indigs--;
      cut--;
      }
    } // loop

  return bcd;
  } // decPackedFromNumber

/* ------------------------------------------------------------------ */
/* decPackedToNumber -- convert BCD Packed Decimal to a decNumber     */
/*                                                                    */
/*   bcd    is the BCD bytes                                          */
/*   length is the length of the BCD array                            */
/*   scale  is the scale associated with the BCD integer              */
/*   dn     is the decNumber [with space for length*2 digits]         */
/*   returns dn, or NULL if error                                     */
/*                                                                    */
/* The BCD packed decimal byte array, together with an associated     */
/* scale, is converted to a decNumber.  The BCD array is assumed full */
/* of digits, and must be ended by a 4-bit sign nibble in the least   */
/* significant four bits of the final byte.                           */
/*                                                                    */
/* The scale is used (negated) as the exponent of the decNumber.      */
/* Note that zeros may have a sign and/or a scale.                    */
/*                                                                    */
/* The decNumber structure is assumed to have sufficient space to     */
/* hold the converted number (that is, up to length*2-1 digits), so   */
/* no error is possible unless the adjusted exponent is out of range, */
/* no sign nibble was found, or a sign nibble was found before the    */
/* final nibble.  In these error cases, NULL is returned and the      */
/* decNumber will be 0.                                               */
/* ------------------------------------------------------------------ */
decNumber * decPackedToNumber(const uByte *bcd, Int length,
                              const Int *scale, decNumber *dn) {
  const uByte *last=bcd+length-1;  // -> last byte
  const uByte *first;              // -> first non-zero byte
  uInt  nib;                       // work nibble
  Unit  *up=dn->lsu;               // output pointer
  Int   digits;                    // digits count
  Int   cut=0;                     // phase of output

  decNumberZero(dn);               // default result
  last=&bcd[length-1];
  nib=*last & 0x0f;                // get the sign
  if (nib==DECPMINUS || nib==DECPMINUSALT) dn->bits=DECNEG;
   else if (nib<=9) return NULL;   // not a sign nibble

  // skip leading zero bytes [final byte is always non-zero, due to sign]
  for (first=bcd; *first==0;) first++;
  digits=(last-first)*2+1;              // calculate digits ..
  if ((*first & 0xf0)==0) digits--;     // adjust for leading zero nibble
  if (digits!=0) dn->digits=digits;     // count of actual digits [if 0,
                                        // leave as 1]

  // check the adjusted exponent; note that scale could be unbounded
  dn->exponent=-*scale;                 // set the exponent
  if (*scale>=0) {                      // usual case
    if ((dn->digits-*scale-1)<-DECNUMMAXE) {      // underflow
      decNumberZero(dn);
      return NULL;}
    }
   else { // -ve scale; +ve exponent
    // need to be careful to avoid wrap, here, also BADINT case
    if ((*scale<-DECNUMMAXE)            // overflow even without digits
         || ((dn->digits-*scale-1)>DECNUMMAXE)) { // overflow
      decNumberZero(dn);
      return NULL;}
    }
  if (digits==0) return dn;             // result was zero

  // copy the digits to the number's units, starting at the lsu
  // [unrolled]
  for (;;) {                            // forever
    // left nibble first
    nib=(unsigned)(*last & 0xf0)>>4;
    // got a digit, in nib
    if (nib>9) {decNumberZero(dn); return NULL;}

    if (cut==0) *up=(Unit)nib;
     else *up=(Unit)(*up+nib*DECPOWERS[cut]);
    digits--;
    if (digits==0) break;               // got them all
    cut++;
    if (cut==DECDPUN) {
      up++;
      cut=0;
      }
    last--;                             // ready for next
    nib=*last & 0x0f;                   // get right nibble
    if (nib>9) {decNumberZero(dn); return NULL;}

    // got a digit, in nib
    if (cut==0) *up=(Unit)nib;
     else *up=(Unit)(*up+nib*DECPOWERS[cut]);
    digits--;
    if (digits==0) break;               // got them all
    cut++;
    if (cut==DECDPUN) {
      up++;
      cut=0;
      }
    } // forever

  return dn;
  } // decPackedToNumber

