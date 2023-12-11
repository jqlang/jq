#ifndef MATH_MACOS_H
#define MATH_MACOS_H

#include <math.h>

#ifdef __APPLE__
// macOS has a bunch of libm deprecation warnings, so let's clean those up
#if defined(HAVE_TGAMMA) && !defined(HAVE_GAMMA)
#define HAVE_GAMMA
#define gamma tgamma
#endif
#if defined(HAVE___EXP10) && !defined(HAVE_EXP10)
#define HAVE_EXP10
#define exp10 __exp10
#endif
#if defined(HAVE_REMAINDER) && !defined(HAVE_DREM)
#define HAVE_DREM
#define drem remainder
#endif

#ifndef HAVE_SIGNIFICAND
// We replace significand with our own, since there's not a rename-replacement
#ifdef HAVE_FREXP
static double __jq_significand(double x) {
  int z;
  return 2*frexp(x, &z);
}
#define HAVE_SIGNIFICAND
#define significand __jq_significand
#elif defined(HAVE_SCALBN) && defined(HAVE_ILOGB)
static double __jq_significand(double x) {
  return scalbn(x, -ilogb(x));
}
#define HAVE_SIGNIFICAND
#define significand __jq_significand
#endif
#endif

#endif // ifdef __APPLE__

#endif
