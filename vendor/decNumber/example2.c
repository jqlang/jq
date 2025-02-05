/* ------------------------------------------------------------------ */
/* Decimal Number Library Demonstration program                       */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2001.  All rights reserved.         */
/* ----------------------------------------------------------------+- */
/*                                                 right margin -->|  */

// example2.c -- calculate compound interest
// Arguments are investment, rate (%), and years

#define  DECNUMDIGITS 38           // work with up to 38 digits
#include "decNumber.h"             // base number library
#include <stdio.h>                 // for printf

int main(int argc, char *argv[]) {
  int need=3;
  if (argc<need+1) {               // not enough words
    printf("Please supply %d number(s).\n", need);
    return 1;
    }

  { // excerpt for User's Guide starts here--------------------------|
  decNumber one, mtwo, hundred;                   // constants
  decNumber start, rate, years;                   // parameters
  decNumber total;                                // result
  decContext set;                                 // working context
  char string[DECNUMDIGITS+14];                   // conversion buffer

  decContextDefault(&set, DEC_INIT_BASE);         // initialize
  set.traps=0;                                    // no traps
  set.digits=25;                                  // precision 25
  decNumberFromString(&one,       "1", &set);     // set constants
  decNumberFromString(&mtwo,     "-2", &set);
  decNumberFromString(&hundred, "100", &set);

  decNumberFromString(&start, argv[1], &set);     // parameter words
  decNumberFromString(&rate,  argv[2], &set);
  decNumberFromString(&years, argv[3], &set);

  decNumberDivide(&rate, &rate, &hundred, &set);  // rate=rate/100
  decNumberAdd(&rate, &rate, &one, &set);         // rate=rate+1
  decNumberPower(&rate, &rate, &years, &set);     // rate=rate^years
  decNumberMultiply(&total, &rate, &start, &set); // total=rate*start
  decNumberRescale(&total, &total, &mtwo, &set);  // two digits please

  decNumberToString(&total, string);
  printf("%s at %s%% for %s years => %s\n",
         argv[1], argv[2], argv[3], string);

  } //---------------------------------------------------------------|
  return 0;
  } // main
