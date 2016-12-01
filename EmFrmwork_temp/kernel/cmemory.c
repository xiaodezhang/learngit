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
#include"cknl.h"

#if CDRAM_MEMBLOCK_USED_EN == 1
u1 g_CListMemBlock[CLISTMEM_SIZE(MAX_MEM_FAST_BLOCK_NUM, MEM_BLOCK_SIZE_FAST, 0)];
u1 g_BuddyMemBlock[BUDDYMEM_SIZE(MAX_MEM_SLOW_BLOCK_LEVEL, MEM_BLOCK_SIZE_SLOW_LOG)];
#else
#include<malloc.h>
#endif

u1 g_SysMemBlock[BUDDYMEM_SIZE(MAX_MEM_SYSTEM_BLOCK_LEVEL, MEM_BLOCK_SIZE_SYSTEM)];
u1 g_MsgMemBlock[CLISTMEM_SIZE(MAX_SYSTEM_MESSAGE_NUM,sizeof(CMSG),0)];

#if MAX_SYSTEM_HANDLER_NUM>0
u1 g_SysHandlerBlock[CHANDLERGROUP_SIZE(MAX_SYSTEM_HANDLER_NUM)];
#endif


u1 g_TimerMemBlock[CLISTMEM_SIZE(CTIMER_MAX_NUM, sizeof(CTIMER), 0)];
CTIMER_WHEEL_GROUP g_TimerWheel;


/*
#if MAX_SYSTEM_OBJECTLINK_NUM>0
u1 g_SysObjLinkBlock[COBJECTLINK_SIZE(MAX_SYSTEM_OBJECTLINK_NUM)];
#endif
*/
//����1�������ڴ����飬���ڹ����С�Ŀ����ڴ��ȡ���Լ�һ��buddy�㷨����飬���ڽϴ���ڴ���ȡ
u1	cmemoryInit(void)
{
#if CDRAM_MEMBLOCK_USED_EN == 1
	ClistPTCreate((void*)g_CListMemBlock, MAX_MEM_FAST_BLOCK_NUM, MEM_BLOCK_SIZE_FAST, 0);
	buddy_create((void*)g_BuddyMemBlock, MAX_MEM_SLOW_BLOCK_LEVEL, MEM_BLOCK_SIZE_SLOW_LOG);
#endif
	return 0;
}


//���������ڴ���ڿ��������С�������buddy_alloc���������ClistPTGetBlock
void*	cmem_alloc(u4 size)
{
	void* addr=(void*)0;
#if CDRAM_MEMBLOCK_USED_EN == 1
	if (size > MEM_BLOCK_SIZE_FAST)
	{
		addr = buddy_alloc((void*)g_BuddyMemBlock, size);
	}
	else if(size)
	{
		addr = ClistPTGetBlock((void*)g_CListMemBlock);
		if (!addr)
			addr = buddy_alloc((void*)g_BuddyMemBlock, size);
	}
#else
	addr = malloc(size);
#endif
	return addr;
}


//������ص����ݲ���������,�򷵻���buddymem
u1	cmem_free(void *addr)
{
#if CDRAM_MEMBLOCK_USED_EN == 1
	if (CKNL_ERROR_NONE==ClistPTPutBlock((void*)g_CListMemBlock, addr))
	{
		return CKNL_ERROR_NONE;
	}
	else
	{
		return buddy_free((void*)g_BuddyMemBlock, addr);
	}
#else
	 free(addr);
	 return CKNL_ERROR_NONE;
#endif
}

u4 cmem_GetUsedMem(void)
{
#if CDRAM_MEMBLOCK_USED_EN == 1
	u4 size;
	size= ClistPTGetUsedMem((void*)g_CListMemBlock);
	size += buddy_GetUsedMem((void*)g_BuddyMemBlock);
	return size;
#else
	return 0;
#endif
}

u4 cmem_GetFreeMem(void)
{
#if CDRAM_MEMBLOCK_USED_EN == 1
	u4 size;
	size = ClistPTGetFreeMem((void*)g_CListMemBlock);
	size += buddy_GetFreeMem((void*)g_BuddyMemBlock);
	return size;
#else
	return 0;
#endif
}

u1	csysmemoryInit(void)
{
	//��ʼ��ϵͳ�ڲ�ʹ�õ��ڴ��
	buddy_create((void*)g_SysMemBlock, MAX_MEM_SYSTEM_BLOCK_LEVEL, MEM_BLOCK_SIZE_SYSTEM);
	//��ʼ��MESSAGE��
	ClistPTCreate(g_MsgMemBlock,MAX_SYSTEM_MESSAGE_NUM,sizeof(CMSG), 0);
	//��ʼ��ϵͳ�ṩ��handler��

#if MAX_SYSTEM_HANDLER_NUM>0
	ChandleLinkCreate(g_SysHandlerBlock, MAX_SYSTEM_HANDLER_NUM);
#endif

	//��ʼ��ϵͳ�ṩ��Timer��
	CTimerGroupInit(GetSysTimerGroup(), CTIMER_MAX_NUM);
	CTimerWheelInit(GetSysTimerWheel());

/*	//��ʼ��ϵͳ�ṩ��objectlink��
#if MAX_SYSTEM_OBJECTLINK_NUM>0
	CLinkCreate(g_SysObjLinkBlock, MAX_SYSTEM_OBJECTLINK_NUM);
#endif
*/
	return 0;
}


//ϵͳ�ڲ�ʹ�õ��ڴ����뺯��
void*	csysmem_alloc(u4 size)
{
	return buddy_alloc((void*)g_SysMemBlock, size);
}

//ϵͳ�ڲ�ʹ�õ��ڴ��ͷź���
u1	csysmem_free(void *addr)
{
	return buddy_free((void*)g_SysMemBlock, addr);
}

u4 csysmem_GetUsedMem(void)
{
	return buddy_GetUsedMem((void*)g_SysMemBlock);
}

u4 csysmem_GetFreeMem(void)
{
	return buddy_GetFreeMem((void*)g_SysMemBlock);
}

u4 CMSG_GetUsedMem(void)
{
	return ClistPTGetUsedMem(g_MsgMemBlock);
}

u4 Chandler_GetUsedMem(void)
{
	return ClistPTGetUsedMem(g_SysHandlerBlock);
}

u4 CTimer_GetUsedMem(void)
{
	return ClistPTGetUsedMem(g_TimerMemBlock);

}