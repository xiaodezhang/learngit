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


//使用链表分区管理空闲块，再使用单链表管理使用块
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

//插入一个链表数据,包括头插入，尾插入和序号插入,返回数据地址指针
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
	{	//如果序号大于实际数值,则自动校正至尾部
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


//基于关键字和比较函数，去除一个元素,如果比较函数为NULL,则比较key和当前节点指针是否相等
u1 CListRemove(void *addr, DPARA key, CGcompare compare)
{
	CLINKLIST* pLinklist = (CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr;
	CLINKLISTNODE* current;
	CLINKLISTNODE* last;
	u1			   cpresult; //比较结果
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

//基于关键字和比较函数，寻找相应的数据
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

//遍历所有链表,如果遍历函数返回值不为0,则中断遍历并输出错误代码
u1 CListTraverse(void *addr, CTraverseFunc handle) //handle为节点遍历策略,由用户自己编写
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
//获取链表中有效元素数
u4 GetListNum(void *addr)
{
	return ((CLINKLIST*)((CLIST_PT*)addr)->ClistExtAddr)->num;
}
//获取链表中空闲可用元素数
u4 GetListIdleNum(void *addr)
{
	return (((CLIST_PT*)addr)->ClistNFree);
}


//简单链表处理 只使用地址,和存储该地址下一个地址的相对偏移量 不含其它表格信息
u1 CSimpleListInit(void** HeadAddr)
{
	if (!HeadAddr)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	*HeadAddr = NULL;
	return CKNL_ERROR_NONE;
}
//获取某个object的下一个object
void* CSimpleListNext(void* Obj, u4 offset)
{
	if (!Obj)
	{
		return NULL;
	}
	return *(void**)((u1*)Obj + offset);
}

//在链表头部增加NewObj, 要求HeadAddr 必须是第一个头地址，否则会出错
u1 CSimpleListAddHead(void** HeadAddr, void* NewObj, u4 offset)
{
	void** Node = (void**)((u1*)NewObj + offset);

	if (!HeadAddr ||!NewObj)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	if (!*HeadAddr)
	{ //如果头地址为空地址则新建第一个Node
		*Node = NULL;
	}
	else
	{
		*Node = *HeadAddr;
	}
	*HeadAddr = NewObj;
	return CKNL_ERROR_NONE;
}
//在链表尾部增加NewObj
u1 CSimpleListAddTail(void** HeadAddr, void* NewObj, u4 offset)
{
	void** Node = HeadAddr;

	if (!HeadAddr || !NewObj)
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}
	if (!*HeadAddr)
	{ //如果头地址为空地址则新建第一个Node
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
//在链表中剔除某个Obj
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

u1 CSimpleListTraverse(void** HeadAddr, u4 offset, CTraverseFunc handle) //handle为节点遍历策略,由用户自己编写
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