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
			CSemDel(pObj->ReleaseSem); //�ź���������ɺ�ɾ��֮
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
			CSemPost(pObj->ReleaseSem);//������ɺ����ź���
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
	if (!callback || !ploopthread || !pObj) //������callbackΪ0��threadΪ0��pObjΪ0
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


//��Object���ӵ�һ���������У����ں�����ѯʹ��

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

//������Ϣ�ĺ���
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

//�ӳٷ�����Ϣ�ĺ���
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

//��ʱ������Ϣ�ĺ���:������ʹ����ʱ�Զ����յ�MSG,ֻ��ʹ�þ�̬MSG���ֶ�����
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

//���ƶ�ʱMessage�ķ��Ͳ�����������ͣ�����������ں�ɾ���ȹ���
u1 CctrlMessageTiming(CMSG* pMsg, u1 opt)
{
	if (!pMsg->ptmr)
	{
		return CMSG_ERR_NOTIMING;
	}
	CTimerCtrl(pMsg->ptmr, opt);
	return CKNL_ERROR_NONE;
}

//ɾ��Msg�Ķ�ʱ���ܣ�ע��˴����������MSG����Ҫ�ֶ�����CrecycleMessage����
//ע�����ʹ�ÿ��ƹ��ܻ��ӳٷ��͹��ܳ�����Զ�����Timer,��Ӧ�ٵ��ñ���������Timer
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
*										���ڲ��Һ������ı��
*********************************************************************************************************
*/
/*
//����һ���洢Objectָ�������
u1 CLinkCreate(void* addr, u4 maxnum)
{
	return CListCreate(addr, maxnum, sizeof(CSIMPLELINKNODE));
}

//��Object���ӵ�һ���������У����ں�����ѯʹ��
CSIMPLELINKNODE* CLinkInsert(void* LinkHeadAddr,void* LinkNode, void* linkAddr)
{
	CSIMPLELINKNODE tempNode;
	CSIMPLELINKNODE* result;
	tempNode.data = LinkNode;
	tempNode.next = NULL;
	result = CListAdd(linkAddr, &tempNode, sizeof(CSIMPLELINKNODE), CLIST_TAIL);
	if (result)
	{
		//result��Next��ֵַ���д���
		CSimpleListAddTail(LinkHeadAddr, result, offsetof(CSIMPLELINKNODE, next));
		return result;
	}
	return NULL;
}

//�жϱȶ�handler��ַ��ֵ��Ҫ��handler��ַ�µĽṹ��ĵ�һ��ֵ����ΪID
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