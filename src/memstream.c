/* 
 * ----------------------------------------------------------------------------
 * 
 * OPEN_MEMSTREAM(3)      BSD and Linux Library Functions     OPEN_MEMSTREAM(3)
 * 
 * SYNOPSIS
 *     #include "memstream.h"
 * 
 *     FILE *open_memstream(char **bufp, size_t *sizep);
 * 
 * DESCRIPTION
 *     The open_memstream()  function opens a  stream for writing to  a buffer.
 *     The   buffer  is   dynamically  allocated   (as  with   malloc(3)),  and
 *     automatically grows  as required.  After closing the  stream, the caller
 *     should free(3) this buffer.
 * 
 *     When  the  stream is  closed  (fclose(3))  or  flushed (fflush(3)),  the
 *     locations  pointed  to  by  bufp  and  sizep  are  updated  to  contain,
 *     respectively,  a pointer  to  the buffer  and  the current  size of  the
 *     buffer.  These values  remain valid only as long  as the caller performs
 *     no further output  on the stream.  If further  output is performed, then
 *     the  stream  must  again  be  flushed  before  trying  to  access  these
 *     variables.
 * 
 *     A null byte  is maintained at the  end of the buffer.  This  byte is not
 *     included in the size value stored at sizep.
 * 
 *     The stream's  file position can  be changed with fseek(3)  or fseeko(3).
 *     Moving the file position past the  end of the data already written fills
 *     the intervening space with zeros.
 * 
 * RETURN VALUE
 *     Upon  successful  completion open_memstream()  returns  a FILE  pointer.
 *     Otherwise, NULL is returned and errno is set to indicate the error.
 * 
 * CONFORMING TO
 *     POSIX.1-2008
 * 
 * ----------------------------------------------------------------------------
 */

#ifdef __APPLE__

#include "memstream.h"

#if _POSIX_C_SOURCE < 200809L

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

struct memstream
{
    int      position;
    int      size;
    int      capacity;
    char    *contents;
    char   **ptr;
    size_t  *sizeloc;
};

#if MEMSTREAM_DEBUG
  static void memstream_print(struct memstream *ms)
  {
      printf("memstream %p {", ms);
      printf(" %i", ms->position);
      printf(" %i", ms->size);
      printf(" %i", ms->capacity);
      printf(" %p", ms->contents);
      printf(" }\n");
  }
# define memstream_info(ARGS) printf ARGS
#else
# define memstream_print(ms)
# define memstream_info(ARGS)
#endif

#define memstream_check(MS) if (!(MS)->contents) { errno= ENOMEM;  return -1; }

static int memstream_grow(struct memstream *ms, int minsize)
{
    int newcap= ms->capacity * 2;					memstream_check(ms);
    while (newcap <= minsize) newcap *= 2;				memstream_info(("grow %p to %i\n", ms, newcap));
    ms->contents= realloc(ms->contents, newcap);
    if (!ms->contents) return -1;	/* errno == ENOMEM */
    memset(ms->contents + ms->capacity, 0, newcap - ms->capacity);
    ms->capacity= newcap;
    *ms->ptr= ms->contents;		/* size has not changed */
    return 0;
}

static int memstream_read(void *cookie, char *buf, int count)
{
    struct memstream *ms= (struct memstream *)cookie;			memstream_check(ms);
    int n= min(ms->size - ms->position, count);				memstream_info(("memstream_read %p %i\n", ms, count));
    if (n < 1) return 0;
    memcpy(buf, ms->contents, n);
    ms->position += n;							memstream_print(ms);
    return n;
}

static int memstream_write(void *cookie, const char *buf, int count)
{
    struct memstream *ms= (struct memstream *)cookie;			memstream_check(ms);
    if (ms->capacity <= ms->position + count)
	if (memstream_grow(ms, ms->position + count) < 0)		/* errno == ENOMEM */
	    return -1;
    memcpy(ms->contents + ms->position, buf, count);			memstream_info(("memstream_write %p %i\n", ms, count));
    ms->position += count;
    if (ms->size < ms->position) *ms->sizeloc= ms->size= ms->position;	memstream_print(ms);
									assert(ms->size < ms->capacity);
									assert(ms->contents[ms->size] == 0);
    return count;
}

static fpos_t memstream_seek(void *cookie, fpos_t offset, int whence)
{
    struct memstream *ms= (struct memstream *)cookie;
    fpos_t pos= 0;							memstream_check(ms);
									memstream_info(("memstream_seek %p %i %i\n", ms, (int)offset, whence));
    switch (whence) {
	case SEEK_SET:	pos= offset;			break;
	case SEEK_CUR:	pos= ms->position + offset;	break;
	case SEEK_END:	pos= ms->size + offset;		break;
	default:	errno= EINVAL;			return -1;
    }
    if (pos >= ms->capacity) memstream_grow(ms, pos);
    ms->position= pos;
    if (ms->size < ms->position) *ms->sizeloc= ms->size= ms->position;	memstream_print(ms);  memstream_info(("=> %i\n", (int)pos));
									assert(ms->size < ms->capacity && ms->contents[ms->size] == 0);
    return pos;
}

static int memstream_close(void *cookie)
{
    struct memstream *ms= (struct memstream *)cookie;			if (!ms->contents) { free(ms);  errno= ENOMEM;  return -1; }
    ms->size= min(ms->size, ms->position);
    *ms->ptr= ms->contents;
    *ms->sizeloc= ms->size;						assert(ms->size < ms->capacity);
    ms->contents[ms->size]= 0;
    free(ms);
    return 0;
}

FILE *open_memstream(char **ptr, size_t *sizeloc)
{
    if (ptr && sizeloc) {
	struct memstream *ms= calloc(1, sizeof(struct memstream));
	FILE *fp= 0;							if (!ms) return 0;	/* errno == ENOMEM */
	ms->position= ms->size= 0;
	ms->capacity= 4096;
	ms->contents= calloc(ms->capacity, 1);				if (!ms->contents) { free(ms);  return 0; } /* errno == ENOMEM */
	ms->ptr= ptr;
	ms->sizeloc= sizeloc;
	memstream_print(ms);
	fp= funopen(ms, memstream_read, memstream_write, memstream_seek, memstream_close);
	if (!fp) {
	    free(ms->contents);
	    free(ms);
	    return 0;	/* errno set by funopen */
	}
	*ptr= ms->contents;
	*sizeloc= ms->size;
	return fp;
    }
    errno= EINVAL;
    return 0;
}

#endif /* _POSIX_C_SOURCE < 200809L */

#endif // __APPLE__
