/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : test_object.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-02
* --------------------
*  ʵ��ʹ��object��handle��������໥���ʵ��ȵĲ���
*  A B C���������߳� ʵ��A����Ϣ��B,B�ȴ�һ��ʱ�����Ϣ��C,C�ȴ�һ��ʱ�䷢��Ϣ��A
*********************************************************************************************************
*/
#include"..\\kernel\cknl.h"
#include<string.h>
#include<stdio.h>

#define TASK_A 0
#define TASK_B 1
#define TASK_C 2


typedef struct __TESTOBJECT{
	//COBJECT MyObj;
	//CHANDLER   myhandler;
	//CHANDLER* tgthandler;
	//CLOOPTHREAD* pthread;
	COBJ_HANDLE_LOOPTHREAD_DEF(1, 1);
	i4 data;
} TESTOBJECT;

const CTHREADPARAM g_testPara[3] =
{
	15,0,2048,0,
	11,0,2048,0,
	10,0,2048,0,
};
u1 TASK_ADEAL(CMSG* pRMSG)
{
#if 1
	TESTOBJECT* tobj;
	CHANDLER* target_handler;
	CMSG* pSendMSG;
	tobj= MyPointer_GET(pRMSG, TESTOBJECT); //����ȡ��MSG�л�ȡʵ��ģ������ָ��
	target_handler = tgtHandler_GET(tobj,0); //��ģ�������л�ȡ�µ�Ŀ�ĵ�ַ
	tobj->data = tobj->data + MSG_D1_GET(pRMSG, i4) + MSG_D2_GET(pRMSG, i4);
	printf("TASKA data=%d\r\n", tobj->data);
	//printf("TASKB Que=%x\r\n", target_handler->loopthread);
	//printf("TASKB Que=%x\r\n", target_handler->loopthread->MyQUE);

	pSendMSG = CobtainMsgTemp(target_handler, 0);
	CWriteMessage(pSendMSG,1,2);
	//CsendMessage(pSendMSG);
	CsendMessageDelay(pSendMSG, 500);
#endif
	return 0;
}

u1 TASK_BDEAL(CMSG* pRMSG)
{
#if 1
	TESTOBJECT* tobj;
	CHANDLER* target_handler;
	CHANDLER* Myhandler;
	//CMSG* pSendMSG;
	tobj = MyPointer_GET(pRMSG, TESTOBJECT); //����ȡ��MSG�л�ȡʵ��ģ������ָ��
	target_handler = tgtHandler_GET(tobj, 0); //��ģ�������л�ȡ�µ�Ŀ�ĵ�ַ
	Myhandler = MyHandler_GET(tobj, 0);

	tobj->data = tobj->data + MSG_D1_GET(pRMSG, i4) + MSG_D2_GET(pRMSG, i4);
	printf("TASKB data=%d\r\n", tobj->data);
	//printf("TASKB Que=%x\r\n", Myhandler->loopthread->MyQUE);
	/*pSendMSG = CobtainMsgTemp(target_handler, 0);
	if (pSendMSG)
	{
		CWriteMessage(pSendMSG, 3, 4);
		CsendMessage(pSendMSG);
	}*/
#endif
	return 0;
}

u1 TASK_CDEAL(CMSG* pRMSG)
{
#if 1
	TESTOBJECT* tobj;
	CHANDLER* target_handler;
	CMSG* pSendMSG;

	tobj = MyPointer_GET(pRMSG, TESTOBJECT); //����ȡ��MSG�л�ȡʵ��ģ������ָ��
	target_handler = tgtHandler_GET(tobj, 0); //��ģ�������л�ȡ�µ�Ŀ�ĵ�ַ
	tobj->data = tobj->data + MSG_D1_GET(pRMSG, i4) + MSG_D2_GET(pRMSG, i4);
	printf("TASKC data=%d\r\n", tobj->data);

	pSendMSG = CobtainMsgTemp(target_handler, 0);
	if (!pSendMSG)//���MESSAGEΪ�������ֿ��ܣ�1 MSG�������ˣ�2 Ŀ��hangler�쳣
	{
		//CDEBUG_ON(121);
		return 0;
	}
	CWriteMessage(pSendMSG, 5, 6);
	CsendMessage(pSendMSG);
#endif
	return 0;
}

CCBFUNC pfunc[3] =
{ TASK_ADEAL,TASK_BDEAL,TASK_CDEAL };


u1 TestObjInit(TESTOBJECT* tObj, u4 ObjID, const  CTHREADPARAM* ThreadPara, CCBFUNC pfunc)
{
	MyObj_Init(tObj, ObjID);
	//CObjectInit(&tObj->MyObj, ObjID, tObj);		//��ʼ��OBJECT
	MyLoopThread_Init(tObj, ThreadPara);
	//tObj->pthread= CLooperThreadCreate(ThreadPara); //��ʼ���߳�
    MyHandler_Init_MyThread(tObj,0,ObjID,pfunc);
	//ChandlerInit(&tObj->myhandler, ObjID, pfunc, tObj->pthread, &tObj->MyObj);//��ʼ��Handler
	MyHandler_InsertSys(tObj,0); //��handler���뵽ϵͳ��ѯ����
	tgtHandler_Init(tObj, 0); //Ŀ��handler��ʼ��Ϊ0

	tObj->data = 0;

	return 0;
}

TESTOBJECT* TestObjCreate(u4 ObjID,const CTHREADPARAM* ThreadPara, CCBFUNC pfunc)
{
	TESTOBJECT* pTemp;
	pTemp = cmem_alloc(sizeof(TESTOBJECT));
	TestObjInit(pTemp, ObjID, ThreadPara, pfunc);
	return pTemp;
}


u1 TestObjDel(TESTOBJECT* pTest)
{
	MyHhandler_DEL(pTest,0);
	//ChandlerClose(&pTest->myhandler); //�ر�hanlder
	//ChandlerDelPtr(&pTest->myhandler); //ɾ��handler����
	MyObjReleaseReq(pTest);
	//CObjectReleaseReq(&pTest->MyObj); //�ͷ�Object;
	MyThread_Del(pTest);
	//CLooperThreadDel(pTest->pthread);//�رճ����߳�
	cmem_free(pTest); //�ͷ��ڴ�
	return 0;
}



void test_object(void)
{
	u4 i,t;
	//CLOOPTHREAD* pthread;
	CMSG* pSendMSG[2];
	TESTOBJECT*  ptestobj[3];
	//��ʼ������
#if 1
	t = cmem_GetUsedMem();
	printf("Start TESTOBJ mem used=%d\r\n", t);
	t = csysmem_GetUsedMem();
	printf("Start TESTOBJ sysmem used=%d\r\n", t);
	t = CMSG_GetUsedMem();
	printf("Start TESTOBJ message used=%d\r\n", t);
	t = Chandler_GetUsedMem();
	printf("Start TESTOBJ handler used=%d\r\n", t);
	t = CTimer_GetUsedMem();
	printf("Start TESTOBJ Timer used=%d\r\n", t);
#endif
	for (i = 0; i < 3; i++)
	{
		ptestobj[i]=TestObjCreate(i, &g_testPara[i], pfunc[i]);
	}
	//�趨Ŀ�ĵ�ַ
	//ChandlerConect(ptestobj[0]->tgthandler, CSysLinkDataFind(1));
	//ChandlerConect(ptestobj[1]->tgthandler, CSysLinkDataFind(2));

	tgtHandler_Conect_Temp(ptestobj[0], 0, MyHandler_GET(ptestobj[1],0));
	tgtHandler_Conect_Temp(ptestobj[1], 0, MyHandler_GET(ptestobj[2], 0));

	//ptestobj[0]->tgthandler = &ptestobj[1]->myhandler;
	//ptestobj[1]->tgthandler = &ptestobj[2]->myhandler;
	//ptestobj[2]->tgthandler = NULL;

	//ChandlerConect(ptestobj[0]->tgthandler, &ptestobj[1]->myhandler);
	//ChandlerConect(ptestobj[1]->tgthandler, &ptestobj[2]->myhandler);
	//ChandlerConect(ptestobj[2]->tgthandler, NULL);

	//�շ���Ϣ
	pSendMSG[0] = CobtainMsgStatic(MyHandler_GET(ptestobj[0], 0), 0);
	pSendMSG[1] = CobtainMsgStatic(MyHandler_GET(ptestobj[2], 0), 0);

	CWriteMessage(pSendMSG[0], 0, 1);
	CWriteMessage(pSendMSG[1], 3, 4);

	t = 0;
	/*while (t<300)
	{
		for (i = 0; i < 1000000; i++)
		{
			;
		}
		CsendMessage(pSendMSG[0]);
		CsendMessage(pSendMSG[1]);
		t++;
	}*/
	
	CsendMessageTiming(pSendMSG[0], 100);
	CsendMessageTiming(pSendMSG[1], 100);

#if 1
	while (1)
	{
		if (ptestobj[0]->data > 3)
		{
			CdelMessageTiming(pSendMSG[0]);
			CdelMessageTiming(pSendMSG[1]);
			break;
		}
		else if (ptestobj[0]->data > 15)
		{
			CctrlMessageTiming(pSendMSG[1], CTIMER_OPERATOR_RUN);
		}
		else if(ptestobj[0]->data > 10)
		{
			CctrlMessageTiming(pSendMSG[1], CTIMER_OPERATOR_STOP);
		}

	}
	t = cmem_GetUsedMem();
	printf("working TESTOBJ mem used=%d\r\n", t);
	t = csysmem_GetUsedMem();
	printf("working TESTOBJ sysmem used=%d\r\n", t);
	t = CMSG_GetUsedMem();
	printf("working TESTOBJ message used=%d\r\n", t);
	t = Chandler_GetUsedMem();
	printf("working TESTOBJ handler used=%d\r\n", t);
	t = CTimer_GetUsedMem();
	printf("Start TESTOBJ Timer used=%d\r\n", t);


	CrecycleMessage(pSendMSG[0]);
	CrecycleMessage(pSendMSG[1]);
#if 1
	//������ɺ�ɾ������ģ��
	for (i = 0; i < 3; i++)
	{
		CDEBUG_ON(TestObjDel(ptestobj[i]));
	}
#endif
	t = cmem_GetUsedMem();
	printf("End TESTOBJ mem used=%d\r\n", t);
	t = csysmem_GetUsedMem();
	printf("End TESTOBJ sysmem used=%d\r\n", t);
	t = CMSG_GetUsedMem();
	printf("End TESTOBJ message used=%d\r\n", t);
	t = Chandler_GetUsedMem();
	printf("End TESTOBJ handler used=%d\r\n", t);
	t = CTimer_GetUsedMem();
	printf("Start TESTOBJ Timer used=%d\r\n", t);
	CDEBUG_ON(5);

#endif
}
