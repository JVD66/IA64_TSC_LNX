/* @FILE: IA64_tsc_info.h : makes Intel Core-i7 Architecture x86_64 TSC Information available in User Space
 *   see: SDM: "Intel® 64 and IA-32 Architectures Software Developer’s Manual" : https://software.intel.com/en-us/articles/intel-sdm
 *         v3, SDG, § 17.16: "Time Stamp Counter", v2, Ch 3, § 190 "CPUID Instruction", 3 § 208, "Feature Information Returned in EDX Register"
 * Copyleft (C) 2017+ Jason Vas Dias (JVD) <jason.vas.dias@gmail.com> - Only Authorship Rights Reserved.
 */
#ifndef _IA64_TSC_INFO_H_
#define _IA64_TSC_INFO_H_
#ifndef __x86_64
#error Oops, this code is just for the Intel x86_64 architecture.
#endif

#ifndef KSYM_LOCATION
#define KSYM_LOCATION "/usr/bin/ksym"
#warning -DKSYM_LOCATION= not specified in CPPFLAGS - default value /usr/bin/ksym being used.
#endif

#ifndef _BASE_TYPES_
#include "base_types.h"
#endif
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

typedef struct ia64_tsc_nccc_cpuid_15H_s
{ U32_t tsc_ccc_d; /* denominator: TSC  / NCCC freq */
  U32_t tsc_ccc_n; /* numerator  : TSC  / NCCC freq */
                   /* both numbers are normalized / minimum number to express ratio, NOT actual freq */
  U32_t tsc_ccc_f; /* actual core crystal clock frequency in HZ - this is the non-adjustable master clock oscillator frequency.
                    * NOT processor base frequency or HPET or TSC or core clock or reference clock or bus clock any other clock frequency! */
} IA64_nccc_t, IA64_TSC_Nominal_Core_Crystal_Clock_t; 

static inline __attribute__((always_inline))
bool
IA64_has_tsc(void)
{ U32_t caps=0;  
  asm volatile
  ( "mov $1, %%eax\n\t"
    "cpuid\n\t"
    "mov %%edx, %0"
  : "=m" (caps) :
  : "%rax","%rbx","%rcx","%rdx"
  );
  // bit 4 is 'has_tsc' :
  return ((caps & (1<<4))?true:false);
}

static __thread
U32_t _ia64_max_leaf=0;
static inline __attribute__((always_inline))
bool
IA64_has_cpuid_leaf_15h(void)
{ if( _ia64_max_leaf == 0 )    
    asm volatile
    ( "mov $0x00000000, %%eax\n\t"
      "cpuid\n\t"
      "mov %%eax,%0"
    : "=m" (_ia64_max_leaf) :
    : "%rax","%rbx","%rcx","%rdx"
    );
  return(0x014U < _ia64_max_leaf);
}

static __thread
U32_t _ia64_max_extended_leaf=0;
static inline __attribute__((always_inline))
bool
IA64_has_cpuid_leaf_80000007h(void)
{ if( _ia64_max_extended_leaf == 0 )
  { asm volatile
    ( "mov $0x80000000, %%eax\n\t"
      "cpuid\n\t"
      "mov %%eax,%0"
    : "=m" (_ia64_max_extended_leaf) :
    : "%rax","%rbx","%rcx","%rdx"
    );
    fprintf(stderr,"max_extended_leaf: %x\n", _ia64_max_extended_leaf);
  }
  return(0x80000006U < _ia64_max_extended_leaf );
}

static inline __attribute__((always_inline))
bool
IA64_has_constant_tsc(void)
{ U32_t caps=0;
  asm volatile
  ( "mov $0x80000007, %%eax\n\t"
    "cpuid\n\t"
    "mov %%edx, %0"
  : "=m" (caps) :
  : "%rax","%rbx","%rcx","%rdx"  
  );
  // bit 8 is 'constant_tsc' :
  return ((caps & (1<<8))?true:false);
}

static inline __attribute__((always_inline))
bool
IA64_has_rdtscp(void)
{ if( _ia64_max_extended_leaf ==  0U)
    IA64_has_cpuid_leaf_80000007h();
  if(0x80000001U > _ia64_max_extended_leaf)
    return false;
  U32_t caps=0;
  asm volatile
  ( "mov $0x80000001, %%eax\n\t"
    "cpuid\n\t"
    "mov %%edx, %0"
  : "=m" (caps) :
  : "%rax","%rbx","%rcx","%rdx"  
  );
  // bit 27 is 'has rdtscp & IA32_TSC_AUX' :
  
  return ((caps & (1<<27))?true:false);
}

static inline __attribute__((always_inline))
IA64_nccc_t
IA64_nccc(void)
{ IA64_nccc_t i={0,0,0};
  U32_t unu;
  asm volatile
  ( "mov $0,    %%ecx\n\t"
    "mov $0x15, %%eax\n\t"    
    "cpuid\n\t"
    "mov %%eax, %0\n\t"
    "mov %%ebx, %1\n\t"
    "mov %%ecx, %2\n\t"
    "mov %%edx, %3"    
  : "=m" (i.tsc_ccc_d) ,
    "=m" (i.tsc_ccc_n) ,
    "=m" (i.tsc_ccc_f) ,
    "=m" (unu) :
  : "%rax","%rbx","%rcx","%rdx"  
  );
  return i;
}

static inline __attribute__((always_inline))
double IA64_nominal_tsc_frequency_hz(register IA64_nccc_t *nccc)
{
  if( (nccc->tsc_ccc_f == 0) || (nccc->tsc_ccc_n == 0) || (nccc->tsc_ccc_d == 0) )
    return 0.0;
  return ( ((double)( nccc->tsc_ccc_f * nccc->tsc_ccc_n)) / ((double)nccc->tsc_ccc_d) );
}

static __thread
int  _cpu0id_fd=-1;
static __thread
char _cpu0id_data[256]={0};

static inline __attribute__((always_inline))
bool IA64_init_cpu0id_data()
{ if(FAIL == (_cpu0id_fd = open("/dev/cpu/0/cpuid", O_RDONLY)))
  { fprintf(stderr,__FILE__":%d:%s: failed to open /dev/cpu/0/cpuid: %d:'%s'.\n",__LINE__,__FUNCTION__,errno,strerror(errno));
    return false;
  }
  if(256  != read(_cpu0id_fd, _cpu0id_data, 256) )
  { close(_cpu0id_fd);
    fprintf(stderr,__FILE__":%d:%s: failed to read %u bytes on /dev/cpu/0/cpuid: %d:'%s'.\n",__LINE__,__FUNCTION__, 256 , errno,strerror(errno));
    return false;
  }
  close(_cpu0id_fd);  
  return true;
}


static inline __attribute__((always_inline))
bool
IA64_has_nonstop_tsc(void)
{ register char c='\0';
  if( _cpu0id_fd == -1 )
    if( ! IA64_init_cpu0id_data() )
      return false;
  c = _cpu0id_data[ ((3*32)+24)/8 ];
  if( c & 1 )
  { //fprintf(stderr,"has nonstop TSC\n");
    return true;
  }
  return false;
}

static inline __attribute__((always_inline))
bool
IA64_tsc_has_art(void)
{ register char c='\0';
  if( _cpu0id_fd == -1 )
    if(!IA64_init_cpu0id_data())
      return false;
  c = _cpu0id_data[ ((3*32)+10)/8 ];  
  if( c & 4 )
  {
    // fprintf(stderr,"has TSC ART hardware\n");
    return true;
  }
  return false;
}

static inline __attribute__((always_inline))
bool
IA64_has_tsc_adjust(void)
{ register char c='\0';
  if( _cpu0id_fd == -1 )
    if(!IA64_init_cpu0id_data())
      return false;
  c = _cpu0id_data[(9*32+1)/8];
  if( c & 1 )
    return true;
  return false;
}
  
__thread
bool _ia64_invariant_tsc_enabled=false;

static inline __attribute__((always_inline))
  bool IA64_invariant_tsc_is_enabled( register IA64_nccc_t *ncccp, register double *ntscfqp )
{ if( _cpu0id_fd != -1 )
    return _ia64_invariant_tsc_enabled;
  IA64_nccc_t tn;
  double tscfq;
  if(NULL == ncccp )
    ncccp = &tn;
  if(NULL == ntscfqp )
    ntscfqp = &tscfq;  
  return ( _ia64_invariant_tsc_enabled =
  ( IA64_has_tsc() &&
    IA64_has_constant_tsc() &&
    IA64_has_rdtscp() &&
    IA64_has_nonstop_tsc() &&  
    ( (IA64_has_cpuid_leaf_15h() &&
       ( 0.0 < (*ntscfqp = IA64_nominal_tsc_frequency_hz( (*ncccp = IA64_nccc(), ncccp) ) ) )
      ) ||
      IA64_has_cpuid_leaf_80000007h()
    )
  ) );   
}

__thread
U32_t _ia64_tsc_khz=0;

static inline __attribute__((always_inline))
U32_t IA64_tsc_khz()
{ UL_t ksym_addr;    
  if( _ia64_tsc_khz != 0 )
    return _ia64_tsc_khz;
  if( _ia64_invariant_tsc_enabled ||
  ((_cpu0id_fd == -1) && IA64_invariant_tsc_is_enabled(NULL,NULL))
    )
  { register size_t sz = getpagesize();
    register char *page = alloca(sz);
    register const char *ksym_location = (OK == access( KSYM_LOCATION, X_OK )) ? KSYM_LOCATION : ((OK == access( "./ksym", X_OK )) ? "./ksym" : "not found") ;
    if(OK != access( ksym_location, X_OK ))
    { fprintf(stderr,__FILE__":%d:(%s): unable to locate executable %s script - KSYM_LOCATION: %s.\n",__LINE__,__FUNCTION__, ksym_location, KSYM_LOCATION);
      return 0;
    }
    snprintf(page, sz,"%s tsc_khz 4", ksym_location);
    register FILE *ksym_fp=popen(page, "r");
    if( fscanf(ksym_fp, "%lx %s %x\n", &ksym_addr, page, &_ia64_tsc_khz) != 3 )
    { fprintf(stderr,__FILE__":%d:(%s): /usr/bin/ksym failed to lookup or provide a value for tsc_khz.\n",__LINE__,__FUNCTION__);
      fclose(ksym_fp);
      return 0;
    }
    fclose(ksym_fp);    
  }
  return _ia64_tsc_khz;
}

#endif

