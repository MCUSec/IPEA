#ifndef _STD_TESTCASE_H
#define _STD_TESTCASE_H

/* This file exists in order to:
 * 1) Include lots of standardized headers in one place
 * 2) To avoid #include-ing things in the middle of your code
 *    #include-ing in the middle of a C/C++ file is apt to cause compiler errors
 * 3) To get good #define's in place
 *
 * In reality you need a complex interaction of scripts of build processes to do
 * this correctly (i.e., autoconf)
 */

#ifdef _WIN32
/* Ensure the CRT does not disable the "insecure functions"
 * Ensure not to generate warnings about ANSI C functions.
 */
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNING 1

/* We do not use _malloca as it sometimes allocates memory on the heap and we
 * do not want this to happen in test cases that expect stack-allocated memory
 * Also, _alloca_s cannot be used as it is deprecated and has been replaced
 * with _malloca */
#define ALLOCA _alloca

/* disable warnings about use of POSIX names for functions like execl()
 Visual Studio wants you to use the ISO C++ name, such as _execl() */
#pragma warning(disable:4996)

#else
/* Linux/GNU wants this macro, otherwise stdint.h and limits.h are mostly useless */
# define __STDC_LIMIT_MACROS 1

#define ALLOCA alloca
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
/* SIZE_MAX, int64_t, etc. are in this file on Linux */
# include <stdint.h>
#endif
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h> /* for open/close etc */
#endif

#ifdef __cplusplus
#include <new> // for placement new

/* classes used in some test cases as a custom type */
class TwoIntsClass 
{
    public: // Needed to access variables from label files
        int intOne;
        int intTwo;
};

class OneIntClass 
{
    public: // Needed to access variables from label files
        int intOne;
};

#endif

#ifndef __cplusplus
/* Define true and false, which are included in C++, but not in C */
#define true 1
#define false 0

#endif /* end ifndef __cplusplus */

/* rand only returns 15 bits, so we xor 3 calls together to get the full result (13 bits overflow, but that is okay) */
// shifting signed values might overflow and be undefined
#define URAND31() (((unsigned)rand()<<30) ^ ((unsigned)rand()<<15) ^ rand())
// choose to produce a positive or a negative number. Note: conditional only evaluates one URAND31
#if defined(TEST_CWE121) || defined(TEST_CWE122) || defined(TEST_CWE126)
    #define RAND32() 30
    #define fgets(buf, len, file) ({(buf)[0] = '3'; (buf)[1] = '0'; (buf)[2] = '\n'; (buf)[3] = '\0'; (buf);})
    #define fscanf(file, fmt, buf) ({*(buf) = 30; 1;})
#elif defined(TEST_CWE124) || defined(TEST_CWE127)
    #define RAND32() -1
    #define fgets(buf, len, file) ({(buf)[0] = '-'; (buf)[1] = '1'; (buf)[2] = '\n'; (buf)[3] = '\0'; (buf);})
    #define fscanf(file, fmt, buf) ({*(buf) = -1; 1;})
#elif defined(TEST_CWE761)
    #define fgets(buf, len, file) ({ \
        (buf)[0] = 'X'; \
        (buf)[1] = 'X'; \
        (buf)[2] = 'X'; \
        (buf)[3] = 'X'; \
        (buf)[4] = 'S'; \
        (buf)[5] = 'S'; \
        (buf)[6] = 'S'; \
        (buf)[7] = '\n'; \
        (buf)[8] = '\0'; \
        (buf); \
    })
    #define getenv(v) "XXXXSSS"
#else
    #define RAND32() ((int)(rand() & 1 ? URAND31() : -URAND31() - 1))
#endif

#define fopen(a, b) ((FILE *)2)
#define fclose(f)

#if 0
    extern void *__asan_malloc(size_t);
    extern void __asan_free(void *p);
    #ifdef malloc
        #undef malloc
    #endif
    #ifdef free
        #undef free
    #endif
    #define malloc(s) __asan_malloc(s)
    #define free(p) __asan_free(p)
#endif


/* rand only returns 15 bits, so we xor 5 calls together to get the full result (11 bits overflow, but that is okay) */
// shifting signed values might overflow and be undefined
#define URAND63() (((uint64_t)rand()<<60) ^ ((uint64_t)rand()<<45) ^ ((uint64_t)rand()<<30) ^ ((uint64_t)rand()<<15) ^ rand())
// choose to produce a positive or a negative number. Note: conditional only evaluates one URAND63
#define RAND64() ((int64_t)(rand() & 1 ? URAND63() : -URAND63() - 1))

/* struct used in some test cases as a custom type */
typedef struct _twoIntsStruct
{
    int intOne;
    int intTwo;
} twoIntsStruct;

#ifdef __cplusplus
extern "C" {
#endif

/* The variables below are declared "const", so a tool should
   be able to identify that reads of these will always return their 
   initialized values. */
extern const int GLOBAL_CONST_TRUE; /* true */
extern const int GLOBAL_CONST_FALSE; /* false */
extern const int GLOBAL_CONST_FIVE; /* 5 */

/* The variables below are not defined as "const", but are never
   assigned any other value, so a tool should be able to identify that
   reads of these will always return their initialized values. */
extern int globalTrue; /* true */
extern int globalFalse; /* false */
extern int globalFive; /* 5 */

#ifdef __cplusplus
}
#endif

#include "std_testcase_io.h"

#endif
