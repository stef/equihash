#ifndef __EQUIHASH_H
#define __EQUIHASH_H

#include <stdint.h>
#include <stddef.h>

size_t solsize(const unsigned n, const unsigned k);
int solve(const unsigned n, const unsigned k, const uint8_t *seed, const size_t seed_len, uint8_t *csol, const size_t csol_len);
int verify(const unsigned n, const unsigned k, const uint8_t *seed, const size_t seed_len, const uint8_t *sol, const size_t sol_len);

#endif //define __EQUIHASH_H
