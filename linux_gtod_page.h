#ifndef _LINUX_GTOD_PAGE_H_
#define _LINUX_GTOD_PAGE_H_
#define _GNU_SOURCE
#include <features.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "base_types.h"

typedef unsigned long gtod_long_t;
/* copied from Linux's arch/x86/include/asm/vgtod.h :
 */  
struct vsyscall_gtod_data {
	unsigned seq;

	int vclock_mode;
	U64_t	cycle_last;
	U64_t	mask;
	U32_t	mult;
	U32_t	shift;

	/* open coded 'struct timespec' */
	U64_t		wall_time_snsec;
	gtod_long_t	wall_time_sec;
	gtod_long_t	monotonic_time_sec;
	U64_t		monotonic_time_snsec;
	gtod_long_t	wall_time_coarse_sec;
	gtod_long_t	wall_time_coarse_nsec;
	gtod_long_t	monotonic_time_coarse_sec;
	gtod_long_t	monotonic_time_coarse_nsec;

	int		tz_minuteswest;
	int		tz_dsttime;
};

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096U
#endif

__thread 
  struct vsyscall_gtod_data _lnx_gtod = {0};
  ;
__thread 
  int _lnx_kcore_fd=BAD_FD
  ;
__thread 
  UL_t _lnx_gtod_page=0
     , _lnx_gtod_offs=0
  ;
__thread
  bool _lnx_close_kcore_fd=false;

static const char _lnx_gtod_get_address_cmd[] =
"/bin/bash -c 'ksym vsyscall_gtod_data 4 | while read addr name rest; do kaddr2offs $addr; done'"
  ;

static void _init_lnx_gtod_page(void)
{ char buf[64];
  register bool needs_putenv=false; 
  if( _lnx_gtod_page == 0 )
  { register const char *p = getenv("LINUX_GTOD_PAGE_AND_OFFSET") ;
    register const char *b=p;
    if( p == NULL )
    { FILE *pfp=popen(_lnx_gtod_get_address_cmd, "r");
      if( pfp != NULL )
      { if(fscanf(pfp, "%63s", &buf[0]) != 1)
        { fprintf(stderr,"Oops, failed to determine address of vsyscall_gtod_data using command: '%s'.\n", _lnx_gtod_get_address_cmd);
          fclose(pfp);
          return;
        }
        fclose(pfp);
      }
      p = &buf[0];
      b = p;
      needs_putenv=true;
    }
    for(; (*p != '\0') && (*p != ':'); p+=1);
    if( *p != ':' )
    { fprintf(stderr,"Oops, the environment variable LINUX_GTOD_PAGE_AND_OFFSET=%s contains no offset (after ':'). Please use kaddr2offs to set this value.\n", b);
      return;
    } 
    register char *ps=alloca( (p - b) + 1 );
    memcpy( ps, b, p - b);
    ps[p-b]='\0';
    _lnx_gtod_page = strtoul(ps, NULL,0);
    _lnx_gtod_offs = strtoul(p+1,NULL,0);
  }
  register UL_t page = _lnx_gtod_page;
  register UL_t offs = _lnx_gtod_offs;
  register off_t offset = (page * PAGE_SIZE) + offs;
  register byte_t *d;
  register int fd, r, s;
  if( (page > 0) && (page != ~0UL) && (offs > 0) && (offs != ~0UL) && ( ! (offs & 3) ) )
  { if( _lnx_kcore_fd == BAD_FD )
    {  fd  = open("/proc/kcore", O_RDONLY);
       if( BAD_FD != fd )
       { 
         _lnx_kcore_fd = fd;
       } else
       { fprintf(stderr,"Oops, failed to open /proc/kcore  %d: '%s'.",  errno, strerror(errno));
         return;
       }
    } else
      fd = _lnx_kcore_fd;
  } else
  { fprintf(stderr,"Oops, the environment variable LINUX_GTOD_PAGE_AND_OFFSET=%s contains an invalid page (%lx) or (not 4-byte aligned) offset (%lx) . Please use kaddr2offs to set this value.\n",
      buf, page, offs
     );
    _lnx_gtod_page = _lnx_gtod_offs=0;
    return;
  }
  if( offset != lseek(fd, offset, SEEK_SET) )
  { fprintf(stderr,"Oops, failed to lseek /proc/kcore to %lx: %d: '%s'.", offset, errno, strerror(errno));
    close(fd);
    return;
  }  
  errno=0;
  s=sizeof(struct vsyscall_gtod_data);
  d=(byte_t*)&_lnx_gtod;
  while( s > 0 )
  { r = read( fd, d + (sizeof(struct vsyscall_gtod_data)-s), s);
    if( r <= 0 )
    { fprintf(stderr,"Oops, failed to open /proc/kcore: %d: '%s'.", errno, strerror(errno));
      close(fd);          
      return;          
    }
    s -= r;
  }
  if( _lnx_close_kcore_fd )
  { close(fd);
    _lnx_kcore_fd = BAD_FD;  
  }else
    lseek(fd, offset, SEEK_SET);
  if(needs_putenv)
  { snprintf(buf,63,"LINUX_GTOD_PAGE_AND_OFFSET=%lx:%lx",page,offs);
    putenv(buf);
  }
  /*
  if( _lnx_gtod.vclock_mode != 0 )
  {
    fprintf(stderr, "%s: read linux GTOD structure @ 0x%lx:%lx in /proc/kcore\nGTOD DATA : %u %u %u\n",
    __FUNCTION__,page *PAGE_SIZE,  offs, _lnx_gtod.vclock_mode, _lnx_gtod.mult, _lnx_gtod.shift);
  }
  */
}

static void _init_lnx_gtod_page(void) __attribute__((constructor));

static void lnx_gtod_sync(void) __attribute__((alias("_init_lnx_gtod_page")));

#endif
