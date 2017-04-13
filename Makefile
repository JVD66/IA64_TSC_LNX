all:ttsc1 tlgtd scripts/vma2offset
clean:ttsc1.clean
CC=gcc
CPPFLAGS:=$(CPPFLAGS) -I.
CFLAGS:=$(CFLAGS) -std=gnu11 -O3 -mtune=native -g -fPIC -pthread -Wall 
LDFLAGS=$(CFLAGS) -lpthread -lm
ifeq ($(filter -DKSYM_LOCATION%,$(CPPFLAGS)),)
test/ttsc1.o: CPPFLAGS:=$(CPPFLAGS) -DKSYM_LOCATION=\"$(shell pwd)/scripts/ksym\"
endif
test/ttsc1.o:test/ttsc1.c IA64_tsc_info.h IA64_rdtsc.h
test/tlgtd.o:test/tlgtd.c linux_gtod_page.h
ttsc1: test/ttsc1.o
tlgtd: test/tlgtd.o
vma2offset.o:scripts/vma2offset.c
scripts/vma2offset: scripts/vma2offset.o
ttsc1.clean:
	@rm -vf test/ttsc1 test/ttsc1.o test/tlgtd test/tlgtd.o scripts/vma2offset scripts/vma2offset.o

