/*Code by Dmitry Khovratovich, 2016
CC0 license
*/
#include "equihash.hpp"

#include <inttypes.h>
#include <ctime>
#include <string.h>
#include <cstdlib>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

void TestEquihash(const unsigned n, const unsigned k, const uint8_t *seed, const uint16_t seed_len) {
  Equihash equihash(n,k,seed, seed_len);
  clock_t cstart, cend;
  cstart = clock();
  uint64_t start= rdtsc();
  Proof p = equihash.FindProof();
  uint64_t end= rdtsc();
  cend = clock();
  printf("%2.2f %f ", (double)(end - start) / (1UL << 20), ((double) (cend - cstart)) / CLOCKS_PER_SEC);

  //p.dump();
  uint8_t csol[p.solsize+4];
  p.serialize(csol, sizeof(csol));

  Proof p2 = unserialize(n,k,seed,seed_len, csol, sizeof(csol));
  //p2.dump();
  assert(p==p2);
  cstart = clock();
  start = rdtsc();
  p2.verify();
  end = rdtsc();
  cend = clock();
  printf("%2.2f %f\n", (double)(end - start) / (1UL << 20), ((double) (cend - cstart)) / CLOCKS_PER_SEC);
}

static void fatal(const char *error) {
  fprintf(stderr, "Error: %s\n", error);
  exit(1);
}

static void usage(const char *cmd) {
  printf("Usage: %s  [-n N] [-k K] "
         "[-s S]\n",
         cmd);
  printf("Parameters:\n");
  printf("\t-n N \t\tSets the tuple length of iterations to N\n");
  printf("\t-k K\t\tSets the number of steps to K \n");
  //printf("\t-s S\t\tSets seed  to S\n");
}


int main(int argc, char *argv[]) {
  uint32_t n = 60, k=4;
  char *seedfn=NULL;
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  /* parse options */
  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    unsigned long input = 0;
    if (!strcmp(a, "-n")) {
      if (i < argc - 1) {
        i++;
        input = strtoul(argv[i], NULL, 10);
        if (input == 0 ||
            input > 255) {
          fatal("bad numeric input for -n");
        }
        n = input;
        continue;
      }
      else {
        fatal("missing -n argument");
      }
    }
    else if (!strcmp(a, "-k")) {
      if (i < argc - 1) {
        i++;
        input = strtoul(argv[i], NULL, 10);
        if (input == 0 ||
            input > 20) {
          fatal("bad numeric input for -k");
        }
        k = input;
        continue;
      }
      else {
        fatal("missing -k argument");
      }
    }
    if (!strcmp(a, "-s")) {
        if (i < argc - 1) {
            i++;
            seedfn = argv[i];
            continue;
        }
        else {
            fatal("missing -s argument");
        }
    }
  }

  size_t seed_len = 0;
  if(seedfn==NULL) {
    seed_len = 16; // debian-style randomly and arbitrarily aus dem arsch gezogen
  } else {
    struct stat statbuf;
    if(0!=stat(seedfn, &statbuf)) {
      fprintf(stderr, "failed to stat file: %s\n", seedfn);
      return 1;
    }
    seed_len = statbuf.st_size;
  }
  uint8_t seed[seed_len];
  if(seedfn) {
    FILE *fp = fopen(seedfn, "r");
    if(1!=fread(seed,sizeof(seed),1, fp)) {
      fprintf(stderr,"failed to load %s\n", seedfn);
      return 1;
    }
    fclose(fp);
  } else {
    memset(seed,0,seed_len);
  }
  //printf("N:\t%" PRIu32 " \n", n);
  //printf("K:\t%" PRIu32 " \n", k);
  //printf("SEED: ");
  //for (unsigned i = 0; i < SEED_LENGTH; ++i) {
  //    printf(" %02x", seed[i]);
  //}
  printf("Memory:\t\t%" PRIu64 "KiB\n", ((((uint32_t)1) << (n / (k + 1)))*LIST_LENGTH*k*sizeof(uint32_t)) / (1UL << 10));
  for(int i=0; i<100; i++) {
    TestEquihash(n,k,seed, seed_len);
    seed[0]++;
  }

  return 0;
}
