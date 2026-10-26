#ifndef UTIL_H
#define UTIL_H

#ifdef WIN32
/* For WriteConsoleW() below */
#include <windows.h>
#include <io.h>
#include <limits.h>
#include <processenv.h>
#include <shellapi.h>
#include <stdlib.h>
#include <wchar.h>
#include <wtypes.h>
#endif

#include "jv.h"

jv expand_path(jv);
jv get_home(void);
jv jq_realpath(jv);

/*
 * The Windows console does not do UTF-8.  Bytes written to it, with
 * fwrite() or with WriteFile() alike, are decoded with the console's
 * output code page, so anything but ASCII comes out as mojibake unless
 * the user happened to run `chcp 65001` first.  The wide character
 * console API is code page independent, so convert to UTF-16 and use
 * WriteConsoleW() instead.  We must not call SetConsoleOutputCP(): the
 * code page belongs to the console rather than to us, so changing it
 * also changes the shell we were started from (see #1121 and #1184).
 *
 * Only a console needs this; a file or a pipe wants the UTF-8 bytes.
 * The is_tty flag alone cannot tell the two apart, as it is computed
 * once, from stdout, and passed along even for writes to stderr, so
 * ask the handle we are about to write to.
 */

static void priv_fwrite(const char *s, size_t len, FILE *fout, int is_tty) {
#ifdef WIN32
  if (is_tty && len > 0 && len <= INT_MAX / sizeof(wchar_t)) {
    HANDLE h = (HANDLE)_get_osfhandle(fileno(fout));
    DWORD mode;
    if (GetConsoleMode(h, &mode)) {
      int wlen = MultiByteToWideChar(CP_UTF8, 0, s, (int)len, NULL, 0);
      wchar_t *ws = wlen > 0 ? malloc(wlen * sizeof(*ws)) : NULL;
      if (ws != NULL) {
        wlen = MultiByteToWideChar(CP_UTF8, 0, s, (int)len, ws, wlen);
        fflush(fout); /* keep our order with stdio writes to this stream */
        for (int done = 0; done < wlen;) {
          DWORD written;
          /* WriteConsoleW() may write fewer characters than asked for. */
          if (!WriteConsoleW(h, ws + done, wlen - done, &written, NULL) ||
              written == 0)
            break;
          done += written;
        }
        free(ws);
        return;
      }
    }
  }
#endif
  fwrite(s, 1, len, fout);
}

const void *_jq_memmem(const void *haystack, size_t haystacklen,
                       const void *needle, size_t needlelen);

#ifndef MIN
#define MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
   __typeof__ (b) _b = (b); \
   _a < _b ? _a : _b; })
#endif
#ifndef MAX
#define MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
   __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })
#endif

#include <time.h>

#ifndef HAVE_STRPTIME
char* strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

#endif /* UTIL_H */
