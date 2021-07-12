CXXFLAGS=-Wall -g -O3 -std=c++17 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fpic -Werror=format-security -Wl,-z,defs -Wl,-z,relro -ftrapv -Wl,-z,noexecstack
LIBS=-lsodium
PREFIX?=/usr/local
SOEXT?=so

all: bench libequihash.so

android: EXTRA_SOURCES=jni.cc
android: CXXFLAGS+=-I$(SODIUM) -I$(SODIUM)/sodium -L.
android: libequihash.so

libequihash.so: equihash.cc equihash.h equihash.hpp Makefile $(EXTRA_SOURCES)
	$(CXX) -shared $(CXXFLAGS) -Wl,-soname,libequihash.so -o libequihash.so equihash.cc $(EXTRA_SOURCES) $(LIBS)

bench: equihash.cc equihash.hpp equihash-test.cc Makefile libequihash.so
	$(CXX) -Wall -g -march=native -O3 -std=c++17 equihash-test.cc $(LIBS) -L. -lequihash -o bench

install: $(PREFIX)/lib/libequihash.$(SOEXT) $(PREFIX)/include/equihash.h

$(PREFIX)/lib/libequihash.$(SOEXT): libequihash.$(SOEXT)
	cp $< $@

$(PREFIX)/include/equihash.h: equihash.h
	cp $< $@

clean:
	rm -f bench libequihash.so
