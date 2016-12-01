/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : ctimer.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-27
* --------------------
*  timer for MSG post in timing
*********************************************************************************************************
*/
#include"cknl.h"

// Init TimerGroup
u1 CTimerGroupInit(void* timeraddr, u4 timernum)
{
	u1 err;
	memset(timeraddr, 0, sizeof(CTIMER)*timernum);
	err = ClistPTCreate(timeraddr, timernum, sizeof(CTIMER), 0);
	return err;
}

// Init TimerWheelGroup
u1 CTimerWheelInit(void* tmrwheeladdr)
{
	CTIMER_WHEEL_GROUP* tmrwheelgroup;
	u2 i;
	memset(tmrwheeladdr, 0, sizeof(CTIMER_WHEEL_GROUP));
	tmrwheelgroup = (CTIMER_WHEEL_GROUP*)tmrwheeladdr;
	CMutexCreate(tmrwheelgroup->lock);
	for (i = 0; i < CTIMER_WHEEL_SIZE; i++)
	{//初始化将所有时间轮的首位指针指向自己
		tmrwheelgroup->wheel[i].first = (CTIMER*)(tmrwheelgroup->wheel+i);
		tmrwheelgroup->wheel[i].tail  = (CTIMER*)(tmrwheelgroup->wheel+i);
	}
	return 0;
}

// create a timer from timergroup
CTIMER* CTimerCreate(void* timeraddr, CTimerFunc pfunc, void* data, u4 period, u1 opt)
{
	CTIMER* tmr;
	tmr = ClistPTGetBlock(timeraddr);
	if (!tmr)
	{
		return NULL;
	}
	tmr->tmraddr = timeraddr;
	tmr->pfunc = pfunc;
	tmr->data = data;
	tmr->period = period;
	tmr->opt = opt;
	tmr->MatchTime = 0;
	tmr->next = NULL;
	return tmr;
}

// delete a timer to timergroup
u1 __inline __CTimerDel(CTIMER*ptmr)
{
	if (ptmr->tmraddr)
	{
		return ClistPTPutBlock(ptmr->tmraddr, ptmr);
	}
	else
	{
		return 0;
	}
}

//control timer state by opt
u1  CTimerCtrl(CTIMER*ptmr,u1 opt)
{
	ptmr->opt = opt;
	return 0;
}




void __inline __CTimerLink(CTIMER*ptmr, CTIMER_WHEEL*tmrwheel, u4 cTmrTime)
{
	CTIMER_WHEEL* pspoke;
	ptmr->MatchTime = ptmr->period + cTmrTime;
	pspoke = tmrwheel + (ptmr->MatchTime % CTIMER_WHEEL_SIZE);

	ptmr->next = (CTIMER*)pspoke;
	ptmr->prev = pspoke->tail;
	pspoke->tail->next = ptmr;
	pspoke->tail = ptmr;
}



void __inline __CTimerUnLink(CTIMER*ptmr)
{
	ptmr->prev->next=ptmr->next;
	ptmr->next->prev= ptmr->prev;
}

// Timer Runing function in Task or in interrupt(select by a macro CTIMER_TICK_INTASK)
void  CTimerTick(CTIMER_WHEEL_GROUP* ptmr_wheel_group)
{
	u1 result;
	u4 currentTime;
	CTIMER_WHEEL* pspoke;
	CTIMER_WHEEL* pwheel;

	CTIMER*ptmr;
	CTIMER*ptmr_next;

		pwheel = ptmr_wheel_group->wheel;


#if (CTIMER_TICK_IN_MUTEX)
		CMutexLock(ptmr_wheel_group->lock);
#endif
		currentTime = ++ptmr_wheel_group->currentTime;						 /* Increment the current time                        */
		pspoke = pwheel + (currentTime % CTIMER_WHEEL_SIZE);

		ptmr = pspoke->first;
		while (ptmr != (CTIMER*)pspoke)//循环链表 头和尾部都指向时间轮本身时表示为空
		{
			ptmr_next = ptmr->next;								 /* Point to next timer to update because current ... */
			if (currentTime == ptmr->MatchTime)
			{
				__CTimerUnLink(ptmr);
				//ptmr_prev->next = ptmr->next;
				if (ptmr->pfunc&&(ptmr->opt&CTIMER_MASK_SENDMSG))
				{
					result=(*ptmr->pfunc)(ptmr->data);
				}

				if (ptmr->opt&CTIMER_MASK_LINK)__CTimerLink(ptmr, pwheel, currentTime);
				else if (ptmr->opt&CTIMER_MASK_DEL)__CTimerDel(ptmr);
			}	/* ... timer could get unlinked from the wheel.      */
			ptmr = ptmr_next;
		}
#if  (CTIMER_TICK_IN_MUTEX) 
		CMutexUnlock(ptmr_wheel_group->lock);
#endif
}

//Ctimer link used by user
void CTimerUserLink(CTIMER*ptmr, CTIMER_WHEEL_GROUP* ptmr_wheel_group)
{
	CMutexLock(ptmr_wheel_group->lock);
	__CTimerLink(ptmr, ptmr_wheel_group->wheel, ptmr_wheel_group->currentTime);
	CMutexUnlock(ptmr_wheel_group->lock);
}

//Ctimer unlink used by user
void CTimerUserUnlinkDel(CTIMER*ptmr, CTIMER_WHEEL_GROUP* ptmr_wheel_group)
{
	CMutexLock(ptmr_wheel_group->lock);
	__CTimerUnLink(ptmr);
	CMutexUnlock(ptmr_wheel_group->lock);
	__CTimerDel(ptmr);
}


#ifdef WIN32
static void CALLBACK CtimerRunWin(LPVOID lpParam, BOOLEAN  bTimerOrWaitFired)
{
	CTimerTick((TDPARA)lpParam);
}
#endif

//初始化系统事件定时器
void CsystimerInit(void)
{
#ifdef WIN32
	HANDLE hTimer;
	HANDLE hTimerQueue;
	hTimerQueue = CreateTimerQueue();
	if (hTimerQueue)
	{
		CreateTimerQueueTimer(&hTimer, hTimerQueue, CtimerRunWin, GetSysTimerWheel(), CTIMER_TICK_PERIOD, CTIMER_TICK_PERIOD,0);
	}
#endif
}