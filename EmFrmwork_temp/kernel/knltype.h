/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : knltype.H
* By      : wangchao
* Version : V0.01
* Date    : 2016-09-14
* --------------------
*  kernel types define by different os
*********************************************************************************************************
*/
#ifndef KNLTYPE_H
#define KNLTYPE_H
/*
*********************************************************************************************************
*                                              OS TYPES
*********************************************************************************************************
*/

//#define UCOS

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

#ifndef WIN32
typedef int  BOOL;

#ifndef TRUE
#define TRUE		   1
#endif

#ifndef FALSE
#define FALSE		   0
#endif

#ifndef NULL
#define NULL		   (void*)0
#endif

#endif
typedef unsigned char  u1;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  i1;                    /* Signed    8 bit quantity                           */
typedef unsigned short u2;                   /* Unsigned 16 bit quantity                           */
typedef signed   short i2;                   /* Signed   16 bit quantity                           */
typedef unsigned int   u4;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   i4;                   /* Signed   32 bit quantity                           */
typedef float          f4;                   /* Single precision floating point                    */
typedef double         f8;                   /* Double precision floating point                    */
typedef void*          MPARA;                /* kernel  MSG para type				               */
typedef void*          DPARA;                /* kernel  General data para type				        */

#endif /* KNLTYPE_H */