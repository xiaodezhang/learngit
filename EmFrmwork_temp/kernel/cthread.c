/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cobject.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-08
* --------------------
*  Thread and LooperThread generate and Loop 
*********************************************************************************************************
*/
#include"cknl.h"
#include <string.h>

// Loop
TDOUT Looper(TDPARA para)
{
	CQUE* pQUE = (CQUE*)para;
	CMSG  *pMSG;
	u1 result;
	for (;;)
	{
		pMSG = (CMSG*)CQPend(pQUE);
		if (pMSG)
		{
			if (pMSG == (CMSG*)MSG_STOP)	//�յ�stopָ����Զ��˳�
			{
				break;
			}
			result=(*pMSG->target->callback)(pMSG);//? ע���Ƿ���п�ָ��? ���п�ָ�����û���ʼ���������жϺͿ���
			if (pMSG->recyleEn)
				CrecycleMessage(pMSG);
		}
	}	
	return 0;
}


thread_t CThreadtCreateByParam(TDFUNC pfunc,const CTHREADPARAM* pthreadparam)
{
	thread_t pthread;
#ifdef UCOS
	stackaddr=csysmem_alloc(pthreadparam->stacksize);
	CThreadCreate_t(pfunc, pthreadparam);
#else
	pthread=CThreadCreate_t(pfunc, pthreadparam);
	CSetThreadPriority(pthread, pthreadparam->prio);
#endif
	return pthread;
}

u1 CThreadtDel(thread_t pthread)
{
#ifdef UCOS
	csysmem_free(stackaddr);
#else
	 CThreadDel_t(pthread);
#endif
	 return 0;
}

CTHREAD CThreadCreate(TDFUNC pfunc, const CTHREADPARAM* pthreadparam)
{
	CTHREAD tempThread;
	tempThread.Mythread=CThreadtCreateByParam(pfunc, pthreadparam);
	return tempThread;
}

u1 CThreadDel(CTHREAD* pthread)
{
	CThreadtDel(pthread->Mythread);
	return 0;
}

// Create Lopper Thread
CLOOPTHREAD* CLooperThreadCreate(const CTHREADPARAM* pthreadparam)
{
	CTHREADPARAM  tempparam;
	CLOOPTHREAD* MyLoopThread;
	if ((MAX_SYSTEM_LOOPQUE_SIZE <= sizeof(CQUE) + sizeof(CLOOPTHREAD)))
	{
		CDEBUG_ON(DATASIZE_OVERFLOW);
		return NULL;
	}
	MyLoopThread	   = csysmem_alloc(MAX_SYSTEM_LOOPQUE_SIZE);
	if (!MyLoopThread)
	{
		return NULL;
	}
	memmove(&tempparam, pthreadparam, sizeof(CTHREADPARAM)); //���ⲿ��ַ���ݿ�������ʱ�����У������޸��ڲ���paramֵ
	MyLoopThread->MyQUE = (CQUE*)(MyLoopThread+1); //���е�ַ�Զ�����Myloopthread��ַ��
	CQCreate(MyLoopThread->MyQUE, MyLoopThread->MyQUE+1, (MAX_SYSTEM_LOOPQUE_SIZE-sizeof(CQUE)-sizeof(CLOOPTHREAD))/sizeof(void*)); //����ʵ����ʼ��ַ���̵߳�ַ�Ͷ���ͷ��ַ֮��
	tempparam.param = (TDPARA)MyLoopThread->MyQUE;
	MyLoopThread->Mythread = CThreadtCreateByParam(Looper, &tempparam);
	return MyLoopThread;
}

// delete Lopper Thread
//? must wait until run over looper
//? ������Looper�߳������ڹرգ��������ⲿ�ر�
u1 CLooperThreadDel(CLOOPTHREAD* pMyThread)
{
	CQPost(pMyThread->MyQUE, (CMSG*)MSG_STOP); //����һ����ʱ�������ܵ����л᲻��������? ����Del������ȴ�Ŀ���߳̽��������������������������ʱ����������ʹ�����ǰ����
	CQDel(pMyThread->MyQUE);
	CThreadtDel(pMyThread->Mythread);
	return csysmem_free((void*)pMyThread);
}
