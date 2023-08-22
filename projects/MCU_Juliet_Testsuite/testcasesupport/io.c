#include <inttypes.h> // for PRId64
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include "std_testcase.h"

#ifndef _WIN32
#include <wchar.h>
#endif

void printLine (const char * line)
{
    if(line != NULL) 
    {
        while (*line++ != '\0') __asm volatile("nop");
    }
}

void printWLine (const wchar_t * line)
{
    if(line != NULL) 
    {
        while (*line++ != L'\0') __asm volatile("nop");
    }
}

void printIntLine (int intNumber)
{
    (void)intNumber;
}

void printShortLine (short shortNumber)
{
    (void)shortNumber;
}

void printFloatLine (float floatNumber)
{
    (void)floatNumber;
}

void printLongLine (long longNumber)
{
    (void)longNumber;
}

void printLongLongLine (int64_t longLongIntNumber)
{
    (void)longLongIntNumber;
}

void printSizeTLine (size_t sizeTNumber)
{
    (void)sizeTNumber;
}

void printHexCharLine (char charHex)
{
    (void)charHex;
}

void printWcharLine(wchar_t wideChar) 
{
    (void)wideChar;
}

void printUnsignedLine(unsigned unsignedNumber) 
{
    (void)unsignedNumber;
}

void printHexUnsignedCharLine(unsigned char unsignedCharacter) 
{
    (void)unsignedCharacter;
}

void printDoubleLine(double doubleNumber) 
{
    (void)doubleNumber;
}

void printStructLine (const twoIntsStruct * structTwoIntsStruct)
{
    (void)structTwoIntsStruct->intOne;
    (void)structTwoIntsStruct->intTwo;
}

void printBytesLine(const unsigned char * bytes, size_t numBytes)
{
    size_t i;
    for (i = 0; i < numBytes; ++i)
    {
        (void)bytes[i];
    }
}

/* Decode a string of hex characters into the bytes they represent.  The second
 * parameter specifies the length of the output buffer.  The number of bytes
 * actually written to the output buffer is returned. */
size_t decodeHexChars(unsigned char * bytes, size_t numBytes, const char * hex)
{
    size_t numWritten = 0;

    /* We can't sscanf directly into the byte array since %02x expects a pointer to int,
     * not a pointer to unsigned char.  Also, since we expect an unbroken string of hex
     * characters, we check for that before calling sscanf; otherwise we would get a
     * framing error if there's whitespace in the input string. */
    while (numWritten < numBytes && isxdigit(hex[2 * numWritten]) && isxdigit(hex[2 * numWritten + 1]))
    {
        int byte;
        sscanf(&hex[2 * numWritten], "%02x", &byte);
        bytes[numWritten] = (unsigned char) byte;
        ++numWritten;
    }

    return numWritten;
}

/* Decode a string of hex characters into the bytes they represent.  The second
 * parameter specifies the length of the output buffer.  The number of bytes
 * actually written to the output buffer is returned. */
 size_t decodeHexWChars(unsigned char * bytes, size_t numBytes, const wchar_t * hex)
 {
    size_t numWritten = 0;

    /* We can't swscanf directly into the byte array since %02x expects a pointer to int,
     * not a pointer to unsigned char.  Also, since we expect an unbroken string of hex
     * characters, we check for that before calling swscanf; otherwise we would get a
     * framing error if there's whitespace in the input string. */
    while (numWritten < numBytes && iswxdigit(hex[2 * numWritten]) && iswxdigit(hex[2 * numWritten + 1]))
    {
        int byte;
        swscanf(&hex[2 * numWritten], L"%02x", &byte);
        bytes[numWritten] = (unsigned char) byte;
        ++numWritten;
    }

    return numWritten;
}

/* The two functions always return 1 or 0, so a tool should be able to 
   identify that uses of these functions will always return these values */
int globalReturnsTrue() 
{
    return 1;
}

int globalReturnsFalse() 
{
    return 0;
}

int globalReturnsTrueOrFalse() 
{
#if defined(OMITGOOD)
    return 1;
#elif defined(OMITBAD)
    return 0;
#else
    return (rand() % 2);
#endif
}

/* The variables below are declared "const", so a tool should
   be able to identify that reads of these will always return their 
   initialized values. */
const int GLOBAL_CONST_TRUE = 1; /* true */
const int GLOBAL_CONST_FALSE = 0; /* false */
const int GLOBAL_CONST_FIVE = 5; 

/* The variables below are not defined as "const", but are never
   assigned any other value, so a tool should be able to identify that
   reads of these will always return their initialized values. */
int globalTrue = 1; /* true */
int globalFalse = 0; /* false */
int globalFive = 5; 

/* define a bunch of these as empty functions so that if a test case forgets
   to make their's statically scoped, we'll get a linker error */
void good1() { }
void good2() { }
void good3() { }
void good4() { }
void good5() { }
void good6() { }
void good7() { }
void good8() { }
void good9() { }

/* shouldn't be used, but just in case */
void bad1() { }
void bad2() { }
void bad3() { }
void bad4() { }
void bad5() { }
void bad6() { }
void bad7() { }
void bad8() { }
void bad9() { }

/* define global argc and argv */

#ifdef __cplusplus
extern "C" {
#endif

int globalArgc = 0;
char** globalArgv = NULL;

#ifdef __cplusplus
}
#endif
