CXXFLAGS=-Wall -O3 -std=c++17 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fpic -Werror=format-security -Wl,-z,defs -Wl,-z,relro -ftrapv -Wl,-z,noexecstack
LIBS=-lsodium
PREFIX?=/usr/local
SOEXT?=so
EXTRA_CXXFLAGS=-march=native -fstack-clash-protection -fcf-protection=full

all: bench libequihash.so

android: EXTRA_SOURCES=jni.cc
android: CXXFLAGS+=-I$(SODIUM) -I$(SODIUM)/sodium -L.
android: libequihash.so
android: EXTRA_CXXFLAGS=

libequihash.so: equihash.cc equihash.h equihash.hpp Makefile $(EXTRA_SOURCES)
	$(CXX) -shared $(CXXFLAGS) $(EXTRA_CXXFLAGS) -Wl,-soname,libequihash.so -o libequihash.so equihash.cc $(EXTRA_SOURCES) $(LIBS)

bench: equihash.cc equihash.hpp equihash-test.cc Makefile libequihash.so
	$(CXX) -Wall -march=native -O3 -std=c++17 equihash-test.cc $(LIBS) -L. -lequihash -o bench

libequihash.pc:
	echo "prefix=$(PREFIX)" >libequihash.pc
	cat libequihash.pc.skel >>libequihash.pc 

install: $(PREFIX)/lib/libequihash.$(SOEXT) $(PREFIX)/include/equihash.h $(PREFIX)/share/pkgconfig/libequihash.pc

$(PREFIX)/lib/libequihash.$(SOEXT): libequihash.$(SOEXT)
	cp $< $@

$(PREFIX)/include/equihash.h: equihash.h
	cp $< $@

$(PREFIX)/share/pkgconfig/libequihash.pc: libequihash.pc
	cp $< $@

clean:
	rm -f bench libequihash.so libequihash.pc
