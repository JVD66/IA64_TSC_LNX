<i>IA64_TSC_LNX</i> : Intel Core 64-bit Architecture
Time Stamp Counter (TSC) Library for Linux
===
<pre>
  Version    : 1.0.0a  
  Author     : Jason Vas Dias<jason.vas.dias@gmail.com>  
 Provides    :
</pre> 
 <ul>
 <li> Detection of CPU & linux synthesized TSC CPU capabilities:  
      constant_tsc nonstop_tsc rdtsc tsc_adjust CPUID:15H{ ART / TSC ratio, ART FREQ }  
 <li> Usage of Linux VDSO resident (shared memory map with kernel)  
      vsyscall_gtod_data structure to duplicate exactly how Linux  
      interprets TSC, but in User-Space ; also functions to read  
      the actual 'Calibrated' TSC frequency value from live kernel.  
 <li> Installed files:
 <pre>
    /usr/lib64/libIA64_TSC_LNX.so -> libIA64_TSC_LNX.so.${IA64_TSC_LNX_VERSION}  
    /usr/include/IA64_TSC_LNX/IA64_TSC.h  
    /usr/share/man/man3/IA64_TSC.3  
    ( if pkgconfig installed ):  
    /usr/lib/pkgconfig/IA64_TSC.pc  
    /usr/bin/tsc_info : A bash shell script to emit information about TSC.
 </pre>    
 <li> Functions:
 <pre>
    bool IA64_has_monotonic_nonstop_tsc() ;  
    // IFF the above function returns true, then these functions return  
    // the Time Stamp Counter value:  
    U64_t IA64_rdtscp();  /* cancellation point */  
    U64_t IA64_rdtsc();   /* not cancellation point : do not use outside assembler  
                           * where you control processor pipelining & speculative  
                           * branch prediction  
                           */
 <pre>
</ul>

Demonstration :
<pre>
  $ make
  $ ./ttsc1
max_extended_leaf: 80000008
has tsc: 1 constant: 1
Invariant TSC is enabled: Actual TSC freq: 2.893299GHz - TSC adjust: 1.
ts2 - ts1: 219 ts3 - ts2: 107 ns1: 0.000000334 ns2: 0.000001955
ts3 - ts2: 132 ns1: 0.000000329
ts3 - ts2: 15 ns1: 0.000000323
ts3 - ts2: 17 ns1: 0.000000372
ts3 - ts2: 17 ns1: 0.000000319
ts3 - ts2: 17 ns1: 0.000000351
ts3 - ts2: 17 ns1: 0.000000342
ts3 - ts2: 17 ns1: 0.000000350
ts3 - ts2: 17 ns1: 0.000000358
ts3 - ts2: 17 ns1: 0.000000347
ts3 - ts2: 17 ns1: 0.000000319
t1 - t0: 46223 - ns2: 0.000047863
</pre>

The numbers on the right (shown as ns1) are the delta of the results clock_gettime(CLOCK_MONOTONIC_RAW(), &tsp),
those on the left (shown as 'ts3 - ts2') are the delta of the  results of IA64_s_ns_since_start() which uses IA64_rdtscp() .

<b>READERS PLEASE NOTE</b> :

<b><i>THIS PACKAGE IS NOT YET READY</i></b> .  I am creating the GIT today (2017-04-15) which
is my first GitHub project , to see what facilities GitHub provides & plan how
to structure build & repository .

This message will disappear in a few days when the project should be considered
'Ready For Download'.

I would make this GIT private during this period, but I don't want to pay
a monthly fee for this.





