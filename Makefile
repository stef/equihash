CXXFLAGS=-Wall -g -march=native -O3 -std=c++17 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fpic -fstack-clash-protection -fcf-protection=full -Werror=format-security -Wl,-z,defs -Wl,-z,relro -ftrapv -Wl,-z,noexecstack
LIBS=-lsodium
PREFIX?=/usr/local
SOEXT?=so

all: bench libequihash.so

libequihash.so: equihash.cc equihash.h equihash.hpp Makefile
	g++ -shared $(CXXFLAGS) -Wl,-soname,libequihash.so -o libequihash.so equihash.cc $(LIBS)

bench: equihash.cc equihash.hpp equihash-test.cc Makefile libequihash.so
	g++ -Wall -g -march=native -O3 -std=c++17 equihash-test.cc $(LIBS) -L. -lequihash -o bench

install: $(PREFIX)/lib/libequihash.$(SOEXT) $(PREFIX)/include/equihash.h

$(PREFIX)/lib/libequihash.$(SOEXT): libequihash.$(SOEXT)
	cp $< $@

$(PREFIX)/include/equihash.h: equihash.h
	cp $< $@

clean:
	rm -f bench libequihash.so
