/* vns2offset.c : converts <address> <section address> <section size> <offset> <block_size> arguments in to <page>:<offset>
 * used by parser of 'objdump -fh /proc/kcore' (/usr/bin/kaddr2offs) to determine which section contains kernel virtual memory address.
 * Then the output of this is passed to eg. the timestamp module as its 'address of vsyscall_gtod_data structure', and it can
 * map a page of its memory read-only to that address in the kernel and get the timing parameters right.
 */
#define _GNU_SOURCE
#include <features.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "base_types.h"

int main(int argc, const char *const *argv, const char *const *envp)
{
  if( argc < 5 )
  { fprintf(stderr, "%s: expects <address in hex> <section vma in hex> <section size in hex> <section offset in hex> [ <block_size> (4096)] arguments.\n\tIFF the section contains the address, will print the <block>':'<offset> of block to seek to and offset within block of address.\n", argv[0]);
    return 1;
  }
  register
  UL_t address=strtoul(argv[1], NULL, 16)
     , vma    =strtoul(argv[2], NULL, 16)
     , size   =strtoul(argv[3], NULL, 16)
     , offset =strtoul(argv[4], NULL, 16)
     , bs     =strtoul(argv[5], NULL, 0)
    ;
  if((bs == 0) || (bs==~0UL))
  { fprintf(stderr,"assuming block size is 4096.\n");
    bs=4096;
  }
  if( bs !=  (bs & ~(bs - 1) ) )
  { fprintf(stderr,"only one bit in block size=%lx (%ld) may be set.\n", bs, bs);
    return 1;
  }
  if(   (address != ~0UL) && (vma != ~0UL) && (size != ~0UL) && (offset != ~0UL) 
     && (address > 0UL) && (vma > 0UL) && (size > 0UL) && (offset > 0UL) 
     && (address >= vma)
     && (address <= (vma + size))
    )
  { register UL_t page = ((address - vma) + offset) / bs
                , offs = ((address - vma) + offset) % bs;
    printf("%lu:%lu\n", page, offs);
    return 0;
  }
  return 1;
}
