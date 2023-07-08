CURL=curl

CC = cc
CFLAGS ?= -O2
CFLAGS += -std=c99 -pedantic -Wall

all: bench

LIBUTF8PROC = ../utf8proc.o

bench: bench.o util.o $(LIBUTF8PROC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bench.o util.o $(LIBUTF8PROC)

DATAURL = https://raw.githubusercontent.com/duerst/eprun/master/benchmark
DATAFILES = Deutsch_.txt Japanese_.txt Korean_.txt Vietnamese_.txt

$(DATAFILES):
	$(CURL) -O $(DATAURL)/$@

bench.out: $(DATAFILES) bench
	./bench -nfkc $(DATAFILES) > $@

# you may need make CPPFLAGS=... LDFLAGS=... to help it find ICU
icu: icu.o util.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ icu.o util.o -licuuc

icu.out: $(DATAFILES) icu
	./icu $(DATAFILES) > $@

unistring: unistring.o util.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ unistring.o util.o -lunistring

unistring.out: $(DATAFILES) unistring
	./unistring $(DATAFILES) > $@

.c.o:
	$(CC) $(CPPFLAGS) -I.. $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.txt bench *.out icu unistring
