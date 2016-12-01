/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cmemory.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-02
* --------------------
*  memory config functions for cknl
*********************************************************************************************************
*/
#include"..\\kernel\cknl.h"
#include<string.h>
#include<stdio.h>

void testcmemory(void)
{
#define testTimes  10
	u2 fastmem[testTimes];
	u2 slowmem[testTimes];
	u2 i,err;
	u1* testmemfast[testTimes];
	u1* testmemslow[testTimes];
	for (i = 0; i < testTimes; i++)
	{
		fastmem[i] = 1024;
		slowmem[i] = 1024 * i;
		testmemfast[i] = (u1*)cmem_alloc(fastmem[i]);
		if (testmemfast[i] != NULL)
		{
			memset(testmemfast[i], i+16, fastmem[i]);
			printf("fastMEM%d,startAddr%x,endAddr%x\r\n", i, (u4)testmemfast[i], (u4)(testmemfast[i]+ fastmem[i]));
		}
		testmemslow[i] = (u1*)cmem_alloc(slowmem[i]);
		if (testmemslow[i] != NULL)
		{
			memset(testmemslow[i], i+32, slowmem[i]);
			printf("slowMEM%d,startAddr%x,endAddr%x\r\n", i, (u4)testmemslow[i], (u4)(testmemslow[i] + slowmem[i]));
		}
		
	}
	for (i = 0; i < testTimes; i++)
	{
		err=cmem_free(testmemfast[testTimes-1-i]);
		if(err)
		{
			printf("fastMEM,errBy %d\r\n", err);
		}
		err=cmem_free(testmemslow[(testTimes - 1 - i+5)% testTimes]);
		if (err)
		{
			printf("slowMEM,errBy %d\r\n", err);
		}
	}
}