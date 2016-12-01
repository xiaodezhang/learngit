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

CKNL_CR G_KNR_CR; //系统进入关键字段的全局变量，不允许嵌套
u4		G_KNRCR_MASK;
//采用1个链表内存管理块，用于管理较小的快速内存获取，以及一个buddy算法管理块，用于较大的内存块获取
u1	CknlInit(void)
{
	CKNL_CRTICAL_INIT();
	cmemoryInit();
	csysmemoryInit();	//系统内存初始化
	CsystimerInit();	//定时器内存和功能初始化
	CDebugInit();		//debug模块初始化
	return 0;
}
