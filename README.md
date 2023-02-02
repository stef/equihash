# Equihash (fork)

This is a fork of https://github.com/khovratovich/equihash/ - it is changed to be used in production setup, by:
   - cleaning up the code,
   - removing benchmark and trace/debug output,
   - reading in arbitrary seeds, and
   - (de)serializing the solutions.
   - Wrapping all this in a library,
   - providing a simple C API (solve/verify),
   - and providing python bindings to this.

This is an optimized C++ implementation of Equihash, the [memory-hard Proof-of-Work
 with fast verification](https://www.internetsociety.org/sites/default/files/blogs-media/equihash-asymmetric-proof-of-work-based-generalized-birthday-problem.pdf). Equihash is designed by Alex Biryukov and Dmitry Khovratovich, cryptographers at the [University of Luxembourg](https://www.cryptolux.org/index.php/Home).

Equihash is an asymmetric proof-of-work algorithm based on a computationally
hard generalized birthday problem, which requires a lot of memory to generate
a proof, but is instant
to verify. Equihash is adapted as the PoW in [Zcash](https://z.cash/) a public implementation
of the cryptocurrency protocol [Zerocash](http://zerocash-project.org/paper). It is possible to use Equihash in TLS as a [client puzzle](https://tools.ietf.org/html/draft-nygren-tls-client-puzzles-00).

Equihash has two parameters: **N** (width in bits) and **K** (length), which determine the complexity
of the underlying problem and thus the memory and time complexity of the Equihash PoW. The underlying hash function is Blake2b, but any collision-resistant hash function would work too.

The time complexity is proportional to K2^{N/(K+1)}, and memory complexity to 2^{K+N/(K+1)}. The proof size is 2^{K}(1+N/(K+1))+192 bits. Verification requires 2^K hashes and XORs.

Please report bugs as issues on this repository.

## Recommended parameters (N,K)

For cryptocurrencies: (100/110/120,4), (108/114/120/126,5).

For client puzzles: (60/70/80/90,4), (90/96/102,5).

## Usage

`make` builds the executable `equihash` and the library `libequihash.so`.

### Command-line utility

`equihash` is a command-line utility to benchmark specific Equihash instances
on your system and to solve challenges and verify solutions. To show usage instructions, run
`./equihash` without arguments as

```
 Usage: ./equihash  [bench|solve|verify] [-v] [-n N] [-k K] [-i benchmark iterations][-f file] [-s file]
Parameters:
        bench           run benchmark
        solve           solve puzzle
        verify          verify solution
        size            calculate size of solution
        -v              verbose
        -n N            Sets the tuple length of iterations to N
        -k K            Sets the number of steps to K
        -i              sample size for benchmark
        -f file         Sets seed to file
        -s file         Sets solution to file
```

For example, to compute Equihash using N=120 and k=5 using this
projects Makefile as seed, consuming at least 32 MB of RAM

```
$ ./equihash -n 120 -k 5 -s Makefile
```

To solve a challenge consisting of the makefile of this project for
N=40 and K=4:

```
$ ./equihash solve -v -n 40 -k 4 -f Makefile -s /tmp/s
```

When the equihash CLI frontend is solving a challenge, it emits on fd
3 a simple protocol that allows better UI integration (see also the
ehwait wrapper below).

To verify the solution of the previous example:
```
$ ./equihash verify -v -n 40 -k 4 -f Makefile -s /tmp/s
```

To calculate the size of a solution for a given N and K parameter simply run:
```
$ ./equihash size -n 40 -k 4
22
```

Additionally there is two simple wrapper scripts provided: ehwait and ehpuzzle.

The `ehwait` wrapper is a simple bash wrapper, which understands the
following simple notification protocol:

On filedescriptor 3 any tool that supports this (pwdsphinx for example
does) has to emit the an ASCII string containing the values of N and K
and terminated by a newline as soon as an equihash calculation is
started. When the calculation is finished, the time elapsed in seconds
(with fractions, no terminating newline) is written also as an ASCII
number to fd 3, the filedescriptor 3 closes immediately after
this. This allows tools that might use significant time to solve an
equihash challenge to notify the user about the waiting time.

ehwait supports two frontends: pinentry (default) and zenity, simply
prefix your call to `equihash` with `ehwait` (and optionally give the
frontend variable if you have zenity and want that as a frontend.)

```
$ FRONTEND=zenity ./ehwait ./equihash solve -n 100 -k 4 -f Makefile -s /tmp/s
```

The other wrapper that comes with equihash is `ehpuzzle`, this is a
simple bash wrapper which creates secure challenges that cannot be
predicted or precomputed by solvers. The challenger and the verifer
need to have a secret cryptographic key of 32 bytes to ensure
authenticity of the challenges.

To create a challenge:
```
ehpuzzle challenge 80 4 /tmp/data </tmp/key >/tmp/challenge
```
All parameters are positional, in this order:
 - N
 - K
 - a file containing arbitrary data

The wrapper needs the secret signing key on standard input and will
write the challenge on standard output.

A solver can then take the challenge and solve it:
```
ehpuzzle solve /tmp/challenge >/tmp/solution
```

The solving mode already makes use of ehwait so a user is also
notified on the UI that this will take some time.

Finally a verifier (which can be the same as the challenger) verifies
if the challenge is valid, and if it is it also verifies the solution,
if it is then the verifier returns with exit code 0:

```
ehpuzzle verify Makefile /tmp/solution </tmp/key && echo "all is good!"
```

## Alternative implementations


* [Zcash](https://github.com/zcash/zcash/) by
  [@str4d](https://github.com/str4d)

## Intellectual property

The Equihash code in this
repository is copyright (c) 2016 Dmitry Khovratovich (University of Luxembourg)  under
[CC0 license](https://creativecommons.org/about/cc0).

The license is GPL-compatible.

## Credits

This project was funded through the NGI0 PET Fund, a fund established
by NLnet with financial support from the European Commission's Next
Generation Internet programme, under the aegis of DG Communications
Networks, Content and Technology under grant agreement No 825310.
