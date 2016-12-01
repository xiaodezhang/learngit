/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : clinklist.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-09
* --------------------
*  a singlelinklist used by listpt
*********************************************************************************************************
*/
#include"cknl.h"
#include<string.h>


//ʹ���������������п飬��ʹ�õ��������ʹ�ÿ�
u1	CListCreate(void *addr, u4 nblks, u4 blksize)
{
	u1 err;
	CLINKLIST* pLinklist;
	err = ClistPTCreate(addr, nblks, blksize + sizeof(CLINKLISTNODE), sizeof(CLINKLIST));
	if (err == CKNL_ERROR_NONE)
	{
		pLinklist = (CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr;
		pLinklist->Head = NULL;
		pLinklist->Tail = NULL;
		pLinklist->num = 0;
		CMutexCreate(pLinklist->lock);
	}
	return err;
}

//����һ����������,����ͷ���룬β�������Ų���,�������ݵ�ַָ��
void* CListAdd(void *addr, void* pdata, u4 datasize, i4 Index)
{
	CLINKLIST* pLinklist = (CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr;
	CLINKLISTNODE* temp;
	CLINKLISTNODE* current;

	if (datasize > ((CLIST_PT*)addr)->ClistBlkSize - sizeof(CLINKLISTNODE))
	{
		CDEBUG_ON(CLIST_ERR);
		return NULL;
	}
	temp = (CLINKLISTNODE*)ClistPTGetBlock(addr);
	if (!temp)
	{
		return NULL;
	}
	CMutexLock(pLinklist->lock);
	if(Index > (i4)pLinklist->num)
	{	//�����Ŵ���ʵ����ֵ,���Զ�У����β��
		Index = pLinklist->num;
	}
	if (pLinklist->num == 0)
	{
		pLinklist->Head = temp;
		pLinklist->Tail = temp;
		temp->next = NULL;
	}
	else if (Index == CLIST_TAIL)
	{
		pLinklist->Tail->next = temp;
		pLinklist->Tail = temp;
		temp->next = NULL;
	}
	else if (Index == CLIST_HEAD)
	{
		temp->next = pLinklist->Head;
		pLinklist->Head = temp;
	}
	else
	{
		current = pLinklist->Head;
		while (1<Index--)
		{
			current = current->next;
		}
		temp->next = current->next;
		current->next = temp;
		if (current == pLinklist->Tail)
		{
			pLinklist->Tail = temp;
		}
	}
	pLinklist->num++;
	CMutexUnlock(pLinklist->lock);
	memmove((u1*)temp + sizeof(CLINKLISTNODE), pdata, datasize);
	return ((u1*)temp + sizeof(CLINKLISTNODE));
}


//���ڹؼ��ֺͱȽϺ�����ȥ��һ��Ԫ��,����ȽϺ���ΪNULL,��Ƚ�key�͵�ǰ�ڵ�ָ���Ƿ����
u1 CListRemove(void *addr, DPARA key, CGcompare compare)
{
	CLINKLIST* pLinklist = (CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr;
	CLINKLISTNODE* current;
	CLINKLISTNODE* last;
	u1			   cpresult; //�ȽϽ��
	CMutexLock(pLinklist->lock);
	current = pLinklist->Head;
	last	= NULL;
	while (current)
	{
		cpresult = (compare) ? compare(key, (u1*)current + sizeof(CLINKLISTNODE)) : (key == (u1*)current + sizeof(CLINKLISTNODE));
		if (cpresult)
		{
			if (last)
			{
				last->next = current->next;
			}
			pLinklist->num--;
			if (pLinklist->num == 0)
			{
				pLinklist->Head = NULL;
				pLinklist->Tail = NULL;
			}
			else if (current == pLinklist->Head)
			{
				pLinklist->Head = current->next;
			}
			else if (current == pLinklist->Tail)
			{
				pLinklist->Tail = last;
			}
			ClistPTPutBlock(addr, (void*)current);
			CMutexUnlock(pLinklist->lock);
			return CKNL_ERROR_NONE;
		}
		last =	  current;
		current = current->next;
	}
	CMutexUnlock(pLinklist->lock);
	return CDEBUG_ON(NO_VALID_DATA);
}

u1 CListRemovePtr(void *addr, void* ptr)
{
	return CListRemove(addr, ptr, NULL);
}

//���ڹؼ��ֺͱȽϺ�����Ѱ����Ӧ������
void* CListFind(void *addr, DPARA key, CGcompare compare)
{
	CLINKLIST* pLinklist = (CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr;
	CLINKLISTNODE* current;
	CMutexLock(pLinklist->lock);
	current = pLinklist->Head;
	while (current)
	{
		if (compare(key, (u1*)current + sizeof(CLINKLISTNODE)))
		{
			CMutexUnlock(pLinklist->lock);
			return (void*)((u1*)current + sizeof(CLINKLISTNODE));
		}
		current = current->next;
	}
	CMutexUnlock(pLinklist->lock);
	return NULL;
}

//������������,���������������ֵ��Ϊ0,���жϱ���������������
u1 CListTraverse(void *addr, CTraverseFunc handle) //handleΪ�ڵ��������,���û��Լ���д
{
	u1 err;
	CLINKLIST* pLinklist = (CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr;
	CLINKLISTNODE* current;
	CMutexLock(pLinklist->lock);
	current = pLinklist->Head;

	while (current) {
		err=handle((void*)((u1*)current + sizeof(CLINKLISTNODE)));
		if (err)
		{
			CMutexUnlock(pLinklist->lock);
			return err;
		}
		current = current->next;
	}
	CMutexUnlock(pLinklist->lock);
	return CKNL_ERROR_NONE;
}
//��ȡ��������ЧԪ����
u4 GetListNum(void *addr)
{
	return ((CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr)->num;
}
//��ȡ�����п��п���Ԫ����
u4 GetListIdleNum(void *addr)
{
	return (((CLIST_PT*)addr)->ClistNFree);
}


//�������� ֻʹ�õ�ַ,�ʹ洢�õ�ַ��һ����ַ�����ƫ���� �������������Ϣ
u1 CSimpleListInit(void** HeadAddr)
{
	if (!HeadAddr)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	*HeadAddr = NULL;
	return CKNL_ERROR_NONE;
}
//��ȡĳ��object����һ��object
void* CSimpleListNext(void* Obj, u4 offset)
{
	if (!Obj)
	{
		return NULL;
	}
	return *(void**)((u1*)Obj + offset);
}

//������ͷ������NewObj, Ҫ��HeadAddr �����ǵ�һ��ͷ��ַ����������
u1 CSimpleListAddHead(void** HeadAddr, void* NewObj, u4 offset)
{
	void** Node = (void**)((u1*)NewObj + offset);

	if (!HeadAddr ||!NewObj)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	if (!*HeadAddr)
	{ //���ͷ��ַΪ�յ�ַ���½���һ��Node
		*Node = NULL;
	}
	else
	{
		*Node = *HeadAddr;
	}
	*HeadAddr = NewObj;
	return CKNL_ERROR_NONE;
}
//������β������NewObj
u1 CSimpleListAddTail(void** HeadAddr, void* NewObj, u4 offset)
{
	void** Node = HeadAddr;

	if (!HeadAddr || !NewObj)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	if (!*HeadAddr)
	{ //���ͷ��ַΪ�յ�ַ���½���һ��Node
		*HeadAddr = NewObj;
		*(void**)((u1*)(NewObj) + offset)=NULL;
		return CKNL_ERROR_NONE;
	}
	while (*Node)
	{
		Node= (void**)((u1*)(*Node) + offset);
	}
	*Node = NewObj;
	Node= (void**)((u1*)NewObj + offset);
	*Node = NULL;
	return CKNL_ERROR_NONE;
}
//���������޳�ĳ��Obj
u1 CSimpleListRemovePtr(void** HeadAddr, void* DelObj, u4 offset)
{
	void** current;

	if (!HeadAddr || !*HeadAddr || !DelObj)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	/*if (*HeadAddr == DelObj)
	{
		*HeadAddr = *(void**)((u1*)(DelObj) + offset);
		return CKNL_ERROR_NONE;
	}*/

	current = HeadAddr;// (void**)((u1*)(*HeadAddr) + offset);
	while(*current)
	{
		if (*current == DelObj)
		{
			*current = *(void**)((u1*)(*current) + offset);
			return CKNL_ERROR_NONE;
		}
		current= (void**)((u1*)(*current)+offset);
	}
	return CDEBUG_ON(NO_VALID_DATA);
}

u1 CSimpleListTraverse(void** HeadAddr, u4 offset, CTraverseFunc handle) //handleΪ�ڵ��������,���û��Լ���д
{
	u1 err;
	void** current;
	if (!HeadAddr)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	current = HeadAddr;
	while (*current) {
		err = handle(*current);
		if (err)
		{
			return err;
		}
		current = (void**)((u1*)(*current) + offset);
	}
	return CKNL_ERROR_NONE;
}

u4 GetSimpleListNum(void** HeadAddr, u4 offset)
{
	u4 num	=0;
	void** current;
	current = HeadAddr;
	while (*current) {
		num++;
		current = (void**)((u1*)(*current) + offset);
	}
	return num;
}