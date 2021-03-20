#!/usr/bin/env python3

import equihash

n=102
k=5
seed="some initial seed"
sol = equihash.solve(n, k, seed)
print(equihash.verify(n,k,seed,sol))
