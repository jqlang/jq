/* ------------------------------------------------------------------ */
/* Decimal Number Library Demonstration program                       */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2001, 2007.  All rights reserved.   */
/* ----------------------------------------------------------------+- */
/*                                                 right margin -->|  */

// example8.c -- using decQuad with the decNumber module

// compile: example8.c decContext.c decQuad.c
//     and: decNumber.c decimal128.c decimal64.c

#include "decQuad.h"               // decQuad library
#include "decimal128.h"            // interface to decNumber
#include <stdio.h>                 // for printf

int main(int argc, char *argv[]) {
  decQuad a;                       // working decQuad
  decNumber numa, numb;            // working decNumbers
  decContext set;                  // working context
  char string[DECQUAD_String];     // number->string buffer

  if (argc<3) {                    // not enough words
    printf("Please supply two numbers for power(2*a, b).\n");
    return 1;
    }
  decContextDefault(&set, DEC_INIT_DECQUAD); // initialize

  decQuadFromString(&a, argv[1], &set);      // get a
  decQuadAdd(&a, &a, &a, &set);              // double a
  decQuadToNumber(&a, &numa);                // convert to decNumber
  decNumberFromString(&numb, argv[2], &set);
  decNumberPower(&numa, &numa, &numb, &set); // numa=numa**numb
  decQuadFromNumber(&a, &numa, &set);        // back via a Quad
  decQuadToString(&a, string);               // ..

  printf("power(2*%s, %s) => %s\n", argv[1], argv[2], string);
  return 0;
  } // main
