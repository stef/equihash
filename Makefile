CFLAGS?=-Wall -g -O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fpic -Werror=format-security -Wl,-z,defs -Wl,-z,relro -ftrapv -Wl,-z,noexecstack
CXXFLAGS?=-Wall -g -O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fpic -Werror=format-security -Wl,-z,defs -Wl,-z,relro -ftrapv -Wl,-z,noexecstack
CXXFLAGS+=-std=c++17
LIBS=-lsodium
PREFIX?=/usr/local
SOEXT?=so
EXTRA_CXXFLAGS=-fstack-clash-protection
#EXTRA_CXXFLAGS+=-march=native 
INSTALL?=install

SYS=$(shell gcc -dumpmachine)
ifneq (, $(findstring x86_64, $(SYS)))
    EXTRA_CXXFLAGS+=-fcf-protection=full
endif

all: libequihash.so libequihash.a equihash

android: EXTRA_SOURCES=jni.cc
android: CXXFLAGS+=-I$(SODIUM) -I$(SODIUM)/sodium -L.
android: libequihash.so
android: EXTRA_CXXFLAGS=

equihash.o: equihash.cc equihash.h equihash.hpp Makefile
	$(CXX) -c $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o equihash.o equihash.cc

libequihash.so: equihash.o Makefile $(EXTRA_SOURCES)
	$(CXX) -shared $(CXXFLAGS) $(EXTRA_CXXFLAGS) -Wl,-soname,libequihash.so.0 -o libequihash.so equihash.o $(EXTRA_SOURCES) $(LIBS)

libequihash.a: equihash.o
	ar rcs $@ $^

bench.o: bench.cc
	$(CXX) -c $(CXXFLAGS) -o bench.o bench.cc

equihash: main.c bench.o
	$(CC) -g $(CFLAGS) main.c $(LIBS) bench.o -L. -lequihash -lstdc++ -o equihash

libequihash.pc:
	echo "prefix=$(PREFIX)" >libequihash.pc
	cat libequihash.pc.skel >>libequihash.pc

install: $(DESTDIR)$(PREFIX)/lib/libequihash.$(SOEXT) $(DESTDIR)$(PREFIX)/lib/libequihash.a $(DESTDIR)$(PREFIX)/include/equihash.h $(DESTDIR)$(PREFIX)/share/pkgconfig/libequihash.pc $(DESTDIR)$(PREFIX)/bin/equihash $(DESTDIR)$(PREFIX)/bin/ehwait $(DESTDIR)$(PREFIX)/bin/ehpuzzle

$(DESTDIR)$(PREFIX)/lib/libequihash.$(SOEXT): libequihash.$(SOEXT)
	$(INSTALL) -D $< $@

$(DESTDIR)$(PREFIX)/lib/libequihash.a: libequihash.a
	$(INSTALL) -D -m 0644 $< $@

$(DESTDIR)$(PREFIX)/include/equihash.h: equihash.h
	$(INSTALL) -D -m 0644 $< $@

$(DESTDIR)$(PREFIX)/share/pkgconfig/libequihash.pc: libequihash.pc
	$(INSTALL) -D -m 0644 $< $@

$(DESTDIR)$(PREFIX)/bin/equihash: equihash
	$(INSTALL) -D -m 0755 $< $@

$(DESTDIR)$(PREFIX)/bin/ehwait: ehwait
	$(INSTALL) -D -m 0755 $< $@

$(DESTDIR)$(PREFIX)/bin/ehpuzzle: ehpuzzle
	$(INSTALL) -D -m 0755 $< $@

clean:
	rm -f libequihash.so libequihash.pc libequihash.a equihash.o bench.o equihash
