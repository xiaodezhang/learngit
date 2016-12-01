/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : testcase.H
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-02
* --------------------
*  testcase for cknl
*********************************************************************************************************
*/
#ifndef __TESTCASE_H__
#define __TESTCASE_H__

#include"..\\kernel\cknl.h"

#ifdef __cplusplus
extern "C" {
#endif
extern void testcmemory(void);
extern void test_linklist(void);
extern void test_object(void);
extern void test_debug(void);
#ifdef __cplusplus
}
#endif
#endif