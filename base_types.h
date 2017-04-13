#ifndef _BASE_TYPES_
#define _BASE_TYPES_
#include <features.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
typedef unsigned long  UL_t, U64_t;
typedef unsigned long long  ULL_t, U128_t;
typedef unsigned int   U32_t;
typedef unsigned short U16_t;
typedef unsigned char  byte_t;
#endif
#ifndef _SYS_STATUS_
#define _SYS_STATUS_
#ifdef OK
#undef OK
#endif
#ifdef FAIL
#undef FAIL
#endif
#ifdef EXIT_OK
#undef EXIT_OK
#endif
#ifdef EXIT_FAIL
#undef EXIT_FAIL
#endif
typedef enum sys_status_e
{ OK       = 0 
, FAIL     = -1
, BAD_FD   = -1  
, EXIT_OK  = 0 
, EXIT_FAIL= 1
} SysStatus_t;
#endif
