#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "util.h"

/* read file named FILENAME into an array of *len bytes,
   returning NULL on error */
uint8_t *readfile(const char *filename, size_t *len)
{
	 *len = 0;
	 struct stat st;
	 if (0 != stat(filename, &st)) return NULL;
	 *len = st.st_size;
	 FILE *f = fopen(filename, "r");
	 if (!f) return NULL;
	 uint8_t *s = (uint8_t *) malloc(sizeof(uint8_t) * *len);
	 if (!s) return NULL;
	 if (fread(s, 1, *len, f) != *len) {
		  free(s);
		  s = NULL;
	 }
	 fclose(f);
	 return s;
}

mytime gettime(void) {
	 mytime t;
	 gettimeofday(&t, NULL);
	 return t;
}

/* time difference in seconds */
double elapsed(mytime t1, mytime t0)
{
     return (double)(t1.tv_sec - t0.tv_sec) +
          (double)(t1.tv_usec - t0.tv_usec) * 1.0E-6;
}

