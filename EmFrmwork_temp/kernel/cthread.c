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
			if (pMSG == (CMSG*)MSG_STOP)	//收到stop指令后自动退出
			{
				break;
			}
			result=(*pMSG->target->callback)(pMSG);//? 注意是否会有空指针? 所有空指针在用户初始化环节中判断和控制
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
	memmove(&tempparam, pthreadparam, sizeof(CTHREADPARAM)); //将外部地址数据拷贝到临时变量中，用于修改内部的param值
	MyLoopThread->MyQUE = (CQUE*)(MyLoopThread+1); //队列地址自动放在Myloopthread地址后
	CQCreate(MyLoopThread->MyQUE, MyLoopThread->MyQUE+1, (MAX_SYSTEM_LOOPQUE_SIZE-sizeof(CQUE)-sizeof(CLOOPTHREAD))/sizeof(void*)); //队列实际起始地址在线程地址和队列头地址之后
	tempparam.param = (TDPARA)MyLoopThread->MyQUE;
	MyLoopThread->Mythread = CThreadtCreateByParam(Looper, &tempparam);
	return MyLoopThread;
}

// delete Lopper Thread
//? must wait until run over looper
//? 不能在Looper线程自身内关闭，必须在外部关闭
u1 CLooperThreadDel(CLOOPTHREAD* pMyThread)
{
	CQPost(pMyThread->MyQUE, (CMSG*)MSG_STOP); //发送一个临时变量到管道队列会不会有问题? 由于Del函数会等待目标线程结束后再清理其他变量，因此临时变量不会在使用完成前结束
	CQDel(pMyThread->MyQUE);
	CThreadtDel(pMyThread->Mythread);
	return csysmem_free((void*)pMyThread);
}
