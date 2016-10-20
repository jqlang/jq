#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "rand.h"
#include "tinymt64.h"

// TODO Add support for cryptographically secure PRNG
// TODO Add Windows CNG

// PRNG state
static tinymt64_t fast_rand;
// Function pointer for uniform random uint64_t
static uint64_t (*rand_uint)();


// ---
// API
// ---
bool jq_rand_initialized() {
  return rand_uint;
}

uint64_t jq_rand_int(uint64_t max_val) {
  uint64_t thresh = -1 - ((-1 % max_val) + 1) % max_val;
  uint64_t result;
  // Efficient rejection sample
  while (thresh < (result = rand_uint()));
  return result % max_val;
}

double jq_rand_double() {
  // Convert a uniform uint64_t to a uniform double in [0, 1)
  // XXX This implementation is taken from TinyMT and depends heavily on the
  // byte order and double implementation, but these are not checked at compile.
  union {
    uint64_t u;
    double d;
  } conv;
  conv.u = (rand_uint() >> 12) | UINT64_C(0x3ff0000000000000);
  return conv.d - 1.0;
}

#if HAVE_GETRANDOM_SYSCALL || HAVE__DEV_URANDOM
// ----------------------------------
// Systems with sources of randomness
// ----------------------------------
// Systems that that have a source of randomness we fill into a buffer

// Fill randbuf with new randomness
inline static void fill_randbuf();

static uint64_t randbuf[64];
static uint64_t * const end = randbuf + sizeof(randbuf) / sizeof(*randbuf);
static uint64_t *next = randbuf + sizeof(randbuf) / sizeof(*randbuf);

uint64_t rand_uint_system() {
  if (next == end) {
    fill_randbuf();
    next = randbuf;
  }
  return *next++;
}
static uint64_t (*rand_uint)() = &rand_uint_system;

#if HAVE_GETRANDOM_SYSCALL
// -------------------
// Modern linux kernel
// -------------------
// Use getrandom syscall which has some advantages over /dev/urandom

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/random.h>

inline static void fill_randbuf() {
  if (syscall(SYS_getrandom, randbuf, sizeof(randbuf), 0) != sizeof(randbuf)) {
    fprintf(stderr, "error: could not get data from random pool.\n");
    exit(2);
  }
}

#elif HAVE__DEV_URANDOM
// ------------
// Unix machine
// ------------
// Use /dev/urandom as fallback

inline static void fill_randbuf() {
  FILE *devur = fopen("/dev/urandom", "r");
  if (!devur) {
    fprintf(stderr, "error: could not open /dev/urandom.\n");
    exit(2);
  }
  if (fread(&randbuf, sizeof(randbuf), 1, devur) != 1) {
    fprintf(stderr, "error: could not get data from /dev/urandom.\n");
    exit(2);
  }
  if (fclose(devur)) {
    fprintf(stderr, "error: could not close /dev/urandom.\n");
    exit(2);
  }
  next = randbuf;
}

#else
#error impossible to reach here
#endif

#else
// No source of system randomness, so we need the user to initialize a
// seed for the PRNG

#endif


// ----------------
// Mersenne Twister
// ----------------
uint64_t rand_uint_mt() {
  return tinymt64_generate_uint64(&fast_rand);
}

void jq_rand_init_seed(uint64_t seed) {
  tinymt64_init(&fast_rand, seed);
  rand_uint = &rand_uint_mt;
}
