/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cdebug.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-11-07
* --------------------
*  kernel debug function with backtrace and other function
*********************************************************************************************************
*/
#include"cknl.h"
#include<stdio.h>
#include<string.h>
#include <assert.h>
/*
x86 CPU ebp  扩展基址指针 相当于栈底指针
        esp  栈顶指针(以高地址为底部，压栈后该值减小)
		eip  cpu下一条指令的指针
栈结构: ---高地址--- 参数N
					...
					参数1
					EIP
	   ----下一帧------
					EBP
					临时变量1
					...
					临时变量N
	  
					参数N
	   ----低地址---
*/

/*
未实现
ARM CPU fp  扩展基址指针 相当于栈底指针
		sp  栈顶指针(以高地址为底部，压栈后该值减小)
		lr  cpu下一条指令的指针(函数返回指针)
		pc  当前调用函数指针
栈结构: ---高地址--- PC
					LR
					SP
					FP
					参数1
					...
					参数N
	   ----下一帧------
					PC
	   ----低地址---
*/

#if(CDEBUG_EN)

#define ADVANCE_STACK_FRAME(ebp)	(*(ebp))
#define RETURN_ADDRESS(ebp)			*((ebp)+1)
#define GETEBP_BY_PARAM(param)		((void **)(&(param))-2)

typedef struct __CDEBUGPARAM {
	char* string;		//指向临时数据输出的数据缓冲器
#ifdef CDEBUG_FILE
	FILE* fp;			//指向用于存储debug信息的文件
#endif
	u4	  lasterror;	//上一个出错的错误代码
	u4	  iomode;		//debug输出接口
} CDEBUGPARAM;

CDEBUGPARAM  m_Cdebugparam;//用于内部使用的全局变量

//利用首参数和ebp的位置获得ebp值
void **getEBP(u4 dummy)
{
	void **myebp = (void **)&dummy - 2;
	return(*myebp);
}

i4 CDebug_backtrace(void **array,u4 size)
{
	u4 cnt = 0;
	u4 dummy=0;
	i4 dist;
	void **myebp = getEBP(dummy);

	while (myebp&&cnt < size)
	{
		dist = (u4)ADVANCE_STACK_FRAME (myebp)-(u4)myebp;
		if (dist<0|| dist>(MAX_STACK_FRAME_SIZE))
			break;
		array[cnt++] = RETURN_ADDRESS(myebp);
		myebp = ADVANCE_STACK_FRAME(myebp);
	}
	return cnt;
}
#endif

i4 CDebugPrintf(const char *format, ...)
{
#if(CDEBUG_EN)
	va_list ap;
	u4 len;
	CDEBUGPARAM* para = &m_Cdebugparam;
	if (CDEBUG_IOMODE_NOUT ==para->iomode)
	{
		return	0;
	}
	va_start(ap, format); len=vsprintf(para->string, format, ap); va_end(ap);
	assert(len < MAX_CDEBUG_BUFF_SIZE);
	switch (para->iomode)
	{
	case CDEBUG_IOMODE_STDOUT:
#ifdef CDEBUG_STDOUT
		fprintf(CDEBUG_STDOUT, para->string);
#endif
		break;
	case CDEBUG_IOMODE_FLIE:
#ifdef CDEBUG_FILE
		fprintf(para->fp, para->string);
		fflush(para->fp);
#endif
		break;
	default:
		break;
	}
	return len;
#else
	return 0;
#endif
}



u4 CDebugErrExport(u4 errcode)
{
#if(CDEBUG_EN)
	void* btaddr[MAX_STACK_FRAME_NUM];
	CDEBUGPARAM* para;
	u2 i,cnt;
	u4 len;
	if (CKNL_ERROR_NONE==errcode)
	{
		return CKNL_ERROR_NONE;
	}
	para = &m_Cdebugparam;
	para->lasterror = errcode;
	if (CDEBUG_IOMODE_NOUT == para->iomode)
	{
		return	errcode;
	}
	cnt = CDebug_backtrace(btaddr, MAX_STACK_FRAME_NUM);
	
	len = sprintf(para->string, "[ERROR %d] ", errcode);
	i = 1;//从序号1开始打印(不打印序号0表示的CDebugErrExport函数)
	while (i<cnt)
	{
		len += sprintf(para->string+ len,"[0x%p] ", btaddr[i]);
		i++;
	}
	len = sprintf(para->string + len, "\r\n");

	switch (para->iomode)
	{
	case CDEBUG_IOMODE_STDOUT:
#ifdef CDEBUG_STDOUT
		fprintf(CDEBUG_STDOUT, para->string);
#endif
		break;
	case CDEBUG_IOMODE_FLIE:
#ifdef CDEBUG_FILE
		fprintf(para->fp, para->string);
		fflush(para->fp);
#endif
		break;
	default:
		break;
	}
#endif
	return errcode;
}


u1 CDebugSetIOMODE(u4 mode)
{
#if(CDEBUG_EN)
	CDEBUGPARAM* para = &m_Cdebugparam;
	para->iomode = mode;
#endif
	return CKNL_ERROR_NONE;
}
void CDebugInit(void)
{
#if(CDEBUG_EN)
	CDEBUGPARAM* para =&m_Cdebugparam;

	para->string = csysmem_alloc(MAX_CDEBUG_BUFF_SIZE);
#ifdef CDEBUG_FILE
	para->fp = fopen(CDEBUG_FILE, "w+");
#endif
	para->lasterror = CKNL_ERROR_NONE;
	para->iomode = CDEBUG_INIT_IOMODE;
#endif
}

void CDebugStop(void)
{
#if(CDEBUG_EN)
	CDEBUGPARAM* para = &m_Cdebugparam;

	csysmem_free(para->string);
#ifdef CDEBUG_FILE
	fclose(para->fp);
#endif
#endif
}