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
x86 CPU ebp  ��չ��ַָ�� �൱��ջ��ָ��
        esp  ջ��ָ��(�Ըߵ�ַΪ�ײ���ѹջ���ֵ��С)
		eip  cpu��һ��ָ���ָ��
ջ�ṹ: ---�ߵ�ַ--- ����N
					...
					����1
					EIP
	   ----��һ֡------
					EBP
					��ʱ����1
					...
					��ʱ����N
	  
					����N
	   ----�͵�ַ---
*/

/*
δʵ��
ARM CPU fp  ��չ��ַָ�� �൱��ջ��ָ��
		sp  ջ��ָ��(�Ըߵ�ַΪ�ײ���ѹջ���ֵ��С)
		lr  cpu��һ��ָ���ָ��(��������ָ��)
		pc  ��ǰ���ú���ָ��
ջ�ṹ: ---�ߵ�ַ--- PC
					LR
					SP
					FP
					����1
					...
					����N
	   ----��һ֡------
					PC
	   ----�͵�ַ---
*/

#if(CDEBUG_EN)

#define ADVANCE_STACK_FRAME(ebp)	(*(ebp))
#define RETURN_ADDRESS(ebp)			*((ebp)+1)
#define GETEBP_BY_PARAM(param)		((void **)(&(param))-2)

typedef struct __CDEBUGPARAM {
	char* string;		//ָ����ʱ������������ݻ�����
#ifdef CDEBUG_FILE
	FILE* fp;			//ָ�����ڴ洢debug��Ϣ���ļ�
#endif
	u4	  lasterror;	//��һ������Ĵ������
	u4	  iomode;		//debug����ӿ�
} CDEBUGPARAM;

CDEBUGPARAM  m_Cdebugparam;//�����ڲ�ʹ�õ�ȫ�ֱ���

//�����ײ�����ebp��λ�û��ebpֵ
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
	i = 1;//�����1��ʼ��ӡ(����ӡ���0��ʾ��CDebugErrExport����)
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