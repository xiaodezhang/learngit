/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cqueue.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-08
* --------------------
*  a queue used by LooperThread  
*********************************************************************************************************
*/
#include"cknl.h"


// Init a Queue Infomation
u1 CQCreate(CQUE* pQUE,void  *Qstart, u2 size)
{
	pQUE->size		= size;
	pQUE->num		= 0;
	pQUE->QStart	=  (void**)Qstart;
	pQUE->QEnd		=  pQUE->QStart + size;
	pQUE->Qin		= pQUE->QStart;
	pQUE->Qout		= pQUE->QStart;
	pQUE->hSEM		= CSemCreate(0);
	return CKNL_ERROR_NONE;
}

// Delete a Queue Infomation
u1 CQDel(CQUE* pQUE)
{
	CSemDel(pQUE->hSEM);
	return CKNL_ERROR_NONE;
}

//Pend Message from Queue
void  *CQPend(CQUE* pQUE)
{
	void *pmsg;
	CSemPend(pQUE->hSEM); //wait for SEM 
	CKNL_ENTER_CRITICAL();
	if (pQUE->num > 0u)
	{
		pmsg = *pQUE->Qout++;
		pQUE->num--;
		if (pQUE->Qout == pQUE->QEnd) {
			pQUE->Qout = pQUE->QStart;
		}
	}
	else
	{
		pmsg = (void*)0;
	}
	CKNL_EXIT_CRITICAL();
	return (pmsg);                                    /* Return received message */
}

//Post Message to Queue
u1  CQPost(CQUE* pQUE, void *pmsg)
{
	CKNL_ENTER_CRITICAL();
	if (pQUE->num < pQUE->size)
	{
		*pQUE->Qin++ = pmsg;
		pQUE->num++;
		if (pQUE->Qin == pQUE->QEnd) {
			pQUE->Qin = pQUE->QStart;
		}
		CKNL_EXIT_CRITICAL();
		CSemPost(pQUE->hSEM); //post a SEM 
		return CKNL_ERROR_NONE;
	}
	CKNL_EXIT_CRITICAL();
	return CDEBUG_ON(CQUEUE_FULL);
}
