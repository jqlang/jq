#ifndef MAIN_WIN32_H
#define MAIN_WIN32_H

#ifdef WIN32
#include <stringapiset.h>
#include <wchar.h>

#define DEFINE_MAIN(ARGC, ARGV) \
  int umain(ARGC, ARGV); \
  int wmain(int argc, wchar_t* wargv[]) { \
    size_t arg_sz; \
    char **argv = alloca(argc * sizeof(wchar_t*)); \
    for (int i = 0; i < argc; i++) { \
      argv[i] = alloca((arg_sz = WideCharToMultiByte(CP_UTF8, \
                                                     0, \
                                                     wargv[i], \
                                                     -1, 0, 0, 0, 0))); \
      WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], arg_sz, 0, 0); \
    } \
    return umain(argc, argv); \
  } \
  int umain(ARGC, ARGV)
#else
#define DEFINE_MAIN(ARGC, ARGV) \
  int main(ARGC, ARGV)
#endif

#endif
