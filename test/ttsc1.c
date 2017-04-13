/* ttsc1.c : Demonstrates latency of use of IA64_rdtsc.h's IA64_s_ns_since_start()  vs. using clock_gettime(CLOCK_MONOTONIC_RAW)
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

#include "IA64_tsc_info.h"
#include "IA64_rdtsc.h"

int main(int argc, const char *const* argv, const char *const* envp)
{ bool
    has_tsc         =IA64_has_tsc()
  , has_constant_tsc=IA64_has_constant_tsc()
  , has_15h         =IA64_has_cpuid_leaf_15h()
  , has_80000007h   =IA64_has_cpuid_leaf_80000007h();
  fprintf(stderr,"has tsc: %u constant: %u\n",has_tsc, has_constant_tsc);
  if( has_tsc && has_constant_tsc )
  { UL_t tsc_khz = IA64_tsc_khz();
    IA64_nccc_t nccc={0};
    double tsc_n_freq=0.0;
    bool tsc_is_enabled = IA64_invariant_tsc_is_enabled(&nccc, &tsc_n_freq);
    if( tsc_is_enabled &&  has_15h )
        fprintf(stderr,"Invariant TSC is enabled : TSC / CCC: %u / %u = %u ; CCC freq: %uHz  Nominal TSC freq:: %.9EHz : Actual TSC freq: %.9GGHz - TSC adjust: %u.\n"
        , nccc.tsc_ccc_n , nccc.tsc_ccc_d
        , nccc.tsc_ccc_n / nccc.tsc_ccc_d
        , nccc.tsc_ccc_f
        , tsc_n_freq
        , (((double)tsc_khz)*1e3)/1e9
        , IA64_has_tsc_adjust()
        );
    else if( tsc_is_enabled &&  has_80000007h )     
      fprintf(stderr,"Invariant TSC is enabled: Actual TSC freq: %.9GGHz - TSC adjust: %u.\n",(((double)tsc_khz)*1e3)/1e9, IA64_has_tsc_adjust());
    if(tsc_is_enabled)
    { struct timespec tsp1, tsp2, tsp_b, tsp_e;
      clock_gettime(CLOCK_MONOTONIC_RAW,&tsp_b);
      clock_gettime(CLOCK_MONOTONIC_RAW,&tsp1);
      clock_gettime(CLOCK_MONOTONIC_RAW,&tsp2);            
      register UL_t ts0 = IA64_s_ns_since_start();      
      register UL_t ts2 = IA64_s_ns_since_start();
      register UL_t ts1 = ts2;
      register UL_t ts3 = IA64_s_ns_since_start();
      clock_gettime(CLOCK_MONOTONIC_RAW,&tsp_e);                                    
      register U128_t
        n_ns1=(((((U128_t)tsp2.tv_sec) * 1000000000ULL) + ((U128_t) tsp2.tv_nsec)) -      
               ((((U128_t)tsp1.tv_sec) * 1000000000ULL) + ((U128_t) tsp1.tv_nsec)))
      , n_ns2=(((((U128_t)tsp_e.tv_sec) * 1000000000ULL) + ((U128_t) tsp_e.tv_nsec)) -      
               ((((U128_t)tsp_b.tv_sec) * 1000000000ULL) + ((U128_t) tsp_b.tv_nsec)));
      fprintf(stderr, "ts2 - ts1: %lu ts3 - ts2: %lu ns1: %u.%.9u ns2: %u.%.9u\n",
        ts2 - ts0, ts3-ts2
      , (U32_t)((n_ns1 / 1000000000ULL)&0xffffffffUL), (U32_t)((n_ns1 % 1000000000ULL) & 0xffffffffUL)
      , (U32_t)((n_ns2 / 1000000000ULL)&0xffffffffUL), (U32_t)((n_ns2 % 1000000000ULL) & 0xffffffffUL)
      );
      register byte_t i;    
      for( i = 0; i < 10; i+=1 )
      { clock_gettime(CLOCK_MONOTONIC_RAW,&tsp1);
        clock_gettime(CLOCK_MONOTONIC_RAW,&tsp2);      
        ts2 = IA64_s_ns_since_start();      
        ts3 = IA64_s_ns_since_start();
        n_ns1=(((((U128_t)tsp2.tv_sec) * 1000000000ULL) + ((U128_t) tsp2.tv_nsec)) -      
               ((((U128_t)tsp1.tv_sec) * 1000000000ULL) + ((U128_t) tsp1.tv_nsec)))
              ;
        fprintf(stderr, "ts3 - ts2: %lu ns1: %u.%.9u\n",
          ts3 - ts2
        , (U32_t)((n_ns1 / 1000000000ULL)&0xffffffffU)
        , (U32_t)((n_ns1 % 1000000000ULL) & 0xffffffffUL)
        );
        ts1= IA64_s_ns_since_start();
        clock_gettime(CLOCK_MONOTONIC_RAW,&tsp_e);                              
      }
      n_ns2=(((((U128_t)tsp_e.tv_sec) * 1000000000ULL) + ((U128_t) tsp_e.tv_nsec)) -      
            ((((U128_t)tsp_b.tv_sec) * 1000000000ULL) + ((U128_t) tsp_b.tv_nsec)));      
      fprintf(stderr, "t1 - t0: %lu - ns2: %u.%.9u\n",
        ts1 - ts0
      , (U32_t)((n_ns2 / 1000000000ULL)&0xffffffffU), (U32_t)((n_ns2 % 1000000000ULL) & 0xffffffffUL)
      );      
    } else
    {  fprintf(stderr,__FILE__"%d:(%s): sorry, modern 7th-generation Intel \"Invariant TSC\" with ART (always running timer) is not fully enabled on this platform :\n\thas_tsc: %u has_leaf_15h: %u has_leaf_80000007h: %u has_constant_tsc: %u has_rdtscp: %u has_nonstop_tsc: %u \n",__LINE__,__FUNCTION__,
       has_tsc , has_15h, has_80000007h, has_constant_tsc,
       IA64_has_rdtscp(),
       IA64_has_nonstop_tsc()
      );
    }
  }
  return 0;
}
