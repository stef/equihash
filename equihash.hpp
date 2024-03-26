/*Code by Dmitry Khovratovich, 2016
CC0 license
*/

#ifndef __EQUIHASH_HPP
#define __EQUIHASH_HPP

#include <cstdint>

#include <vector>
#include <cstdio>

/* The block used to initialize the PoW search
   @v actual values
*/

namespace _POW{
  /* Different nonces for PoW search
     @v actual values
  */
  typedef uint32_t Nonce;
  typedef uint32_t Input;

  /*Actual proof of work */
  struct Proof{
    const unsigned n;
    const unsigned k;
    const uint8_t* seed;
    const uint16_t seed_len;
    const Nonce nonce;
    const std::vector<Input> inputs;
    const unsigned digitbits;
    const unsigned solsize;
    Proof(const unsigned n_v, const unsigned k_v, const uint8_t *I_v, const uint16_t seed_len, Nonce V_v, std::vector<Input> inputs_v):
      n(n_v), k(k_v), seed(I_v), seed_len(seed_len), nonce(V_v), inputs(inputs_v), digitbits(n_v/(k_v+1)), solsize(inputs_v.size() *(digitbits+1) / 8){};
    Proof():n(0),k(1),seed(0), seed_len(0),nonce(0),inputs(std::vector<Input>()), digitbits(0), solsize(0) {};
    bool operator ==(const Proof &b) const;
    int serialize(uint8_t *csol, const size_t csol_len);
    void dump();
    bool verify();
  };

  Proof unserialize(const unsigned n, const unsigned k, const uint8_t *seed, const uint16_t seed_len, const uint8_t *input, const uint32_t blen);

  class Tuple {
  public:
    std::vector<uint32_t> blocks;
    Input reference;
    Tuple(unsigned i) { blocks.resize(i); }
    Tuple(const Tuple &r) = default;
    Tuple& operator=(const Tuple &r) {
      blocks = r.blocks;
      reference = r.reference;
      return *this;
    }
  };

  class Fork {
  public:
    Input ref1, ref2;
    Fork() {};
    Fork(Input r1, Input r2) : ref1(r1), ref2(r2) {};
  };

  /*Algorithm class for creating proof
    Assumes that n/(k+1) <=32
   */
  class Equihash{
    std::vector<std::vector<Tuple>> tupleList;
    std::vector<unsigned> filledList;
    std::vector<Proof> solutions;
    std::vector<std::vector<Fork>> forks;
    const unsigned n;
    const unsigned k;
    const uint8_t *seed;
    const uint16_t seed_len;
    Nonce nonce;
  public:
    /*
      Initializes memory.
    */
    Equihash(const unsigned n_in, const unsigned k_in, const uint8_t *s, const uint16_t l) :n(n_in), k(k_in), seed(s), seed_len(l) {};
    ~Equihash() {};
    Proof FindProof();
    void FillMemory(uint32_t length);      //fill with hash
    void InitializeMemory(); //allocate memory
    void ResolveCollisions(bool store);
    std::vector<Input> ResolveTree(Fork fork);
    std::vector<Input> ResolveTreeByLevel(Fork fork, unsigned level);
    void PrintTuples(FILE* fp);
  };
}

#endif //define __EQUIHASH_HPP
