#include "equihash.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
  undefined,
  bench_mode,
  solve_mode,
  verify_mode,
} equihash_mode_t;

void bench(const uint32_t n, const uint32_t k, const uint32_t iter, const int verbose, const size_t seed_len, uint8_t *seed);

static void fatal(const char *error) {
  fprintf(stderr, "Error: %s\n", error);
  exit(1);
}

static void usage(const char *cmd) {
  printf("Usage: %s  [bench|solve|verify] [-v] [-n N] [-k K] "
         "[-i benchmark iterations]"
         "[-f file] "
         "[-s file]\n",
         cmd);
  printf("Parameters:\n");
  printf("\tbench \t\trun benchmark\n");
  printf("\tsolve \t\tsolve puzzle\n");
  printf("\tverify \t\tverify solution\n");
  printf("\t-v \t\tverbose\n");
  printf("\t-n N \t\tSets the tuple length of iterations to N\n");
  printf("\t-k K\t\tSets the number of steps to K \n");
  printf("\t-i \t\tsample size for benchmark\n");
  printf("\t-f file\t\tSets seed to file\n");
  printf("\t-s file\t\tSets solution to file\n");
}


int main(int argc, char *argv[]) {
  uint32_t n = 60, k=4, iter=10;
  char *filename=NULL, *solution=NULL;
  int verbose = 0;

  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  equihash_mode_t mode = undefined;

  /* parse options */
  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    unsigned long input = 0;
    if (!strcmp(a, "bench")) {
      mode = bench_mode;
      continue;
    }

    if (!strcmp(a, "solve")) {
      mode = solve_mode;
      continue;
    }

    if (!strcmp(a, "verify")) {
      mode = verify_mode;
      continue;
    }

    if (!strcmp(a, "-v")) {
      verbose = 1;
      continue;
    }

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

    else if (!strcmp(a, "-i")) {
      if (i < argc - 1) {
        i++;
        input = strtoul(argv[i], NULL, 10);
        if (input == 0) {
          fatal("bad numeric input for -i");
        }
        iter = input;
        continue;
      }
      else {
        fatal("missing -i argument");
      }
    }

    else if (!strcmp(a, "-f")) {
        if (i < argc - 1) {
            i++;
            filename = argv[i];
            continue;
        }
        else {
            fatal("missing -f argument");
        }
    }

    else if (!strcmp(a, "-s")) {
        if (i < argc - 1) {
            i++;
            solution = argv[i];
            continue;
        }
        else {
            fatal("missing -s argument");
        }
    }
  }

  size_t seed_len = 0;
  if(filename==NULL) {
    if(mode != bench_mode) {
      fatal("must provide input file for non-benchmarking modes\n");
    }
    seed_len = 16; // debian-style randomly and arbitrarily aus dem arsch gezogen
  } else {
    struct stat statbuf;
    if(0!=stat(filename, &statbuf)) {
      fprintf(stderr, "failed to stat file: %s\n", filename);
      return 1;
    }
    seed_len = statbuf.st_size;
  }

  uint8_t seed[seed_len];
  if(filename) {
    FILE *fp = fopen(filename, "r");
    if(1!=fread(seed,sizeof(seed),1, fp)) {
      fprintf(stderr,"failed to load %s\n", filename);
      return 1;
    }
    fclose(fp);
  } else {
    memset(seed,0,seed_len);
  }

  if(mode == bench_mode) {
    bench(n, k, iter, verbose, seed_len, seed);
    return 0;
  }

  if(mode == solve_mode) {
    size_t sol_len = solsize(n, k);
    uint8_t sol[sol_len];
    if(!solve(n, k, seed, seed_len, sol, sol_len)) {
      fprintf(stderr,"no solution found\n");
      return 1;
    }

    FILE *fp = fopen(solution, "w");
    if(1!=fwrite(sol,sizeof(sol),1, fp)) {
      fprintf(stderr,"failed to write solution to %s\n", solution);
      return 1;
    }
    fclose(fp);

  } else if (mode == verify_mode) {
    size_t sol_len = solsize(n, k);

    struct stat statbuf;
    if(0!=stat(solution, &statbuf)) {
      fprintf(stderr, "failed to stat solution: %s\n", solution);
      return 1;
    }
    if(statbuf.st_size<0 || sol_len!=(size_t)statbuf.st_size) {
      fprintf(stderr, "error: solution size(%ld) is incorrect, expected %ld", seed_len, sol_len);
      return 1;
    }
    uint8_t sol[sol_len];
    FILE *fp = fopen(solution, "r");
    if(1!=fread(sol,sizeof(sol),1, fp)) {
      fprintf(stderr,"failed to load %s\n", solution);
      return 1;
    }
    fclose(fp);

    return !verify(n, k, seed, seed_len, sol, sol_len);
  }

  return 0;
}
