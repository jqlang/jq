// winfd: open files onto specific file descriptors, then run a command.
// Needed on Windows only.
//
//   winfd [-r FD FILE|-w FD FILE|-a FD FILE|-b FD FILE]... [--] EXE [ARG...]
//
//   -r  open FILE read-only                           (sh: FD<FILE)
//   -w  open FILE write-only, create, truncate        (sh: FD>FILE)
//   -a  open FILE write-only, create, append          (sh: FD>>FILE)
//   -b  open FILE read/write, create, don't truncate  (sh: FD<>FILE)
//
// On Windows, file descriptors are emulated by the C runtime, and the table
// that maps them to OS handles is handed to child processes in a
// runtime-specific format. The `/bin/sh` provided by MSYS2, which uses the
// Cygwin/MSYS2 runtime, thus cannot pass any file descriptors above 2 to a `jq`
// that uses the native Windows runtime (msvcrt/ucrt). (The three standard
// streams have special support in the Win32 API.)
//
// As a consequence, on Windows, the shell script `tests/shtest` cannot use the
// shell to set up FDs to test `--rawfd` and `--slurpfd`. It instead goes
// through this program, which is built as a native Windows executable using the
// same toolchain that builds `jq`.
//
// NB: for compatibility with MSYS2 `sh`, the special quoting rules that usually
// apply to the program name on a Windows command line are not used to interpret
// `EXE`. The command line passed to `EXE` is thus not `EXE ARGS`. `winfd`
// instead unquotes `EXE` and then wraps it in quotes.

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>

#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include <windows.h>

#define WINERR_APP 0x20000000
#define CODE_ERROR WINERR_APP | 1
#define CODE_USAGE WINERR_APP | 2
#define CODE_SETUP WINERR_APP | 0x10000000 // ... | [a <WinError.h> code]
#define CODE_EXEC  WINERR_APP | 0x40000000 // ... | [a <WinError.h> code]

static void usage(void) {
  fputws(L"usage: winfd [-r|-w|-a|-b FD FILE]... [--] COMMAND [ARG...]\n",
         stderr);
  exit(CODE_USAGE);
}

static int parse_fd(const wchar_t *s) {
  wchar_t *end;
  errno = 0;
  long fd = wcstol(s, &end, 10);
  if (errno || end == s || *end || fd < 0 || fd > INT_MAX) {
    fwprintf(stderr, L"winfd: not a file descriptor number: %ls\n", s);
    exit(CODE_USAGE);
  }
  return (int)fd;
}

// Skip the pointed-at argument on a command line, following Windows conventions
// for quoting and argument splitting, and return a pointer to the beginning of
// the next argument or to the terminating NUL.
static wchar_t *skip_arg(const wchar_t *p, int is_program_name) {
  // We don't need to determine the actual argument value.
  int quoted = 0, backslashed = 0;
  while (*p && (quoted || !(*p == L' ' || *p == L'\t'))) {
    if (*p == L'"' && !backslashed)
      quoted = !quoted;
    backslashed = !is_program_name && !backslashed && *p == L'\\';
    p++;
  }
  while (*p == L' ' || *p == L'\t')
    p++;
  return (wchar_t*)p;
}

static void redirect(int target, const wchar_t *path, int flags) {
  if (target < 2) {
    fwprintf(stderr, L"winfd: refusing to redirect standard fd %d\n", target);
    exit(CODE_USAGE);
  }

  _doserrno = 0;
  int fd = _wopen(path, flags | _O_BINARY, _S_IREAD | S_IWRITE);
  if (fd < 0) {
    int code = CODE_SETUP | _doserrno;
    fwprintf(stderr, L"winfd: cannot open %ls: %ls\n", path, _wcserror(errno));
    exit(code);
  }

  if (fd == target)
    return;
  _doserrno = 0;
  if (_dup2(fd, target) < 0) {
    int code = CODE_SETUP | _doserrno;
    fwprintf(stderr, L"winfd: cannot dup2 onto fd %d: %ls\n",
             target, _wcserror(errno));
    exit(code);
  }
  _close(fd);
}

static wchar_t exearg[1 << 15];
int wmain(int argc, wchar_t **argv) {
  int i = 1;
  while (i < argc && argv[i][0] == L'-' && argv[i][1]) {
    const wchar_t *letters = argv[i] + 1;
    int flags;
    if      (!wcscmp(letters, L"-")) { i++; break; }
    else if (!wcscmp(letters, L"r")) flags = _O_RDONLY;
    else if (!wcscmp(letters, L"w")) flags = _O_WRONLY | _O_CREAT | _O_TRUNC;
    else if (!wcscmp(letters, L"a")) flags = _O_WRONLY | _O_CREAT | _O_APPEND;
    else if (!wcscmp(letters, L"b")) flags = _O_RDWR   | _O_CREAT;
    else                             usage();

    if (i + 2 >= argc)
      usage();
    redirect(parse_fd(argv[i + 1]), argv[i + 2], flags);
    i += 3;
  }
  if (i >= argc)
    usage();

  // argv[i] is EXE, and its ARGS follow. The CRT functions for creating a new
  // process (which are the ones that pass FDs > 2) take an "argv", but they
  // don't quote the arguments as needed to actually get the child to have the
  // specified `argv`. To transmit ARGS faithfully, we locate the corresponding
  // chunk of our raw command line and pass it verbatim as the new "argv[1]".
  const wchar_t *args = GetCommandLineW();
  for (int j = 0; j < i + 1; j++)
    args = skip_arg(args, !j);
  // EXE is the new `argv[0]`, but needs some massaging to make sure it gets to
  // the child in a legible form.
  int out = swprintf(exearg, sizeof exearg / sizeof *exearg,
                     L"\"%ls\"", argv[i]);
  if (out < 0) {
    fputws(L"winfd: failed to quote executable name\n", stderr);
    exit(CODE_ERROR);
  }

  const wchar_t *child_argv[] = {exearg, args, 0};
  _doserrno = 0;
  intptr_t status = _wspawnvp(_P_WAIT, argv[i], child_argv);
  if (status == -1) {
    int code = CODE_EXEC | _doserrno;
    fwprintf(stderr, L"winfd: cannot run %ls: %ls\n",
             exearg, _wcserror(errno));
    exit(code);
  }

  return (int)status;
}
