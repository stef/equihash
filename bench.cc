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

static uint64_t rdtsc(void) {
#ifdef _MSC_VER
    return __rdtsc();
#else
#if defined(__amd64__) || defined(__x86_64__)
    uint64_t rax, rdx;
    __asm__ __volatile__("rdtsc" : "=a"(rax), "=d"(rdx) : : );
    return (rdx << 32) | rax;
#elif defined(__i386__) || defined(__i386) || defined(__X86__)
    uint64_t rax;
    __asm__ __volatile__("rdtsc" : "=A"(rax) : : );
    return rax;
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 6)  // V6 is the earliest arch that has a standard cyclecount
  return 0;
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  // Read the user mode perf monitor counter access permissions.
  asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {  // Is it counting?
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      // The counter is set up to count every 64th cycle
      return static_cast<int64_t>(pmccntr) * 64;  // Should optimize to << 6
    }
  }
  return 0;
#else
#error "Not implemented!"
#endif
#endif
}

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
  double solve_tics=0, verify_tics=0;
  double solve_time=0,  verify_time=0;
  uint32_t failed = 0;
  for(uint32_t i=0; i<iter; i++) {
     Equihash equihash(n,k,seed, seed_len);
     clock_t cstart, cend;
     cstart = clock();
     uint64_t start= rdtsc();
     Proof p = equihash.FindProof();
     uint64_t end= rdtsc();
     cend = clock();
     solve_tics+=(double)(end - start) / (1UL << 20);
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
     start = rdtsc();
     p2.verify();
     end = rdtsc();
     cend = clock();
     verify_tics+=(double)(end - start) / (1UL << 20);
     verify_time+=(double)(cend - cstart) / CLOCKS_PER_SEC;

     if(verbose) {
       fprintf(stderr, "\r%d\t", i+1);
       fprintf(stderr, "solve: %2.2ftics\t%fs\t", solve_tics / (i+1), solve_time / (i+1));
       fprintf(stderr, "verify: %2.2ftics\t%fs\t", verify_tics / (i+1), verify_time / (i+1));
       fprintf(stderr, "failed: %d", failed);
     }

     seed[0]++;
     if((iter % (1<<8)) == 0 && iter > 0 && seed_len > 1) seed[1]++;
     if((iter % (1<<16)) == 0 && iter > 0 && seed_len > 2) seed[2]++;
     if((iter % (1<<24)) == 0 && iter > 0 && seed_len > 3) seed[3]++;
  }

  if(verbose) printf("\n");
  printf("%2.2f\t%f\t", solve_tics / iter, solve_time / iter);
  printf("%2.2f\t%f\n", verify_tics / iter, verify_time / iter);
}
