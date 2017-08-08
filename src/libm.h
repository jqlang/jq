#ifdef HAVE_ACOS
LIBM_DD(acos)
#else
LIBM_DD_NO(acos)
#endif
#ifdef HAVE_ACOSH
LIBM_DD(acosh)
#else
LIBM_DD_NO(acosh)
#endif
#ifdef HAVE_ASIN
LIBM_DD(asin)
#else
LIBM_DD_NO(asin)
#endif
#ifdef HAVE_ASINH
LIBM_DD(asinh)
#else
LIBM_DD_NO(asinh)
#endif
#ifdef HAVE_ATAN
LIBM_DD(atan)
#else
LIBM_DD_NO(atan)
#endif
#ifdef HAVE_ATAN2
LIBM_DDD(atan2)
#else
LIBM_DDD_NO(atan2)
#endif
#ifdef HAVE_ATANH
LIBM_DD(atanh)
#else
LIBM_DD_NO(atanh)
#endif
#ifdef HAVE_CBRT
LIBM_DD(cbrt)
#else
LIBM_DD_NO(cbrt)
#endif
#ifdef HAVE_COS
LIBM_DD(cos)
#else
LIBM_DD_NO(cos)
#endif
#ifdef HAVE_COSH
LIBM_DD(cosh)
#else
LIBM_DD_NO(cosh)
#endif
#ifdef HAVE_EXP
LIBM_DD(exp)
#else
LIBM_DD_NO(exp)
#endif
#ifdef HAVE_EXP2
LIBM_DD(exp2)
#else
LIBM_DD_NO(exp2)
#endif
#ifdef HAVE_FLOOR
LIBM_DD(floor)
#else
LIBM_DD_NO(floor)
#endif
#ifdef HAVE_HYPOT
LIBM_DDD(hypot)
#else
LIBM_DDD_NO(hypot)
#endif
#ifdef HAVE_J0
LIBM_DD(j0)
#else
LIBM_DD_NO(j0)
#endif
#ifdef HAVE_J1
LIBM_DD(j1)
#else
LIBM_DD_NO(j1)
#endif
#ifdef HAVE_LOG
LIBM_DD(log)
#else
LIBM_DD_NO(log)
#endif
#ifdef HAVE_LOG10
LIBM_DD(log10)
#else
LIBM_DD_NO(log10)
#endif
#ifdef HAVE_LOG2
LIBM_DD(log2)
#else
LIBM_DD_NO(log2)
#endif
#ifdef HAVE_POW
LIBM_DDD(pow)
#else
LIBM_DDD_NO(pow)
#endif
#ifdef HAVE_REMAINDER
LIBM_DDD(remainder)
#else
LIBM_DDD_NO(remainder)
#endif
#ifdef HAVE_SIN
LIBM_DD(sin)
#else
LIBM_DD_NO(sin)
#endif
#ifdef HAVE_SINH
LIBM_DD(sinh)
#else
LIBM_DD_NO(sinh)
#endif
#ifdef HAVE_SQRT
LIBM_DD(sqrt)
#else
LIBM_DD_NO(sqrt)
#endif
#ifdef HAVE_TAN
LIBM_DD(tan)
#else
LIBM_DD_NO(tan)
#endif
#ifdef HAVE_TANH
LIBM_DD(tanh)
#else
LIBM_DD_NO(tanh)
#endif
#ifdef HAVE_TGAMMA
LIBM_DD(tgamma)
#else
LIBM_DD_NO(tgamma)
#endif
#ifdef HAVE_Y0
LIBM_DD(y0)
#else
LIBM_DD_NO(y0)
#endif
#ifdef HAVE_Y1
LIBM_DD(y1)
#else
LIBM_DD_NO(y1)
#endif
#ifdef HAVE_JN
LIBM_DDD(jn)
#endif
#ifdef HAVE_YN
LIBM_DDD(yn)
#endif
#ifdef HAVE_CEIL
LIBM_DD(ceil)
#else
LIBM_DD_NO(ceil)
#endif
#ifdef HAVE_COPYSIGN
LIBM_DDD(copysign)
#else
LIBM_DDD_NO(copysign)
#endif
#if defined(HAVE_DREM) && !defined(WIN32)
LIBM_DDD(drem)
#else
LIBM_DDD_NO(drem)
#endif
#ifdef HAVE_ERF
LIBM_DD(erf)
#else
LIBM_DD_NO(erf)
#endif
#ifdef HAVE_ERFC
LIBM_DD(erfc)
#else
LIBM_DD_NO(erfc)
#endif
#if defined(HAVE_EXP10) && !defined(WIN32)
LIBM_DD(exp10)
#else
LIBM_DD_NO(exp10)
#endif
#ifdef HAVE_EXPM1
LIBM_DD(expm1)
#else
LIBM_DD_NO(expm1)
#endif
#ifdef HAVE_FABS
LIBM_DD(fabs)
#else
LIBM_DD_NO(fabs)
#endif
#ifdef HAVE_FDIM
LIBM_DDD(fdim)
#else
LIBM_DDD_NO(fdim)
#endif
#ifdef HAVE_FMA
LIBM_DDDD(fma)
#else
LIBM_DDDD_NO(fma)
#endif
#ifdef HAVE_FMAX
LIBM_DDD(fmax)
#else
LIBM_DDD_NO(fmax)
#endif
#ifdef HAVE_FMIN
LIBM_DDD(fmin)
#else
LIBM_DDD_NO(fmin)
#endif
#ifdef HAVE_FMOD
LIBM_DDD(fmod)
#else
LIBM_DDD_NO(fmod)
#endif
#ifdef HAVE_GAMMA
LIBM_DD(gamma)
#else
LIBM_DD_NO(gamma)
#endif
#ifdef HAVE_LGAMMA
LIBM_DD(lgamma)
#else
LIBM_DD_NO(lgamma)
#endif
#ifdef HAVE_LOG1P
LIBM_DD(log1p)
#else
LIBM_DD_NO(log1p)
#endif
#ifdef HAVE_LOGB
LIBM_DD(logb)
#else
LIBM_DD_NO(logb)
#endif
#ifdef HAVE_NEARBYINT
LIBM_DD(nearbyint)
#else
LIBM_DD_NO(nearbyint)
#endif
#ifdef HAVE_NEXTAFTER
LIBM_DDD(nextafter)
#else
LIBM_DDD_NO(nextafter)
#endif
#ifdef HAVE_NEXTTOWARD
LIBM_DDD(nexttoward)
#else
LIBM_DDD_NO(nexttoward)
#endif
#if defined(HAVE_POW10) && !defined(WIN32)
LIBM_DD(pow10)
#else
LIBM_DD_NO(pow10)
#endif
#ifdef HAVE_RINT
LIBM_DD(rint)
#else
LIBM_DD_NO(rint)
#endif
#ifdef HAVE_ROUND
LIBM_DD(round)
#else
LIBM_DD_NO(round)
#endif
#ifdef HAVE_SCALB
LIBM_DDD(scalb)
#else
LIBM_DDD_NO(scalb)
#endif
#ifdef HAVE_SCALBLN
LIBM_DDD(scalbln)
#else
LIBM_DDD_NO(scalbln)
#endif
#if defined(HAVE_SIGNIFICAND) && !defined(WIN32)
LIBM_DD(significand)
#else
LIBM_DD_NO(significand)
#endif
#ifdef HAVE_TRUNC
LIBM_DD(trunc)
#else
LIBM_DD_NO(trunc)
#endif
#ifdef HAVE_LDEXP
LIBM_DDD(ldexp)
#else
LIBM_DDD_NO(ldexp)
#endif
