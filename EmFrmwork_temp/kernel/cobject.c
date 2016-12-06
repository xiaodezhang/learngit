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
*  Cobject include handler\Message\Obj
*********************************************************************************************************
*/
#include"cknl.h"

/*
*********************************************************************************************************
*                                             COBJECT
*********************************************************************************************************
*/
u1 CObjectInit(COBJECT* pObj, u4 ObjID,void* pointer)
{
	pObj->ObjID = ObjID;
	pObj->refcnt = 1;
	pObj->pointer = pointer;
	//pObj->LinkAddr = LinkAddr;
	//pObj->RelaseFunc = RelaseFunc;
	pObj->ReleaseSem = NULL;
	return CKNL_ERROR_NONE;
}

u1 CObjectReleaseReq(COBJECT* pObj)
{
	if (pObj->refcnt)
	{
		pObj->refcnt--;
		if (pObj->refcnt)
		{
			pObj->ReleaseSem = CSemCreate(0);
			CSemPend(pObj->ReleaseSem);
			CSemDel(pObj->ReleaseSem); //信号量处理完成后删除之
		}
	}
	return 0;
}


u1 CObjectReleaseDeal(COBJECT* pObj)
{
	if (pObj->refcnt)
	{
		pObj->refcnt--;
		if (!pObj->refcnt&&pObj->ReleaseSem)
		{
			CSemPost(pObj->ReleaseSem);//处理完成后发送信号量
		}
	}
	return 0;
}
/*
u1 CObjectDel(COBJECT* pObj)
{
CObjectRealseFuncDeal(pObj);
return 0;
}
*/
/*
*********************************************************************************************************
*                                             CHANDLE
*********************************************************************************************************
*/
// Loop
u1 ChandleLinkCreate(void* addr, u4 maxnum)
{
	return CListCreate(addr, maxnum, sizeof(CHANDLER*));
}

u1 ChandlerInit(CHANDLER* phanlder, u4 ObjID, CCBFUNC callback, CLOOPTHREAD* ploopthread, COBJECT* pObj)
{

	phanlder->ObjID		= ObjID;
	phanlder->callback	= callback;
	phanlder->loopthread = ploopthread;
	phanlder->pObj		= pObj;
	phanlder->Reserved1 = 0;
	phanlder->Reserved2 = 0;
	phanlder->LinkAddr = NULL;
	phanlder->working	= 1;
	if (!callback || !ploopthread || !pObj) //不允许callback为0或thread为0或pObj为0
	{
		return CDEBUG_ON(CHANLDR_CREATE_ERR);
	}
	return CKNL_ERROR_NONE;
}

u1 ChandlerOpen(CHANDLER* phanlder)
{
	if (!phanlder)
	{
		return CDEBUG_ON(CHANLDR_ERR);
	}
	phanlder->working = 1;

	return CKNL_ERROR_NONE;
}

u1 ChandlerClose(CHANDLER* phanlder)
{
	if (!phanlder)
	{
		return CDEBUG_ON(CHANLDR_ERR);
	}
	phanlder->working = 0;

	return CKNL_ERROR_NONE;
}

CHANDLER* ChandlerConect(CHANDLER* target,u1 staticmode)
{
	if (!target|| !target->working||!target->pObj||!target->callback|| !target->loopthread)
	{
		return NULL;
	}
	if (staticmode)
	{
		target->pObj->refcnt++;
	}
	return target;
}

u1 ChandlerDisConect(CHANDLER* phanlder)
{
	if (!phanlder->pObj)
	{
		return CHANLDR_ERR;
	}
	CObjectReleaseDeal(phanlder->pObj);
	return CKNL_ERROR_NONE;
}


//将Object链接到一个链表组中，用于后续查询使用

u1 ChandlerInsert(CHANDLER* phandler, void* linkAddr)
{
	if ((!phandler->LinkAddr)&&CListAdd(linkAddr, &phandler, sizeof(CHANDLER*), CLIST_TAIL))
	{
		phandler->LinkAddr = linkAddr;
		return CKNL_ERROR_NONE;
	}
	else
		return CLIST_ERR;
}

u1	ChandlerIDcompare(const void * ObjID, const CHANDLER** handler)
{
	return (u4)ObjID == (*handler)->ObjID;
}

u1	ChandlerPtrcompare(const void * ObjID, const CHANDLER** handler)
{
	return ObjID == (*handler);
}

CHANDLER* ChandlerFind(void* addr, u4 ObjID)
{
	return CListFind(addr, (void*)ObjID, ChandlerIDcompare);
}

u1 ChandlerDelbyID(void* addr, u4 ObjID)
{
	return CListRemove(addr, (void*)ObjID, ChandlerIDcompare);
}

u1 ChandlerDelPtr(CHANDLER* handler)
{
	u1 err;
	err= CListRemove(handler->LinkAddr, (void*)handler, ChandlerPtrcompare);
	if (!err)
	{
		handler->LinkAddr = NULL;
	}
	return err;
}

/*
*********************************************************************************************************
*                                             CMESSAGE
*********************************************************************************************************
*/

CMSG* CobtainMessage(CHANDLER* handler, i2 what, u1 recyleEn)
{
	CMSG* pMSG;

	if (!handler || !handler->working)
	{
		return NULL;
	}
	pMSG= ClistPTGetBlock(GetSysMsgGroup());
	if (pMSG)
	{
		pMSG->target = handler;
		pMSG->what = what;
		pMSG->recyleEn = recyleEn;
		pMSG->ptmr = NULL;
# if CKNL_OBJ_RELEASE_IN_MSG_EN ==1
		handler->pObj->refcnt++;
#endif
	}
	return pMSG;
}

u1 CrecycleMessage(CMSG* pMSG)
{
# if CKNL_OBJ_RELEASE_IN_MSG_EN ==1
	CObjectReleaseDeal(pMSG->target->pObj);
#endif
	return ClistPTPutBlock(GetSysMsgGroup(), pMSG);
}
/*
u1 CRegHandler(CHANDLER* handler, CCBFUNC callbackfunc, CLOOPTHREAD pthread)
{
	return 0;
}
*/

//发送消息的函数
u1 CsendMessage(CMSG* pMsg)
{
	if (!pMsg->target || !(pMsg->target->working))
	{
		if (pMsg->recyleEn)
			CrecycleMessage(pMsg);
		return CMSG_TARGET_ERR;
	}
	return CQPost(pMsg->target->loopthread->MyQUE, pMsg);
}

//延迟发送消息的函数
u1 CsendMessageDelay(CMSG* pMsg, u4 Delay)
{
	if (!pMsg->target || !(pMsg->target->working))
	{
		if (pMsg->recyleEn)
			CrecycleMessage(pMsg);
		return CMSG_TARGET_ERR;
	}
	pMsg->ptmr =CTimerCreate(GetSysTimerGroup(), CsendMessage, pMsg, Delay, CTIMER_OPERATOR_ONESHOT);
	if (!pMsg->ptmr)
	{
		return CTIMER_CREATE_ERR;
	}
	CTimerUserLink(pMsg->ptmr, GetSysTimerWheel());
	return CKNL_ERROR_NONE;
}

//定时发送消息的函数:不可以使用临时自动回收的MSG,只能使用静态MSG并手动回收
u1 CsendMessageTiming(CMSG* pMsg, u4 period)
{
	if (!pMsg->target || !(pMsg->target->working))
	{
		if (pMsg->recyleEn)
			CrecycleMessage(pMsg);
		return CMSG_TARGET_ERR;
	}
	if (pMsg->recyleEn)
	{
		CrecycleMessage(pMsg);
		return CMSG_ERR_TEMP_TIMING;
	}
	pMsg->ptmr = CTimerCreate(GetSysTimerGroup(), CsendMessage, pMsg, period, CTIMER_OPERATOR_RUN);
	if (!pMsg->ptmr)
	{
		return CTIMER_CREATE_ERR;
	}
	CTimerUserLink(pMsg->ptmr, GetSysTimerWheel());
	return CKNL_ERROR_NONE;
}

//控制定时Message的发送操作，包括暂停、重启、到期后删除等功能
u1 CctrlMessageTiming(CMSG* pMsg, u1 opt)
{
	if (!pMsg->ptmr)
	{
		return CMSG_ERR_NOTIMING;
	}
	CTimerCtrl(pMsg->ptmr, opt);
	return CKNL_ERROR_NONE;
}

//删除Msg的定时功能，注意此处不负责回收MSG，需要手动调用CrecycleMessage回收
//注意如果使用控制功能或延迟发送功能程序会自动回收Timer,则不应再调用本函数回收Timer
u1 CdelMessageTiming(CMSG* pMsg)
{
	if (!pMsg->ptmr)
	{
		return CMSG_ERR_NOTIMING;
	}
	CTimerUserUnlinkDel(pMsg->ptmr, GetSysTimerWheel());
	return CKNL_ERROR_NONE;
}
/*
*********************************************************************************************************
*                                             CLINK
*										用于查找和搜索的表格
*********************************************************************************************************
*/
/*
//创建一个存储Object指针的链表
u1 CLinkCreate(void* addr, u4 maxnum)
{
	return CListCreate(addr, maxnum, sizeof(CSIMPLELINKNODE));
}

//将Object链接到一个链表组中，用于后续查询使用
CSIMPLELINKNODE* CLinkInsert(void* LinkHeadAddr,void* LinkNode, void* linkAddr)
{
	CSIMPLELINKNODE tempNode;
	CSIMPLELINKNODE* result;
	tempNode.data = LinkNode;
	tempNode.next = NULL;
	result = CListAdd(linkAddr, &tempNode, sizeof(CSIMPLELINKNODE), CLIST_TAIL);
	if (result)
	{
		//result的Next地址值进行处理
		CSimpleListAddTail(LinkHeadAddr, result, offsetof(CSIMPLELINKNODE, next));
		return result;
	}
	return NULL;
}

//判断比对handler地址的值，要求handler地址下的结构体的第一个值必须为ID
u1	CLinkIDcompare(const void * ObjID, const CSIMPLELINKNODE * handler)
{
	return (u4)ObjID == (*(u4*)(handler->data));
}


void* CLinkDataFind(void* addr, u4 ObjID)
{
	CSIMPLELINKNODE* temp;
	temp=CListFind(addr, (void*)ObjID, CLinkIDcompare);
	return temp->data;
}

u1 CLinkDataRemove(void* addr, u4 ObjID)
{
	return CListRemove(addr, (void*)ObjID, CLinkIDcompare);
}

u1 CLinkRemovePtr(void* addr, CSIMPLELINKNODE* ptr)
{
	return CListRemovePtr(addr, ptr);
}
*/