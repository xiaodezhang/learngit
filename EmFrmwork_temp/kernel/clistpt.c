/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : clistpt.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-02
* --------------------
*  a list used by app list or by memory list
*********************************************************************************************************
*/
#include"cknl.h"

//单链表


//创建一个链表 包括首地址, 数据块数, 每个块的大小,额外的块(在第一个有效块之前，用于存储一些额外数据)
u1	ClistPTCreate(void *addr, u4 nblks, u4 blksize, u4 extblksize)
{
	CLIST_PT  *plist;
    u1        *pblk;
    void     **plink;
	u1        *blkaddr;
    u4		  i;
	
	plist = addr;									  /* Get next free memory partition                */

    if (plist == (CLIST_PT *)0) {                     /* See if we have a memory partition             */
        return CDEBUG_ON(CLIST_INVALID_MEM);
    }
	blkaddr = (u1 *)addr + sizeof(CLIST_PT)+ extblksize;
    plink = (void **)blkaddr;                         /* Create linked list of free memory blocks      */
    pblk  = blkaddr + blksize;
    for (i = 0; i < (nblks - 1); i++) {
        *plink = (void *)pblk;
        plink  = (void **)pblk;
        pblk   = pblk + blksize;
    }
    *plink                = (void *)0;                  /* Last memory block points to NULL              */
	plist->ClistStartAddr = (void *)blkaddr;            /* Store start address of memory partition       */
	plist->ClistPFree     = (void *)blkaddr;            /* Initialize pointer to pool of free blocks     */
	plist->ClistNFree     = nblks;                      /* Store number of free blocks in MCB            */
	plist->ClistNBlks     = nblks;
	plist->ClistBlkSize   = blksize;                    /* Store block size of each memory blocks        */
	plist->ClistEndAddr   = pblk;
	plist->ClistExtAddr   = (u1 *)addr + sizeof(CLIST_PT);
    return (CKNL_ERROR_NONE);
}


//向链表中返回一个块，并重新被链表管理
u1	ClistPTPutBlock(void *addr, void* pblk)
{
	CLIST_PT  *plist=(CLIST_PT*)addr;

	if ((pblk >= plist->ClistEndAddr) || (pblk < plist->ClistStartAddr))
	{
		return CDEBUG_ON(CLIST_INVALID_MEM);
	}

	CKNL_ENTER_CRITICAL();
    if (plist->ClistNFree >= plist->ClistNBlks) {  /* Make sure all blocks not already returned          */
		CKNL_EXIT_CRITICAL();
        return CDEBUG_ON(CLIST_FULL);
    }

    *(void **)pblk      = plist->ClistPFree;   /* Insert released block into free block list         */
    	
	plist->ClistPFree    = pblk;
	plist->ClistNFree++;                          /* One more memory block in this partition            */
	CKNL_EXIT_CRITICAL();
    return (CKNL_ERROR_NONE);                          /* Notify caller that memory block was released       */
}

//从链表中取出一个块，不再被链表管理
void*	ClistPTGetBlock(void *addr)
{
	CLIST_PT  *plist = (CLIST_PT*)addr;
    void      *pblk;

	CKNL_ENTER_CRITICAL();
    
    if(plist->ClistNFree > 0) {                      /* See if there are any free memory blocks       */
        pblk                = plist->ClistPFree;	 /* Yes, point to next free memory block          */
		plist->ClistPFree	= *(void **)pblk;        /*      Adjust pointer to new free list          */
		plist->ClistNFree--;                         /*      One less memory block in this partition  */
    	             
		CKNL_EXIT_CRITICAL();
        return (pblk);                               /*      Return memory block to caller            */
    }

	CKNL_EXIT_CRITICAL();
	CDEBUG_ON(CLIST_EMPTY);
    return NULL;                               /*      Return NULL pointer to caller            */
}

u4 ClistPTGetUsedMem(void *addr)
{
	return (((CLIST_PT*)addr)->ClistNBlks - ((CLIST_PT*)addr)->ClistNFree)*((CLIST_PT*)addr)->ClistBlkSize;
}

u4 ClistPTGetFreeMem(void *addr)
{
	return ((CLIST_PT*)addr)->ClistNFree*((CLIST_PT*)addr)->ClistBlkSize;
}