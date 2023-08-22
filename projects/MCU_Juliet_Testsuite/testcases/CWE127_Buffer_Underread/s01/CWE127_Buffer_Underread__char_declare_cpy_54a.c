/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE127_Buffer_Underread__char_declare_cpy_54a.c
Label Definition File: CWE127_Buffer_Underread.stack.label.xml
Template File: sources-sink-54a.tmpl.c
*/
/*
 * @description
 * CWE: 127 Buffer Under-read
 * BadSource:  Set data pointer to before the allocated memory buffer
 * GoodSource: Set data pointer to the allocated memory buffer
 * Sink: cpy
 *    BadSink : Copy data to string using strcpy
 * Flow Variant: 54 Data flow: data passed as an argument from one function through three others to a fifth; all five functions are in different source files
 *
 * */

#include "std_testcase.h"

#include <wchar.h>

#ifndef OMITBAD

/* bad function declaration */
void CWE127_Buffer_Underread__char_declare_cpy_54b_badSink(char * data);

void CWE127_Buffer_Underread__char_declare_cpy_54_bad()
{
    char * data;
    char dataBuffer[100];
    memset(dataBuffer, 'A', 100-1);
    dataBuffer[100-1] = '\0';
    /* FLAW: Set data pointer to before the allocated memory buffer */
    data = dataBuffer - 8;
    CWE127_Buffer_Underread__char_declare_cpy_54b_badSink(data);
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* good function declaration */
void CWE127_Buffer_Underread__char_declare_cpy_54b_goodG2BSink(char * data);

/* goodG2B uses the GoodSource with the BadSink */
static void goodG2B()
{
    char * data;
    char dataBuffer[100];
    memset(dataBuffer, 'A', 100-1);
    dataBuffer[100-1] = '\0';
    /* FIX: Set data pointer to the allocated memory buffer */
    data = dataBuffer;
    CWE127_Buffer_Underread__char_declare_cpy_54b_goodG2BSink(data);
}

void CWE127_Buffer_Underread__char_declare_cpy_54_good()
{
    goodG2B();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
 * its own for testing or for building a binary to use in testing binary
 * analysis tools. It is not used when compiling all the testcases as one
 * application, which is how source code analysis tools are tested.
 */

#ifdef INCLUDEMAIN

int main(int argc, char * argv[])
{
    /* seed randomness */
    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    printLine("Calling good()...");
    CWE127_Buffer_Underread__char_declare_cpy_54_good();
    printLine("Finished good()");
#endif /* OMITGOOD */
#ifndef OMITBAD
    printLine("Calling bad()...");
    CWE127_Buffer_Underread__char_declare_cpy_54_bad();
    printLine("Finished bad()");
#endif /* OMITBAD */
    return 0;
}

#endif
