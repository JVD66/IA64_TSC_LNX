/* @FILE: IA64_rdtsc.h : makes Intel Core-i7 Architecture x86_64 TSC Values available in User Space
 *   see: SDM: "Intel® 64 and IA-32 Architectures Software Developer’s Manual" : https://software.intel.com/en-us/articles/intel-sdm
 *         v3, SDG, § 17.16: "Time Stamp Counter", v2, ch 4, § 547 "RDTSCP—Read Time-Stamp Counter and Processor ID" instruction ;
 *         Linux's kernel/time/clocksource.c clocks_calc_mult_shift() function.
 * Copyleft (C) 2017+ Jason Vas Dias (JVD) <jason.vas.dias@gmail.com> - Only Authorship Rights Reserved.
 */
#ifndef _IA64_RDTSC_H_
#define _IA64_RDTSC_H_
#include "IA64_tsc_info.h"
#include <math.h>
__thread
U32_t _ia64_tsc_user_cpu; /* TSC Aux value identifies CPU */

static inline __attribute__((always_inline))
U64_t
IA64_tsc_now()
{ if(!(    _ia64_invariant_tsc_enabled
      ||(( _cpu0id_fd == -1) && IA64_invariant_tsc_is_enabled(NULL,NULL))
      )
    )
  { fprintf(stderr, __FILE__":%d:(%s): must be called with invariant TSC enabled.\n",__LINE__,__FUNCTION__);
    return 0;
  }
  U32_t tsc_hi, tsc_lo;
  register UL_t tsc;
  asm volatile
  ( "rdtscp\n\t"
    "mov %%edx, %0\n\t"
    "mov %%eax, %1\n\t"
    "mov %%ecx, %2\n\t"  
  : "=m" (tsc_hi) ,
    "=m" (tsc_lo) ,
    "=m" (_ia64_tsc_user_cpu) :  
  : "%rax","%rcx","%rdx"
  );
  tsc=(((UL_t)tsc_hi) << 32)|((UL_t)tsc_lo);
  return tsc;
}

__thread
U64_t _ia64_first_tsc = 0xffffffffffffffffUL;

static inline __attribute__((always_inline))
U64_t IA64_tsc_ticks_since_start()
{ if(_ia64_first_tsc == 0xffffffffffffffffUL)
  { _ia64_first_tsc = IA64_tsc_now();
    return 0;
  }
  return (IA64_tsc_now() - _ia64_first_tsc) ;
}

static inline __attribute__((always_inline))
U64_t IA64_s_ns_since_start_a()
{ static __thread
  U128_t _tsc_p_s = ~0ULL;
  if( _tsc_p_s == ~0ULL )
    _tsc_p_s =  1000000000000ULL /  
       ( ((U128_t)IA64_tsc_khz()) * 1000ULL )
    ; /* number of peta?-(1e-12)seconds per TSC tick */
  register U128_t n_ps = IA64_tsc_ticks_since_start() * _tsc_p_s;                    /* 1e-12 seconds */
  register UL_t   n    = (n_ps / 1000ULL) + ((n_ps % 1000ULL>499)?1:0) ;
  register UL_t   s    = n / 1000000000UL;
  register UL_t   n_ns = n % 1000000000UL;                                           /* 1e-9  seconds */  
  return  ( (s << 32) | n_ns );  
}

static inline __attribute__((always_inline))
U64_t IA64_s_ns_since_start_b()
{ static __thread
  double _tsc_p_s = 0.0;
  if( _tsc_p_s == 0.0 )
    _tsc_p_s = 1.0 / (IA64_tsc_khz() * 1e3);
  register UL_t ticks  = IA64_tsc_ticks_since_start() ;
  register double ts   = ticks * _tsc_p_s;
  register UL_t   s    = floor( ( ts * 1e9 ) / 1e9 );
  register UL_t   n_ns = ( ts * 1e9 ) - s;                                           /* 1e-9  seconds */  
  return  ( (s << 32) | n_ns );  
}

#define NSEC_PER_SEC 1000000000

static inline __attribute__((always_inline))
void
ia64_tsc_calc_mult_shift
( register U32_t *mult,
  register U32_t *shift
)
{ /* paraphrases Linux clocksource.c's clocks_calc_mult_shift() function:
   * calculates second + nanosecond mult + shift in same way linux does. 
   * It is a shame we cannot use numerically accurate values calculated as in IA64_s_ns_since_start_b()
   * or IA64_s_ns_since_start_a() above, but they take much longer as they use long registers, and 
   * we want to be compatible with what linux returns in struct timespec ts after call to 
   * clock_gettime(CLOCK_MONOTONIC_RAW, &ts).
   */
  const U32_t scale=1000U;
  register U32_t from= IA64_tsc_khz();
  register U32_t to  = NSEC_PER_SEC / scale;
  register U64_t sec = ( ~0UL / from ) / scale;  
  sec = (sec > 600) ? 600 : ((sec > 0) ? sec : 1);
  register U64_t maxsec = sec * scale;
  UL_t tmp;
  U32_t sft, sftacc=32;
  /*
   * Calculate the shift factor which is limiting the conversion
   * range:
   */
  tmp = (maxsec * from) >> 32;
  while (tmp)
  { tmp >>=1;
    sftacc--;
  }
  /*
   * Find the conversion shift/mult pair which has the best
   * accuracy and fits the maxsec conversion range:
   */
  for (sft = 32; sft > 0; sft--) 
  { tmp = ((UL_t) to) << sft;
    tmp += from / 2;
    tmp = tmp / from;
    if ((tmp >> sftacc) == 0)
      break;
  }
  *mult = tmp;
  *shift = sft;
}

__thread
U32_t _ia64_tsc_mult = ~0U, _ia64_tsc_shift=~0U;

static inline __attribute__((always_inline))
U64_t IA64_s_ns_since_start()
{ if( ( _ia64_tsc_mult == ~0U ) || ( _ia64_tsc_shift == ~0U ) )
    ia64_tsc_calc_mult_shift( &_ia64_tsc_mult, &_ia64_tsc_shift);
  register U64_t cycles = IA64_tsc_ticks_since_start();   
  register U64_t ns = ((cycles *((UL_t)_ia64_tsc_mult))>>_ia64_tsc_shift);
  return( (((ns / NSEC_PER_SEC)&0xffffffffUL) << 32) | ((ns % NSEC_PER_SEC)&0x3fffffffUL) );
  /* Yes, we are purposefully ignoring durations of more than 4.2 billion seconds here! */
}

/*crash> p/x  vsyscall_gtod_data
$4 = {
  seq = 0xb31b64,
  vclock_mode = 0x1,
  cycle_last = 0x6318f6e931ae,
  mask = 0xffffffffffffffff,
  mult = 0x587afd,
  shift = 0x18,
  wall_time_snsec = 0x37ccd5db9ccb8a,
  wall_time_sec = 0x58a6788b,
  monotonic_time_sec = 0x930c,
  monotonic_time_snsec = 0x3848a6259ccb8a,
  wall_time_coarse_sec = 0x58a6788b,
  wall_time_coarse_nsec = 0x37ccd5db,
  monotonic_time_coarse_sec = 0x930c,
  monotonic_time_coarse_nsec = 0x3848a625,
  tz_minuteswest = 0x0,
  tz_dsttime = 0x0
}


crash> p  vsyscall_gtod_data
vsyscall_gtod_data = $3 = {
  seq = 11435528,
  vclock_mode = 1,
  cycle_last = 106241552967747,
  mask = 18446744073709551615,
  mult = 5798653,
  shift = 24,
  wall_time_snsec = 12717616550889732,
  wall_time_sec = 1487303904,
  monotonic_time_sec = 36705,
  monotonic_time_snsec = 12853751075817732,
  wall_time_coarse_sec = 1487303904,
  wall_time_coarse_nsec = 758029016,
  monotonic_time_coarse_sec = 36705,
  monotonic_time_coarse_nsec = 766143266,
  tz_minuteswest = 0,
  tz_dsttime = 0
}

Inspection shows that above code calculates SAME mult and shift values as in 
the actual linux vsyscall_gtod_data 'shift' and 'mult' values, and that 
the expression :
    ((cycles *((UL_t)_ia64_tsc_mult))>>_ia64_tsc_shift)
actually does correspond to what linux would do with the TSC value to get timespec tv_sec (high 64 bits) 
and timespec tv_nsec (low 30 bits) .

IMHO, It is a shame that the actual hardware ART numerator + denominator + Crystal Clock frequency aren't
actually used in Linux's calculations! (except in e1000 driver's 'PTP' timestamp-synchronization-with-remote-host code).
 */
#endif
