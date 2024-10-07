#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "signal.h"

void jq_handle_sigpipe(int signum)
{
#ifndef WIN32
  signal(SIGPIPE, SIG_IGN);
  fprintf(stderr, "jq: error: received SIGPIPE\n");
  exit(128 + SIGPIPE);
#endif
}

void jq_register_signal_handler()
{
#ifndef WIN32
  if (signal(SIGPIPE, jq_handle_sigpipe) == SIG_ERR ) {
    fprintf(stderr, "jq: failed to register signal handler");
    exit(1);
  }
#endif
}
