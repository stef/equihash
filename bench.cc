/*Code by Dmitry Khovratovich, 2016
CC0 license
*/
#include "equihash.hpp"
extern "C" {
#include "equihash.h"
}

#include <inttypes.h>
#include <ctime>
#include <string.h>
#include <cstdlib>
#include <assert.h>
#include <time.h>
#include <sys/types.h>

using namespace _POW;

extern "C" {
  void bench(const uint32_t n, const uint32_t k, const uint32_t iter, const int verbose, const size_t seed_len, uint8_t *seed);
}

void bench(const uint32_t n, const uint32_t k, const uint32_t iter, const int verbose, const size_t seed_len, uint8_t *seed) {
  //printf("N:\t%" PRIu32 " \n", n);
  //printf("K:\t%" PRIu32 " \n", k);
  //printf("SEED: ");
  //for (unsigned i = 0; i < SEED_LENGTH; ++i) {
  //    printf(" %02x", seed[i]);
  //}
  printf("n = %d k = %d memory needed: %" PRIu64 "KiB\t solution size: %" PRIu64 "\t\n", n, k, ((((uint32_t)1) << (n / (k + 1)))*LIST_LENGTH*k*sizeof(uint32_t)) / (1UL << 10), solsize(n,k));
  double solve_time=0,  verify_time=0;
  uint32_t failed = 0;
  for(uint32_t i=0; i<iter; i++) {
     Equihash equihash(n,k,seed, seed_len);
     clock_t cstart, cend;
     cstart = clock();
     Proof p = equihash.FindProof();
     cend = clock();
     solve_time+=(double)(cend - cstart) / CLOCKS_PER_SEC;
     if(p.inputs.size()==0) {
       failed++;
       fprintf(stderr,"no solution found\n");
       continue;
     }
     //p.dump();
     uint8_t csol[p.solsize+4];
     p.serialize(csol, sizeof(csol));

     Proof p2 = unserialize(n,k,seed,seed_len, csol, sizeof(csol));
     //p2.dump();
     if(!(p==p2)) {
       p.dump();
       p2.dump();
       fflush(stdout);
     }
     assert(p==p2);
     cstart = clock();
     p2.verify();
     cend = clock();
     verify_time+=(double)(cend - cstart) / CLOCKS_PER_SEC;

     if(verbose) {
       fprintf(stderr, "\r%d\t", i+1);
       fprintf(stderr, "solve: %fs\t", solve_time / (i+1));
       fprintf(stderr, "verify: %fs\t", verify_time / (i+1));
       fprintf(stderr, "failed: %d", failed);
     }

     seed[0]++;
     if((iter % (1<<8)) == 0 && iter > 0 && seed_len > 1) seed[1]++;
     if((iter % (1<<16)) == 0 && iter > 0 && seed_len > 2) seed[2]++;
     if((iter % (1<<24)) == 0 && iter > 0 && seed_len > 3) seed[3]++;
  }

  if(verbose) printf("\n");
  printf("%f\t", solve_time / iter);
  printf("%f\n", verify_time / iter);
}
