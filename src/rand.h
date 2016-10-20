#ifndef RAND_H
#define RAND_H

#include <stdbool.h>
#include <stdint.h>

// Returns true if random functions are working
bool jq_rand_initialized();

// Sets seed and instructs rand to use pseudorandom generation
void jq_rand_init_seed(uint64_t);

// Return a random int in [0, max_val)
uint64_t jq_rand_int(uint64_t max_val);

// Return a random double in [0, 1)
double jq_rand_double();

#endif
