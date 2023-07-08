#include "tests.h"

#define CHECK_NORM(NRM, norm, src) {                                 \
    unsigned char *src_norm = (unsigned char*) utf8proc_ ## NRM((utf8proc_uint8_t*) src);      \
    check(!strcmp((char *) norm, (char *) src_norm),                                  \
          "normalization failed for %s -> %s", src, norm);          \
    free(src_norm);                                                 \
}

int main(int argc, char **argv)
{
     unsigned char buf[8192];
     FILE *f = argc > 1 ? fopen(argv[1], "r") : NULL;
     unsigned char source[1024], NFC[1024], NFD[1024], NFKC[1024], NFKD[1024];

     check(f != NULL, "error opening NormalizationTest.txt");
     while (simple_getline(buf, f) > 0) {
          size_t offset;
          lineno += 1;

          if (buf[0] == '@') {
               printf("line %zd: %s", lineno, buf + 1);
               continue;
          }
          else if (lineno % 1000 == 0)
               printf("checking line %zd...\n", lineno);

          if (buf[0] == '#') continue;

          offset = encode(source, buf);
          offset += encode(NFC, buf + offset);
          offset += encode(NFD, buf + offset);
          offset += encode(NFKC, buf + offset);
          offset += encode(NFKD, buf + offset);

          CHECK_NORM(NFC, NFC, source);
          CHECK_NORM(NFC, NFC, NFC);
          CHECK_NORM(NFC, NFC, NFD);
          CHECK_NORM(NFC, NFKC, NFKC);
          CHECK_NORM(NFC, NFKC, NFKD);

          CHECK_NORM(NFD, NFD, source);
          CHECK_NORM(NFD, NFD, NFC);
          CHECK_NORM(NFD, NFD, NFD);
          CHECK_NORM(NFD, NFKD, NFKC);
          CHECK_NORM(NFD, NFKD, NFKD);

          CHECK_NORM(NFKC, NFKC, source);
          CHECK_NORM(NFKC, NFKC, NFC);
          CHECK_NORM(NFKC, NFKC, NFD);
          CHECK_NORM(NFKC, NFKC, NFKC);
          CHECK_NORM(NFKC, NFKC, NFKD);

          CHECK_NORM(NFKD, NFKD, source);
          CHECK_NORM(NFKD, NFKD, NFC);
          CHECK_NORM(NFKD, NFKD, NFD);
          CHECK_NORM(NFKD, NFKD, NFKC);
          CHECK_NORM(NFKD, NFKD, NFKD);
     }
     fclose(f);
     printf("Passed tests after %zd lines!\n", lineno);
     return 0;
}
