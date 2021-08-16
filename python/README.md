# pyequihash

This is the python bindings for libequihash.

## installation

you'll need https://github.com/stef/equihash/
which depends on libsodium.
a simple `pip install pyequihash` should suffice to install the bindings.

## usage

```python
import equihash

# input parameters
n=102
k=5
seed="some initial seed"
# try to solve the challenge
sol = equihash.solve(n, k, seed)
# verify the solution
print(equihash.verify(n,k,seed,sol))
```

## License

GPLv3.0+
