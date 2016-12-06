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

typedef struct __Stestlink
{
	u4					ObjID;				//描述的模块唯一标识，包括类型,ID等
	u4					data;
}Stestlink;


typedef struct __TestSimplelink
{
	u4					data;
	void**				next;				//描述的模块唯一标识，包括类型,ID等
}TestSimplelink;

#define MAX_TEST_LINK_NUM 16

u1 g_testlink[CLINKLIST_SIZE(MAX_TEST_LINK_NUM,sizeof(Stestlink))];


TestSimplelink g_testsimpleLink[MAX_TEST_LINK_NUM];

u1	CTestIDcompare(const void * ObjID, const void * handler)
{
	return (u4)ObjID == ((CHANDLER*)handler)->ObjID;
}

u1 TestListTraverse(Stestlink * plinkdata)
{
	printf("ListTraverse ID=%d\r\n",plinkdata->ObjID);
	return 0;
}


u1 TestSimpleListTraverse(TestSimplelink * plinkdata)
{
	printf("SimpListTraverse ID=%d\r\n", plinkdata->data);
	return 0;
}

void test_linklist_t(u4 t)
{
#define TestTimes 10
	u2 err,i;
	u4 data;
	void* ptr;
	Stestlink tempdata;
	Stestlink* pdata;

	printf("Start Linklist Insert...\r\n");
	for (i = 0; i < TestTimes; i++)
	{
		tempdata.ObjID = t*100+i;
		tempdata.data = i*i;
		ptr = CListAdd(g_testlink, &tempdata, sizeof(Stestlink), 2); //CLIST_HEAD
		if (ptr)
		{
			data = GetListNum(g_testlink);
			printf("LinkList Num=%d\r\n", data);
			data = GetListIdleNum(g_testlink);
			printf("LinkListIdle Num=%d\r\n", data);
		}
		else
		{
			printf("LinkList errcode = List addr error\r\n");
		}
	}

	printf("Start Linklist Find...\r\n");
	for (i = 0; i <TestTimes; i++)
	{
		data = t * 100 + TestTimes - i - 1;
		pdata = CListFind(g_testlink, (DPARA)data, CTestIDcompare);
		if(pdata)
		printf("LinkList ID=%d, data=%d\r\n", pdata->ObjID, pdata->data);
	}

	printf("Start Linklist Traverse...\r\n");
	CListTraverse(g_testlink, TestListTraverse);

	printf("Start Linklist Remove...\r\n");
	for (i = 0; i < TestTimes-1; i++)
	{
		data = t * 100 + (i + 5) % TestTimes;
		err = CListRemove(g_testlink, (DPARA)data, CTestIDcompare);
		if (!err)
		{
			data = GetListNum(g_testlink);
			printf("LinkList Num=%d\r\n", data);
			data = GetListIdleNum(g_testlink);
			printf("LinkListIdle Num=%d\r\n", data);

		}
		else
		{
			printf("LinkList errcode=%d\r\n", err);
		}
	}
}
void test_linklist_t2(u4 t)
{
	u4 i,data,err;
	TestSimplelink* plist;
	TestSimplelink* ListHeadAddr;
	printf("Start simpleLinklist Insert...\r\n");
	plist = g_testsimpleLink;
	CSimpleListInit(&ListHeadAddr);
	for (i = 0; i < TestTimes; i++)
	{
		plist->data=t * 100 + i;
		CSimpleListAddTail(&ListHeadAddr,plist, offsetof(TestSimplelink, next));

		data = GetSimpleListNum(&ListHeadAddr, offsetof(TestSimplelink, next));
		printf("simpleLinkList Num=%d\r\n", data);

		plist++;
	}

	printf("Start simpleLinklist Next...\r\n");
	plist = ListHeadAddr;
	for (i = 0; i <TestTimes; i++)
	{
		if (plist)
		{
			plist = CSimpleListNext(plist, offsetof(TestSimplelink, next));
			if (plist)
			{
				printf("simpleLinkList Index=%d, data=%d\r\n", i, plist->data);
			}
			else
			{
				printf("simpleLinkList errcode=%d\r\n", 2);
			}
		}
		else
		{
			printf("simpleLinkList errcode=%d\r\n", 1);
		}
	}

	printf("Start simpleLinklist Traverse...\r\n");
	CSimpleListTraverse(&ListHeadAddr, offsetof(TestSimplelink, next), TestSimpleListTraverse);

	printf("Start simpleLinklist Remove...\r\n");
	//plist = g_testsimpleLink;
	for (i = 0; i < TestTimes; i++)
	{
		plist = &g_testsimpleLink[(i + 5) % TestTimes];

		err = CSimpleListRemovePtr(&ListHeadAddr, plist,offsetof(TestSimplelink, next));
		if (!err)
		{
			data = GetSimpleListNum(&ListHeadAddr, offsetof(TestSimplelink, next));
			printf("simpleLinklist Num=%d\r\n", data);
		}
		else
		{
			printf("LinkList errcode=%d\r\n", err);
		}
	}

}

void test_linklist(void)
{
	u4 i;
	CListCreate(g_testlink, MAX_TEST_LINK_NUM, sizeof(Stestlink));
	for (i = 0; i < 1; i++)
	{
		//test_linklist_t(i);
		test_linklist_t2(i);
	}
}