/*Code by Dmitry Khovratovich, 2016
CC0 license
*/

#include "equihash.hpp"
#include <sodium.h>
#include <algorithm>
#include <arpa/inet.h>
#include <string.h>

extern "C" {
#include "equihash.h"
}

//const int NONCE_LENGTH=24; //Length of nonce in bytes;
const int MAX_NONCE = 0xFFFFF;
const int MAX_N = 32; //Max length of n in bytes, should not exceed 32
const unsigned FORK_MULTIPLIER=3; //Maximum collision factor

using namespace _POW;
using namespace std;

void Equihash::InitializeMemory() {
    uint32_t  tuple_n = ((uint32_t)1) << (n / (k + 1));
    Tuple default_tuple(k); // k blocks to store (one left for index)
    std::vector<Tuple> def_tuples(LIST_LENGTH, default_tuple);
    tupleList = std::vector<std::vector<Tuple>>(tuple_n, def_tuples);
    filledList= std::vector<unsigned>(tuple_n, 0);
    solutions.resize(0);
    forks.resize(0);
}

void Equihash::FillMemory(uint32_t length) { //works for k<=7
   const int seed_end_off = seed_len/sizeof(uint32_t);
   uint32_t input[seed_end_off + 2];
   uint8_t *input_ptr=(uint8_t*) input;
   for (unsigned i = 0; i < seed_len; ++i)
        input_ptr[i] = seed[i];
   input[seed_end_off] = nonce;
   input[seed_end_off + 1] = 0;
   uint32_t buf[MAX_N / 4];
   for (unsigned i = 0; i < length; ++i, ++input[seed_end_off + 1]) {
       //blake2b((uint8_t*)buf, &input, NULL, sizeof(buf), sizeof(input), 0);
       crypto_generichash((uint8_t*)buf, sizeof(buf), input_ptr, sizeof(input), NULL,0);
       uint32_t index = buf[0] >> (32 - n / (k + 1));
       unsigned count = filledList[index];
       if (count < LIST_LENGTH) {
           for (unsigned j = 1; j < (k + 1); ++j) {
               //select j-th block of n/(k+1) bits
               tupleList[index][count].blocks[j - 1] = buf[j] >> (32 - n / (k + 1));
           }
           tupleList[index][count].reference = i;
           filledList[index]++;
       }
   }
}

std::vector<Input> Equihash::ResolveTreeByLevel(Fork fork, unsigned level) {
    if (level == 0)
        return std::vector<Input>{fork.ref1, fork.ref2};
    auto v1 = ResolveTreeByLevel(forks[level - 1][fork.ref1], level - 1);
    auto v2 = ResolveTreeByLevel(forks[level - 1][fork.ref2], level - 1);
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

std::vector<Input> Equihash::ResolveTree(Fork fork) {
    return ResolveTreeByLevel(fork, forks.size());
}


void Equihash::ResolveCollisions(bool store) {
    const unsigned tableLength = tupleList.size();  //number of rows in the hashtable
    const unsigned maxNewCollisions = tupleList.size()*FORK_MULTIPLIER;  //max number of collisions to be found
    const unsigned newBlocks = tupleList[0][0].blocks.size() - 1;// number of blocks in the future collisions
    std::vector<Fork> newForks(maxNewCollisions); //list of forks created at this step
    auto tableRow = vector<Tuple>(LIST_LENGTH, Tuple(newBlocks)); //Row in the hash table
    vector<vector<Tuple>> collisionList(tableLength,tableRow);
    std::vector<unsigned> newFilledList(tableLength,0);  //number of entries in rows
    uint32_t newColls = 0; //collision counter
    for (unsigned i = 0; i < tableLength; ++i) {
        for (unsigned j = 0; j < filledList[i]; ++j)        {
            for (unsigned m = j + 1; m < filledList[i]; ++m) {   //Collision
                //New index
                uint32_t newIndex = tupleList[i][j].blocks[0] ^ tupleList[i][m].blocks[0];
                Fork newFork = Fork(tupleList[i][j].reference, tupleList[i][m].reference);
                //Check if we get a solution
                if (store) {  //last step
                    if (newIndex == 0) {//Solution
                        std::vector<Input> solution_inputs = ResolveTree(newFork);
                        solutions.push_back(Proof(n, k, seed, seed_len, nonce, solution_inputs));
                    }
                }
                else {         //Resolve
                    if (newFilledList[newIndex] < LIST_LENGTH && newColls < maxNewCollisions) {
                        for (unsigned l = 0; l < newBlocks; ++l) {
                            collisionList[newIndex][newFilledList[newIndex]].blocks[l]
                                = tupleList[i][j].blocks[l+1] ^ tupleList[i][m].blocks[l+1];
                        }
                        newForks[newColls] = newFork;
                        collisionList[newIndex][newFilledList[newIndex]].reference = newColls;
                        newFilledList[newIndex]++;
                        newColls++;
                    }//end of adding collision
                }
            }
        }//end of collision for i
    }
    forks.push_back(newForks);
    std::swap(tupleList, collisionList);
    std::swap(filledList, newFilledList);
}

Proof Equihash::FindProof(){
    this->nonce = 1;
    while (nonce < MAX_NONCE) {
        nonce++;
        InitializeMemory(); //allocate
        FillMemory(4UL << (n / (k + 1)-1));   //fill with hashes
        for (unsigned i = 1; i <= k; ++i) {
            bool to_store = (i == k);
            ResolveCollisions(to_store); //XOR collisions, concatenate indices and shift
        }
        //Duplicate check
        for (unsigned i = 0; i < solutions.size(); ++i) {
            auto vec = solutions[i].inputs;
            std::sort(vec.begin(), vec.end());
            bool dup = false;
            for (unsigned k = 0; k < vec.size() - 1; ++k) {
                if (vec[k] == vec[k + 1])
                    dup = true;
            }
            if (!dup)
                return solutions[i];
        }
    }
    return Proof(n, k, seed, seed_len, nonce, std::vector<uint32_t>());
}

int Proof::serialize(uint8_t *csol, const size_t csol_len) {
  if(csol_len!=4+solsize) return 0;

  *((uint32_t*)csol)=htonl(nonce);

  uint8_t b;
  for (uint32_t i = 0, j = 0, bits_left = digitbits + 1;
      j < solsize; csol[4+j++] = b) {
    if (bits_left >=8) {
      // Read next 8 bits, stay at same sol index
      b = inputs[i] >> (bits_left -= 8);
    } else { // less than 8 bits to read
      // Read remaining bits and shift left to make space for next sol index
      b = inputs[i];
      b <<= (8 - bits_left); // may also set b=0 if bits_left was 0, which is fine
      // Go to next sol index and read remaining bits
      bits_left += digitbits + 1 - 8;
      b |= inputs[++i] >> bits_left;
    }
  }
  return 1;
}

void Proof::dump() {
  printf("%08x ", nonce);
  for (unsigned i = 0; i < inputs.size(); ++i) {
    printf("%08x ", inputs[i]);
  }
  printf("\n");
}

Proof _POW::unserialize(const unsigned n, const unsigned k, const uint8_t *seed, const uint16_t seed_len, const uint8_t *input, const uint32_t blen) {

  const unsigned digitbits = (n/(k+1));
  const uint32_t proofsize = 1<<k;
  const unsigned solsize = proofsize *(digitbits+1) / 8;
  if(solsize+4 != blen) {
    return Proof();
  }

  const uint32_t nonce = ntohl(*((uint32_t*) input));

  const uint8_t *csol=input+4;
  std::vector<Input> sol;
  sol.resize(proofsize,0);

  for (uint32_t i = 0, j = 0, bits_left = digitbits + 1;
       i < blen-4; i++) {
    if(bits_left > 8) {
      sol[j] <<= 8;
      bits_left -= 8;
      sol[j] |= csol[i];
    } else if(bits_left == 8) {
      sol[j] <<= 8;
      sol[j] |= csol[i];
      bits_left = digitbits+1;
      j++;
    } else {
      sol[j] <<= bits_left;
      sol[j] |= (csol[i] >> (8-bits_left)) & ((1<<bits_left) - 1);
      sol[++j] = csol[i] & ((1 << (8-bits_left)) - 1);
      bits_left = (digitbits + 1) - (8-bits_left);
    }
  }

  return Proof(n, k, seed, seed_len, nonce, sol);
}


bool Proof::operator ==(const Proof &b) const {
  if(n!=b.n) return false;
  if(k!=b.k) return false;
  if(seed_len!=b.seed_len) return false;
  if(0!=memcmp(seed,b.seed,seed_len)) return false;
  if(nonce!=b.nonce) return false;
  if(inputs.size()!=b.inputs.size()) return false;
  for(unsigned i=0; i<inputs.size();i++) if(inputs[i]!=b.inputs[i]) return false;
  return true;
}

bool Proof::verify() {
   const int seed_end_off = seed_len/sizeof(uint32_t);
   uint32_t input[seed_end_off + 2];
   uint8_t *input_ptr=(uint8_t*) input;
   for (unsigned i = 0; i < seed_len; ++i)
        input_ptr[i] = seed[i];
    input[seed_end_off] = nonce;
    input[seed_end_off + 1] = 0;
    uint32_t buf[MAX_N / 4];
    std::vector<uint32_t> blocks(k+1,0);
    for (unsigned i = 0; i < inputs.size(); ++i) {
        input[seed_end_off + 1] = inputs[i];
        //blake2b((uint8_t*)buf, &input, NULL, sizeof(buf), sizeof(input), 0);
        crypto_generichash((uint8_t*)buf, sizeof(buf), input_ptr, sizeof(input), NULL,0);
        for (unsigned j = 0; j < (k + 1); ++j) {
            //select j-th block of n/(k+1) bits
            blocks[j] ^= buf[j] >> (32 - n / (k + 1));
        }
    }
    bool b = true;
    for (unsigned j = 0; j < (k + 1); ++j) {
        b &= (blocks[j] == 0);
    }
    return b;
}

size_t solsize(const unsigned n, const unsigned k)  {
  return ((1<<k) * ((n/(k+1)) + 1) / 8) + /* including nonce: */ 4;
}

int solve(const unsigned n, const unsigned k, const uint8_t *seed, const size_t seed_len, uint8_t *csol, const size_t csol_len) {
  if(csol_len!=solsize(n,k)) return 0;
  Equihash equihash(n,k,seed, seed_len);
  Proof p = equihash.FindProof();
  p.serialize(csol, csol_len);
  return 0;
}

int verify(const unsigned n, const unsigned k, const uint8_t *seed, const size_t seed_len, const uint8_t *sol, const size_t sol_len) {
  Proof p = unserialize(n,k,seed,seed_len, sol, sol_len);
  return p.verify();
}

