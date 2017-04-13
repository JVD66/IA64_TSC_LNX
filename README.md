IA64_TSC_LNX : Intel Core 64-bit Architecture Time Stamp Counter (TSC) Library for Linux
  Version    : 1.0.0a
  Author     : Jason Vas Dias<jason.vas.dias@gmail.com>
  
 Provides    :
 o  Detection of CPU & linux synthesized TSC CPU capabilities:
     constant_tsc nonstop_tsc rdtsc tsc_adjust CPUID:15H{ ART / TSC ratio, ART FREQ }
 o  Usage of Linux VDSO resident (shared memory map with kernel)
    vsyscall_gtod_data structure to duplicate exactly how Linux
    interprets TSC, but in User-Space ; also functions to read
    the actual 'Calibrated' TSC frequency value from live kernel.
 o  Installed files:
    /usr/lib64/libIA64_TSC_LNX.so -> libIA64_TSC_LNX.so.${IA64_TSC_LNX_VERSION}
    /usr/include/IA64_TSC_LNX/IA64_TSC.h
    /usr/share/man/man3/IA64_TSC.3
    ( if pkgconfig installed ):
    /usr/lib/pkgconfig/IA64_TSC.pc
    /usr/bin/tsc_info : A bash shell script to emit information about TSC.
 o Functions:
    bool IA64_has_monotonic_nonstop_tsc() ;
    // IFF the above function returns true, then these functions return
    // the Time Stamp Counter value:
    U64_t IA64_rdtscp();  /* cancellation point */
    U64_t IA64_rdtsc();   /* not cancellation point : do not use outside assembler where you control
                             processor pipelining & speculative branch prediction
                           */
    
