/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cknl.H
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-02
* --------------------
*  all kernel parameter macro define in this file
*********************************************************************************************************
*/
#ifndef __CKNL_H__
#define __CKNL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************************************
*                                          CKNL VERSION NUMBER
*********************************************************************************************************
*/

#define  KNL_VERSION                 01u                /* Version of EmFrmwork (Vx.yy mult. by 100)    */

/*
*********************************************************************************************************
*                                           INCLUDE HEADER FILES
*********************************************************************************************************
*/
#include"knltype.h"
#include"knloperator.h"
#include"knlmap.h"

/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
//错误代码 8bit数据 高5bit代表出错的模块类型， 低3bit代表出错的模块具体错误 1-15代表无类型的通用错误
#define CKNL_ERROR_NONE			0u

#define DATASIZE_OVERFLOW		1u
#define NO_VALID_DATA			2u

#define CLIST_ERR				16u
#define CLIST_FULL				17u
#define CLIST_INVALID_MEM		18u
#define CLIST_INDEX_OVERFLOW	19u
#define CLIST_EMPTY				20u

#define BUDDY_ERROR				24u
#define BUDDY_SIZE_OVERFLOW		25u
#define BUDDY_INVALID_MEM		26u

#define CQUEUE_FULL				33u
#define CMSG_TARGET_ERR			34u
#define CMSG_ERR_TEMP_TIMING	35u
#define CMSG_ERR_NOTIMING		36u

#define CHANLDR_ERR				40u
#define CHANLDR_CREATE_ERR		41u

#define CTIMER_ERR				50u
#define CTIMER_CREATE_ERR		51u
/*
*********************************************************************************************************
*                                             SHARED TYPE
*********************************************************************************************************
*/
//#define	MAX_OBJ_NAME_LEN     8              //OBJ名字的最大长度
typedef u1(*CCBFUNC)(struct __CMSG* PMSG);						//用于处理MESSAGE的函数类型
typedef u1(*CGcompare)(const DPARA dataA, const DPARA dataB);	//用于做比对的函数类型
typedef u1(*CTraverseFunc)(void* plistdata);					//用于遍历链表时进行处理的函数
typedef u1(*CReleaseFunc)(struct __COBJECT* pObj);				//用于释放模块内存时调用的函数
typedef u1(*CTimerFunc)(void* pdata);							//用于遍历链表时进行处理的函数


//注意修改CLIST_PT结构体大小需要修改cknl中的CLISTPT_SIZE大小
typedef struct __CLIST_PT {             /* CLIST_PT CONTROL BLOCK                                       */
	void   *ClistStartAddr;             /* Pointer to beginning of list partition                       */
	void   *ClistEndAddr;			   /* Pointer to end of list partition                             */
	void   *ClistPFree;                 /* Pointer to list of free memory blocks                        */
	void   *ClistExtAddr;               /* Pointer to ext addr for linklist using                       */
	u4		ClistBlkSize;              /* Size (in bytes) of each block of memory                      */
	u4		ClistNBlks;                /* Total number of blocks in this partition                     */
	u4		ClistNFree;                /* Number of memory blocks remaining in this partition          */
} CLIST_PT;

//单链表
typedef struct __CLINKLISTNODE {
	struct	__CLINKLISTNODE   *next;
} CLINKLISTNODE;

typedef struct __CLINKLIST {
	struct	__CLINKLISTNODE   *Head;
	struct	__CLINKLISTNODE   *Tail;
	u4							num;
	CMUTEX						lock;
} CLINKLIST;


typedef struct __CSIMPLELINKNODE{
	void* next;
	void* data;
} CSIMPLELINKNODE;


//CMSG消息结构体,包括消息ID,两个参数,回调函数
typedef struct __CMSG
{
	i2		what;
	u1		recyleEn;
	u1		reserved;
	MPARA   arg1;
	MPARA   arg2;
	//	CSEM*	SyncSem;
	//struct __CHANDLER* source; //源地址
	struct __CHANDLER* target; //目的地址
	struct __CTIMER*   ptmr; //定时发送时所需的timer指针
}CMSG;

//消息队里设计使用二级指针，防止由于队列在full状态时指针弹出，数据还未使用就进行下一次压入导致数据错误
typedef struct __CQUE
{
	u2   	num;
	u2  	size;
	void    **Qin;
	void    **Qout;
	void	**QStart;
	void	**QEnd;
	CSEM    hSEM;
}CQUE;

typedef struct __COBJECT
{
	u4					ObjID;				//描述的模块唯一标识，包括类型,ID等
	u4					refcnt;				//被引用次数计数值,用于安全删除obj
	//CSIMPLELINKNODE*	pLink;				//指向链接到自身的指针
	//void*				LinkAddr;			//存储链接的地址
	//CReleaseFunc		RelaseFunc;			//析构函数
	CSEM				ReleaseSem;			//用于析构的信号量
	void*				pointer;			//指向包含COBJECT的结构体的指针
}COBJECT;


typedef struct __CTHREAD
{
	thread_t	Mythread;
}CTHREAD;

typedef struct __CLOOPTHREAD
{
	thread_t	Mythread;
	CQUE*		MyQUE;
}CLOOPTHREAD;

typedef struct __CTHREADPARAM
{
	u2		prio;
	u2		reserved; 
	u4		stacksize;
	TDPARA	param;
}CTHREADPARAM;

typedef struct __CHANDLER
{
	u4					ObjID;			//描述的模块唯一标识，包括类型,ID等
	u1					working;		//是否正在工作
	u1					Reserved1;		//保留
	u2					Reserved2;		//保留
	void*				LinkAddr;		//存储链接的地址
	COBJECT*			pObj;			//指向实际模块结构体的指针
	CCBFUNC				callback;		//回调函数
	CLOOPTHREAD*		loopthread;		//obj使用的线程
}CHANDLER;


typedef struct __CTIMER
{
	struct __CTIMER* next;
	struct __CTIMER* prev;
	void* data;
	CTimerFunc pfunc;
	void* tmraddr;
	u4	  period;
	u4	  MatchTime;
	u1	  opt;
}CTIMER;

typedef struct __CTIMER_WHEEL
{
	CTIMER* first;
	CTIMER* tail;
}CTIMER_WHEEL;



//使用宏定义 object和handler 便于查找结构体元素
#define __COBJECT_DEFINE()		COBJECT MyObj
#define __CHANDLER_DEFINE(n)	COBJECT Myhandler[n]

/*
*********************************************************************************************************
*                                             CKNL OBJ TYPE
*********************************************************************************************************
*/
#define CKNL_TYPE_NONE        0
#define CKNL_TYPE_HANDLER     1
#define CKNL_TYPE_THREAD      2

/*
*********************************************************************************************************
*                                             CKNL MACRO PARAMETERS
*									(can be changed for adjuset system size)
*********************************************************************************************************
*/

#define CDRAM_MEMBLOCK_USED_EN		1				//1 means used  0 means not used  //是否使用内部的memblock模块昨晚动态内存申请模块

#define MEM_BLOCK_SIZE_SLOW_LOG     11              //大块的mem_block 表示2^n 使用buddy算法获取，速度较慢
#define MEM_BLOCK_SIZE_FAST         1024			//小块的mem_block 使用链表处理，获取速度较快
#define MAX_MEM_SLOW_BLOCK_LEVEL    6               //最大2^n个慢速块
#define MAX_MEM_FAST_BLOCK_NUM      256             //最大n个快速块

#define MEM_BLOCK_SIZE_SYSTEM		10				//系统线程或循环队列所使用的内存空间,不应被应用使用,使用buddy算法，最小使用2^n的块大小
#define MAX_MEM_SYSTEM_BLOCK_LEVEL  6               //最大2^n个慢速块


#define MAX_SYSTEM_MESSAGE_NUM		256				//系统使用的可回收的message最大数
#define MAX_SYSTEM_HANDLER_NUM		256				//系统使用的handler的最大数量
//#define MAX_SYSTEM_OBJECTLINK_NUM	256				//系统使用的最大objectlink数量
#define MAX_SYSTEM_LOOPQUE_SIZE		1024			//系统最大支持的QUE尺寸

#define CKNL_OBJ_RELEASE_IN_MSG_EN	1				//1 means object release can be used in Message Obtain or Recyle 0 means not 


#define	  CTIMER_MAX_NUM			200 //系统定时器的数量
#define	  CTIMER_WHEEL_SIZE			32  //系统时间轮片的数量
#define	  CTIMER_TICK_IN_MUTEX		1  //0 tick时需要使用MUTEX tick时不需要使用MUTEX tick在优先级较高的任务或中断时运行时考虑效率，可以不使用mutex
#define	  CTIMER_TICK_PERIOD		1  //定时器每次tick的周期，单位ms

/*
*********************************************************************************************************
*                                             SHARED DATA
*********************************************************************************************************
*/
extern CKNL_CR	G_KNR_CR;		//系统进入关键字段的全局变量
extern u4		G_KNRCR_MASK;   //用于确定是否系统进入了关键字段

extern u1		g_MsgMemBlock[];
extern u1		g_SysHandlerBlock[];
extern u1		g_SysObjLinkBlock[];

extern u1							g_TimerMemBlock[];
extern struct __CTIMER_WHEEL_GROUP	g_TimerWheel;


#define GetSysMsgGroup()	 ((void*)g_MsgMemBlock)  
#define GetSysHandlerGroup() ((void*)g_SysHandlerBlock)  
#define GetSysObjLink()		 ((void*)g_SysObjLinkBlock)  

#define   GetSysTimerGroup()       g_TimerMemBlock
#define   GetSysTimerWheel()       (&g_TimerWheel)

/*
*********************************************************************************************************
*                                             CMEMORY && CLIST
*********************************************************************************************************
*/

//#define BUDDYMEM_DEBUG
//macro for calc  
#define CLISTPT_SIZE              sizeof(CLIST_PT)    //listpt结构体预留大小
#define BUDDYST_SIZE(level)      ((2<<(level))+8)     //buddymem结构体预留大小

#define CLISTMEM_SIZE(num,size,ext)  ((num)*(size)+CLISTPT_SIZE+(ext))                  //listpt的mem大小
#define BUDDYMEM_SIZE(level,sizelog)    (BUDDYST_SIZE(level)+(1<<(level))*(1<<sizelog)) //buddymem大小计算

#define CLINKLIST_SIZE(num,blksize) CLISTMEM_SIZE(num,(blksize)+sizeof(CLINKLISTNODE),sizeof(CLINKLIST))
#define CLIST_HEAD             (0)             
#define CLIST_TAIL             (-1)              


//functions 
u1		ClistPTCreate(void *addr, u4 nblks, u4 blksize, u4 extblksize);
void*	ClistPTGetBlock(void *addr);
u1  	ClistPTPutBlock(void *addr, void* pblk);
u4		ClistPTGetUsedMem(void *addr);
u4		ClistPTGetFreeMem(void *addr);

u1		buddy_create(u1* addr, u1 sizelog, u2 blksizelog);
void	*buddy_alloc(void* memaddr, u4 size);
u1		buddy_free(void* memaddr, void* addr);
u4		buddy_GetUsedMem(void* memaddr);
u4		buddy_GetFreeMem(void* memaddr);


u1		cmemoryInit(void);
void*	cmem_alloc(u4 size);
u1		cmem_free(void *addr);
u4		cmem_GetUsedMem(void);
u4		cmem_GetFreeMem(void);


u1		csysmemoryInit(void);
void*	csysmem_alloc(u4 size);
u1		csysmem_free(void *addr);
u4		csysmem_GetUsedMem(void);
u4		csysmem_GetFreeMem(void);
u4		CMSG_GetUsedMem(void);
u4		Chandler_GetUsedMem(void);
u4		CTimer_GetUsedMem(void);
/*
*********************************************************************************************************
*                                             CLINKLIST
*********************************************************************************************************
*/
u1 CListCreate(void *addr, u4 nblks, u4 blksize);
void* CListAdd(void *addr, void* pdata, u4 datasize, i4 Index);
u1 CListRemove(void *addr, DPARA key, CGcompare compare);
u1 CListRemovePtr(void *addr, void* ptr);
void* CListFind(void *addr, DPARA key, CGcompare compare);
u1 CListTraverse(void *addr, CTraverseFunc handle);
u4 GetListNum(void *addr);
u4 GetListIdleNum(void *addr);


u1 CSimpleListInit(void** HeadAddr);
void* CSimpleListNext(void* Obj, u4 offset);
u1 CSimpleListAddHead(void** HeadAddr, void* NewObj, u4 offset);
u1 CSimpleListAddTail(void** HeadAddr, void* NewObj, u4 offset);
u1 CSimpleListRemovePtr(void** HeadAddr, void* DelObj, u4 offset);
u1 CSimpleListTraverse(void** HeadAddr, u4 offset, CTraverseFunc handle);
u4 GetSimpleListNum(void** HeadAddr, u4 offset);
/*
*********************************************************************************************************
*                                             CQUEUE
*********************************************************************************************************
*/
u1		CQCreate(CQUE* pQUE, void  *Qstart, u2 size);
u1		CQDel(CQUE* pQUE);
void*	CQPend(CQUE* pQUE);
u1		CQPost(CQUE* pQUE, void *pmsg);
/*
*********************************************************************************************************
*                                             CTREAD
*********************************************************************************************************
*/
CTHREAD CThreadCreate(TDFUNC pfunc, const CTHREADPARAM* pthreadparam);
u1 CThreadDel(CTHREAD* pthread);
CLOOPTHREAD* CLooperThreadCreate(const CTHREADPARAM* pthreadparam);
u1 CLooperThreadDel(CLOOPTHREAD* pMyThread);
/*
*********************************************************************************************************
*                                             COBJECT
*********************************************************************************************************
*/
#define IsNull(obj)					((void*)(obj)==NULL)
#define CHANDLERGROUP_SIZE(num)		CLINKLIST_SIZE(num,sizeof(CHANDLER*)) 
//#define COBJECTLINK_SIZE(num)		CLINKLIST_SIZE(num,sizeof(CSIMPLELINKNODE)) 

#define MSG_STOP					(-1)

#define CWriteMessage(pMsg,d1,d2)					do{pMsg->arg1=(MPARA)(d1);pMsg->arg2=(MPARA)(d2);}while(0)

#if MAX_SYSTEM_HANDLER_NUM>0
#define CSysHandlerCreate(ObjID,callback,pthread,ptr)	ChandlerCreate(g_SysHandlerBlock,ObjID,callback,pthread,ptr)
#define CSysHandlerFind(ObjID)							ChandlerFind(g_SysHandlerBlock,ObjID)
#define CSysHandlerDel(ObjID)							ChandlerDel(g_SysHandlerBlock,ObjID)
#define CSysHandlerInsert(handle)						ChandlerInsert(handle,g_SysHandlerBlock)

//#define CSysObjectInit(pObj,ObjID,pointer)			CObjectInit(pObj,ObjID,g_SysHandlerBlock,pointer)
#endif

//#if MAX_SYSTEM_OBJECTLINK_NUM>0
//#endif
//#define CSysLinkInsert(pObj)							CLinkInsert(g_SysObjLinkBlock,pObj)
//#define CSysLinkDataFind(ObjID)							CLinkDataFind(g_SysObjLinkBlock,ObjID)
//#define CSysLinkDataRemove(ObjID)						CLinkDataRemove(g_SysObjLinkBlock,ObjID)
//#define CSysLinkDataRemovePtr(ptr)						CLinkRemovePtr(g_SysObjLinkBlock,ptr)


#define CobtainMsgStatic(handler,what)					CobtainMessage(handler,what,0)
#define CobtainMsgTemp(handler,what)					CobtainMessage(handler,what,1)

#define ChandlerConectStatic(handler)					ChandlerConect(handler,1)
#define ChandlerConectTemp(handler)						ChandlerConect(handler,0)

#define ChandlerDisConectStatic(handler)				do{ChandlerDisConect(handler); handler=NULL;}while(0)
#define ChandlerDisConectTemp(handler)					do{handler=NULL;}while(0)


#define COBJ_HANDLE_DEF(hnum,tnum)  COBJECT MyObj;\
									CHANDLER MyHandler[hnum];\
									CHANDLER* tgtHandler[tnum]

#define COBJ_HANDLE_LOOPTHREAD_DEF(hnum,tnum)   COBJ_HANDLE_DEF(hnum,tnum);\
												CLOOPTHREAD* pMythread

#define MyPointer_GET(pMSG,MyType)				(MyType*)((pMSG)->target->pObj->pointer)
#define MSG_D1_GET(pMSG,MyType)					(MyType)((pMSG)->arg1)
#define MSG_D2_GET(pMSG,MyType)					(MyType)((pMSG)->arg2)
#define MSG_WHAT(pMSG)							((pMSG)->what)


#define MyObj_GET(p)							&((p)->MyObj)		
#define MyHandler_GET(p,k)						((p)->MyHandler+k)		
#define tgtHandler_GET(p,k)						((p)->tgtHandler[k])
#define MyThread_GET(p)							((p)->pMythread)
#define MyObj_Init(p,OBJID)						CObjectInit(MyObj_GET(p),OBJID,p)	
#define MyLoopThread_Init(p,ThreadPara)			do{(p)->pMythread=CLooperThreadCreate(ThreadPara);}while(0)


#define tgtHandler_Conect_Static(p,k,handler)	do{(p)->tgtHandler[k]=ChandlerConectStatic(handler);}while(0)
#define tgtHandler_Conect_Temp(p,k,handler)		do{(p)->tgtHandler[k]=ChandlerConectTemp(handler);}while(0)
#define tgtHandler_DisConect_Static(p,k)		ChandlerDisConectStatic(MyHandler_GET(p,k))
#define tgtHandler_DisConect_Temp(p,k)			ChandlerDisConectTemp(MyHandler_GET(p,k))
#define tgtHandler_Init(p,k)					do{(p)->tgtHandler[k]=NULL;}while(0)


#define MyHandler_Init_MyThread(p,k,OBJID,pfunc)		ChandlerInit(MyHandler_GET(p,k), OBJID, pfunc,MyThread_GET(p), MyObj_GET(p))
#define MyHandler_Init(p,k,OBJID,pfunc,pthread)			ChandlerInit(MyHandler_GET(p,k), OBJID, pfunc,pthread, MyObj_GET(p))
#define MyHandler_Insert(p,k,LinkAddr)					ChandlerInsert(MyHandler_GET(p,k),LinkAddr)
#if MAX_SYSTEM_HANDLER_NUM>0
#define MyHandler_InsertSys(p,k)						CSysHandlerInsert(MyHandler_GET(p,k))
#endif

#define MyHhandler_DEL(p,k)			ChandlerClose(MyHandler_GET(p,k));\
									ChandlerDelPtr(MyHandler_GET(p,k))

#define MyObjReleaseReq(p)			CObjectReleaseReq(MyObj_GET(p))
#define MyThread_Del(p)				CLooperThreadDel(MyThread_GET(p))

u1 ChandleLinkCreate(void* addr, u4 maxnum);
u1 ChandlerInit(CHANDLER* phanlder, u4 ObjID, CCBFUNC callback, CLOOPTHREAD* ploopthread, COBJECT* pObj);
u1 ChandlerInsert(CHANDLER* phandler, void* linkAddr);
CHANDLER* ChandlerFind(void* addr, u4 ObjID);
u1 ChandlerDelbyID(void* addr, u4 ObjID);
u1 ChandlerDelPtr(CHANDLER* handler);
u1 ChandlerOpen(CHANDLER* phanlder);
u1 ChandlerClose(CHANDLER* phanlder);
CHANDLER* ChandlerConect(CHANDLER* target, u1 staticmode);
u1 ChandlerDisConect(CHANDLER* phanlder);

CMSG* CobtainMessage(CHANDLER* handler, i2 what, u1 recyleEn);
u1 CrecycleMessage(CMSG* pMSG);
u1 CsendMessage(CMSG* pMsg);
u1 CsendMessageDelay(CMSG* pMsg, u4 Delay);
u1 CsendMessageTiming(CMSG* pMsg, u4 period);
u1 CctrlMessageTiming(CMSG* pMsg, u1 opt);
u1 CdelMessageTiming(CMSG* pMsg);

u1 CObjectInit(COBJECT* pObj, u4 ObjID, void* pointer);
u1 CObjectReleaseReq(COBJECT* pObj);

//u1 CLinkCreate(void* addr, u4 maxnum);
//CSIMPLELINKNODE* CLinkInsert(void* LinkHeadAddr, void* LinkNode, void* linkAddr);
//void* CLinkDataFind(void* addr, u4 ObjID);
//u1 CLinkDataRemove(void* addr, u4 ObjID);
//u1 CLinkRemovePtr(void* addr, CSIMPLELINKNODE* ptr);

/*
*********************************************************************************************************
*                                             CTIMER
*********************************************************************************************************
*/


#define   CTIMER_MASK_LINK			0x1
#define   CTIMER_MASK_SENDMSG		0x2
#define   CTIMER_MASK_DEL			0x4

#define   CTIMER_OPERATOR_ONESHOT (CTIMER_MASK_SENDMSG|CTIMER_MASK_DEL)
#define   CTIMER_OPERATOR_RUN     (CTIMER_MASK_SENDMSG|CTIMER_MASK_LINK)
#define   CTIMER_OPERATOR_STOP    (CTIMER_MASK_LINK)
#define   CTIMER_OPERATOR_DELETE  (CTIMER_MASK_DEL)


typedef struct __CTIMER_WHEEL_GROUP
{
	CTIMER_WHEEL wheel[CTIMER_WHEEL_SIZE];
	u4			 currentTime;
	CMUTEX		 lock;
}CTIMER_WHEEL_GROUP;

u1 CTimerGroupInit(void* timeraddr, u4 timernum);
u1 CTimerWheelInit(void* tmrwheeladdr);
CTIMER* CTimerCreate(void* timeraddr, CTimerFunc pfunc, void* data, u4 period, u1 opt);
u1  CTimerCtrl(CTIMER*ptmr, u1 opt);
void CTimerUserLink(CTIMER*ptmr, CTIMER_WHEEL_GROUP* ptmr_wheel_group);
void CTimerUserUnlinkDel(CTIMER*ptmr, CTIMER_WHEEL_GROUP* ptmr_wheel_group);
void CsystimerInit(void);

/*
*********************************************************************************************************
*                                             CDEBUG
*********************************************************************************************************
*/
#define CDEBUG_EN					0 //0 表示不使用DEBUG 1表示使用CDebugErrExport进行debug 

#define CDEBUG_FILE 				"cdebug_err.log"
#define CDEBUG_STDOUT 				stdout
#define MAX_CDEBUG_BUFF_SIZE        1024 //最大1K数据量输出
#define MAX_STACK_FRAME_NUM			8 //最大回溯栈帧数量
#define MAX_STACK_FRAME_SIZE		(1<<16)	//栈帧大小最大假设为1M

#define CDEBUG_IOMODE_NOUT			0
#define CDEBUG_IOMODE_STDOUT		1
#define CDEBUG_IOMODE_FLIE			2

#define CDEBUG_INIT_IOMODE			CDEBUG_IOMODE_STDOUT //初始化时使用的CDEBUG_IOMODE值

#if (CDEBUG_EN)
#define CDEBUG_ON(err)	CDebugErrExport(err)
#else
#define CDEBUG_ON(err)	(err)
#endif

void CDebugInit(void);
u4	 CDebugErrExport(u4 errcode);
u1 CDebugSetIOMODE(u4 mode);
u4 CDebugErrExport(u4 errcode);
i4 CDebugPrintf(const char *format, ...);

/*
*********************************************************************************************************
*                                             CKNL
*********************************************************************************************************
*/

extern u1	CknlInit(void);

#ifdef __cplusplus
}
#endif
#endif /* KNLTYPE_H */