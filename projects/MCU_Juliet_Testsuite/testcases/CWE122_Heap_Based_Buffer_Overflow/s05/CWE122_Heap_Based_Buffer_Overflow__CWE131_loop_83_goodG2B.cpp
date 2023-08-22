/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83_goodG2B.cpp
Label Definition File: CWE122_Heap_Based_Buffer_Overflow__CWE131.label.xml
Template File: sources-sink-83_goodG2B.tmpl.cpp
*/
/*
 * @description
 * CWE: 122 Heap Based Buffer Overflow
 * BadSource:  Allocate memory without using sizeof(int)
 * GoodSource: Allocate memory using sizeof(int)
 * Sinks: loop
 *    BadSink : Copy array to data using a loop
 * Flow Variant: 83 Data flow: data passed to class constructor and destructor by declaring the class object on the stack
 *
 * */
#ifndef OMITGOOD

#include "std_testcase.h"
#include "CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83.h"

namespace CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83
{
CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83_goodG2B::CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83_goodG2B(int * dataCopy)
{
    data = dataCopy;
    /* FIX: Allocate memory using sizeof(int) */
    data = (int *)malloc(10*sizeof(int));
    if (data == NULL) {exit(-1);}
}

CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83_goodG2B::~CWE122_Heap_Based_Buffer_Overflow__CWE131_loop_83_goodG2B()
{
    {
        int source[10] = {0};
        size_t i;
        /* POTENTIAL FLAW: Possible buffer overflow if data was not allocated correctly in the source */
        for (i = 0; i < 10; i++)
        {
            data[i] = source[i];
        }
        printIntLine(data[0]);
        free(data);
    }
}
}
#endif /* OMITGOOD */
