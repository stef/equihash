CXXFLAGS=-Wall -O3 -std=c++17 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fpic -Werror=format-security -Wl,-z,defs -Wl,-z,relro -ftrapv -Wl,-z,noexecstack
LIBS=-lsodium
PREFIX?=/usr/local
SOEXT?=so
EXTRA_CXXFLAGS=-march=native -fstack-clash-protection -fcf-protection=full

all: bench libequihash.so libequihash.a

android: EXTRA_SOURCES=jni.cc
android: CXXFLAGS+=-I$(SODIUM) -I$(SODIUM)/sodium -L.
android: libequihash.so
android: EXTRA_CXXFLAGS=

equihash.o: equihash.cc equihash.h equihash.hpp Makefile
	$(CXX) -c $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o equihash.o equihash.cc

libequihash.so: equihash.o Makefile $(EXTRA_SOURCES)
	$(CXX) -shared $(CXXFLAGS) $(EXTRA_CXXFLAGS) -Wl,-soname,libequihash.so -o libequihash.so equihash.o $(EXTRA_SOURCES) $(LIBS)

libequihash.a: equihash.o
	ar rcs $@ $^

bench: equihash.cc equihash.hpp equihash-test.cc Makefile libequihash.so
	$(CXX) -Wall -march=native -O3 -std=c++17 equihash-test.cc $(LIBS) -L. -lequihash -o bench

libequihash.pc:
	echo "prefix=$(PREFIX)" >libequihash.pc
	cat libequihash.pc.skel >>libequihash.pc 

install: $(PREFIX)/lib/libequihash.$(SOEXT) $(PREFIX)/lib/libequihash.a $(PREFIX)/include/equihash.h $(PREFIX)/share/pkgconfig/libequihash.pc

$(PREFIX)/lib/libequihash.$(SOEXT): libequihash.$(SOEXT)
	cp $< $@

$(PREFIX)/lib/libequihash.a: libequihash.a
	cp $< $@

$(PREFIX)/include/equihash.h: equihash.h
	cp $< $@

$(PREFIX)/share/pkgconfig/libequihash.pc: libequihash.pc
	mkdir -p $(PREFIX)/share/pkgconfig/
	cp $< $@

clean:
	rm -f bench libequihash.so libequihash.pc libequihash.a equihash.o
