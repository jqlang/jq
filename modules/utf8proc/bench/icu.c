#include <stdio.h>
#include <stdlib.h>

/* ICU4C */
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/unorm2.h>

#include "util.h"

int main(int argc, char **argv)
{
	 int i;

	 UErrorCode err;
	 UConverter *uc = ucnv_open("UTF8", &err);
	 if (U_FAILURE(err)) return EXIT_FAILURE;

	 const UNormalizer2 *NFKC = unorm2_getNFKCInstance(&err);
	 if (U_FAILURE(err)) return EXIT_FAILURE;
	 
	 for (i = 1; i < argc; ++i) {
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

		  /* convert UTF8 data to ICU's UTF16 */
		  UChar *usrc = (UChar*) malloc(2*len * sizeof(UChar));
		  ucnv_toUChars(uc, usrc, 2*len, (char*) src, len, &err);
		  if (U_FAILURE(err)) return EXIT_FAILURE;
		  size_t ulen = u_strlen(usrc);

		  /* ICU's insane normalization API requires you to
			 know the size of the destination buffer in advance,
			 or alternatively to repeatedly try normalizing and
			 double the buffer size until it succeeds.  Here, I just
			 allocate a huge destination buffer to avoid the issue. */
		  UChar *udest = (UChar*) malloc(10*ulen * sizeof(UChar));

		  mytime start = gettime();
		  for (int i = 0; i < 100; ++i) {
			   unorm2_normalize(NFKC, usrc, ulen, udest, 10*ulen, &err);
			   if (U_FAILURE(err)) return EXIT_FAILURE;
		  }
		  printf("%s: %g\n", argv[i], elapsed(gettime(), start) / 100);
		  free(udest);
		  free(usrc);
		  free(src);
	 }

	 return EXIT_SUCCESS;
}
