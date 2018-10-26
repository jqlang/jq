/* ------------------------------------------------------------------ */
/* Decimal Number Library Demonstration program                       */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2001.  All rights reserved.         */
/* ----------------------------------------------------------------+- */
/*                                                 right margin -->|  */

// example4.c -- add two numbers, active error handling
// Arguments are two numbers

#define  DECNUMDIGITS 38           // work with up to 38 digits
#include "decNumber.h"             // base number library
#include <stdio.h>                 // for printf

// [snip...
#include <signal.h>                // signal handling
#include <setjmp.h>                // setjmp/longjmp

jmp_buf preserve;                  // stack snapshot

void signalHandler(int);           // prototype for GCC
void signalHandler(int sig) {
  signal(SIGFPE, signalHandler);   // re-enable
  longjmp(preserve, sig);          // branch to preserved point
  }
// ...snip]
int main(int argc, char *argv[]) {
  decNumber a, b;                  // working numbers
  decContext set;                  // working context
  char string[DECNUMDIGITS+14];    // conversion buffer
  int value;                       // work variable

  if (argc<3) {                    // not enough words
    printf("Please supply two numbers to add.\n");
    return 1;
    }
  decContextDefault(&set, DEC_INIT_BASE);    // initialize

// [snip...
  signal(SIGFPE, signalHandler);   // set up signal handler
  value=setjmp(preserve);          // preserve and test environment
  if (value) {                     // (non-0 after longjmp)
    set.status &= DEC_Errors;      // keep only errors
    printf("Signal trapped [%s].\n", decContextStatusToString(&set));
    return 1;
    }
// ...snip]

// [change from Example 1, here]
  // leave traps enabled
  set.digits=DECNUMDIGITS;         // set precision

  decNumberFromString(&a, argv[1], &set);
  decNumberFromString(&b, argv[2], &set);

  decNumberAdd(&a, &a, &b, &set);            // A=A+B
  decNumberToString(&a, string);

  printf("%s + %s => %s\n", argv[1], argv[2], string);
  return 0;
  } // main
