/*********************************************************************
*              SEGGER MICROCONTROLLER SYSTEME GmbH                   *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2002-2005 SEGGER Microcontroller Systeme GmbH           *
*                                                                    *
* Internet: www.segger.com Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : main.h
Purpose : Jlink main include
---------------------------END-OF-HEADER------------------------------
*/

#ifndef MAIN_H       // Avoid multiple inclusion
#define MAIN_H

#include "GLOBAL.h"

/*********************************************************************
*
*       Defines, function replacements
*
**********************************************************************
*/
#ifndef   COUNTOF
  #define COUNTOF(a)    (int)(sizeof(a)/sizeof(a[0]))
#endif
#ifndef   ZEROFILL
  #define ZEROFILL(Obj) memset(&Obj, 0, sizeof(Obj))
#endif
#ifndef   LIMIT
  #define LIMIT(a,b)    if ((a) > (b)) (a) = (b);
#endif
#ifndef   MIN
  #define MIN(a, b)     (((a) < (b)) ? (a) : (b))
#endif
#ifndef   MAX
  #define MAX(a, b)     (((a) > (b)) ? (a) : (b))
#endif

/*********************************************************************
*
*       TRACE
*
**********************************************************************
*/

void TRACE_ShowRegions(void);
void RAWTRACE_Exec(void);

#endif               // Avoid multiple inclusion

/*************************** end of file ****************************/
