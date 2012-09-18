#ifndef JV_DTOA_H
#define JV_DTOA_H
#define Kmax 7

struct Bigint;
struct dtoa_context {
  struct Bigint *freelist[Kmax+1];
  struct Bigint *p5s;
};

void jvp_dtoa_context_init(struct dtoa_context* ctx);
void jvp_dtoa_context_free(struct dtoa_context* ctx);

double jvp_strtod(struct dtoa_context* C, const char* s, char** se);


char* jvp_dtoa(struct dtoa_context* C, double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);
void jvp_freedtoa(struct dtoa_context* C, char *s);

#define JVP_DTOA_FMT_MAX_LEN 32
char* jvp_dtoa_fmt(struct dtoa_context* C, register char *b, double x);
#endif
