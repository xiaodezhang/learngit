/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cknl.c
* By      : wangchao
* Version : V0.01
* Date    : 2016-10-02
* --------------------
*  Initial for Cknl 
*********************************************************************************************************
*/
#include"cknl.h"

CKNL_CR G_KNR_CR; //ϵͳ����ؼ��ֶε�ȫ�ֱ�����������Ƕ��
u4		G_KNRCR_MASK;
//����1�������ڴ����飬���ڹ����С�Ŀ����ڴ��ȡ���Լ�һ��buddy�㷨����飬���ڽϴ���ڴ���ȡ
u1	CknlInit(void)
{
	CKNL_CRTICAL_INIT();
	cmemoryInit();
	csysmemoryInit();	//ϵͳ�ڴ��ʼ��
	CsystimerInit();	//��ʱ���ڴ�͹��ܳ�ʼ��
	CDebugInit();		//debugģ���ʼ��
	return 0;
}
