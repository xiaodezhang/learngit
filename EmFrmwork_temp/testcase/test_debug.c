/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : test_debug.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-11-08
* --------------------
*  用于测试cdebug模块各个函数的性能
*********************************************************************************************************
*/
#include"..\\kernel\cknl.h"
#include<string.h>
#include<stdio.h>



void test_debug(void)
{
	u1 a = 0;
	u1 i = 0;
	for (i = 0; i < 3; i++)
	{
		CDebugSetIOMODE(i);
		for (a = 0; a < 10; a++)
		{
			CDebugPrintf("printf test data=%d,%d\r\n", a, a + 1);
			CDEBUG_ON(a);
		}
	}

}