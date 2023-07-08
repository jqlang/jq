/* comparative benchmark of GNU libunistring */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libunistring */
#include <unistr.h>
#include <uninorm.h>

#include "util.h"

int main(int argc, char **argv)
{
	 int i;
	 uninorm_t nf = UNINORM_NFKC;

	 for (i = 1; i < argc; ++i) {
		  if (!strcmp(argv[i], "-nfkc")) {
			   nf = UNINORM_NFKC;
			   continue;
		  }
		  if (!strcmp(argv[i], "-nfkd")) {
			   nf = UNINORM_NFKD;
			   continue;
		  }
		  if (!strcmp(argv[i], "-nfc")) {
			   nf = UNINORM_NFC;
			   continue;
		  }
		  if (!strcmp(argv[i], "-nfd")) {
			   nf = UNINORM_NFD;
			   continue;
		  }
		  if (argv[i][0] == '-') {
			   fprintf(stderr, "unrecognized option: %s\n", argv[i]);
			   return EXIT_FAILURE;
		  }

		  size_t len;
		  uint8_t *src = readfile(argv[i], &len);
		  if (!src) {
			   fprintf(stderr, "error reading %s\n", argv[i]);
			   return EXIT_FAILURE;
		  }

		  size_t destlen;
		  uint8_t *dest;
		  mytime start = gettime();
		  for (int i = 0; i < 100; ++i) {
			   dest = u8_normalize(nf, src, len, NULL, &destlen);
			   if (!dest) return EXIT_FAILURE;
			   free(dest);
		  }
		  printf("%s: %g\n", argv[i], elapsed(gettime(), start) / 100);
		  free(src);
	 }

	 return EXIT_SUCCESS;
}
