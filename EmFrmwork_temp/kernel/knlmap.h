/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : knlmap.H
* By      : wangchao
* Version : V0.1.1
* Date    : 2016-09-14
* --------------------
*  kernel function mapped from common OS 
*********************************************************************************************************
*/
#pragma once
#ifndef KNLMAP_H
#define KNLMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <windows.h>

typedef LPVOID								TDPARA;                /* Thread Para for  Thread            */
#define TDOUT								DWORD WINAPI           /* Thread output for  Thread          */
typedef LPTHREAD_START_ROUTINE				TDFUNC;
#define SEM_MAX_COUNT						65536					/* max count of the Semaphore        */

#define thread_t							HANDLE
#define CThreadCreate_t(func,threadpara)	CreateThread(NULL,threadpara->stacksize,func,threadpara->param,0,NULL)
#define CThreadDel_t(t)						do{WaitForSingleObject(t,10000);CloseHandle(t);}while(0)
#define CSetThreadPriority(pthread,prio)	SetThreadPriority(pthread,prio)
#define CSEM								HANDLE
#define CSemCreate(SemNum)					CreateSemaphore(NULL,SemNum,SEM_MAX_COUNT,NULL)
#define CSemDel(s)							CloseHandle(s)
#define CSemPost(s)							ReleaseSemaphore(s,1,NULL)
#define CSemPend(s)							WaitForSingleObject(s,INFINITE)

#define CMUTEX								CRITICAL_SECTION
#define CMutexCreate(m)						InitializeCriticalSection(&m)
#define CMutexDel(m)						DeleteCriticalSection(&m)
#define CMutexLock(m)						EnterCriticalSection(&m)
#define CMutexUnlock(m)						LeaveCriticalSection(&m)

#define CKNL_CR								CRITICAL_SECTION
#define CKNL_CRTICAL_INIT()					do{G_KNRCR_MASK=0;CMutexCreate(G_KNR_CR);}while(0)
#define CKNL_ENTER_CRITICAL()				do{if(!G_KNRCR_MASK){CMutexLock(G_KNR_CR);}G_KNRCR_MASK++;}while(0)
#define CKNL_EXIT_CRITICAL()				do{G_KNRCR_MASK--;if(!G_KNRCR_MASK){CMutexUnlock(G_KNR_CR);}}while(0)

#define CAS __sync_bool_compare_and_swap
#elif defined(LINUX) 
#include <pthread.h>

#define thread_t							pthread_t
#define CThreadCreate_t(func,threadpara)	do{pthread_attr_setstacksize(&attr, stksize);pthread_create(&tid,&attr,func,threadpara->param);}while(0)//?
#define CThreadDel_t(t)						pthread_join(t,NULL)

#define CSEM								sem_t
#define CSemCreate(SemNum)					sem_init(&sid,1,SemNum) //?
#define CSemDel(s)							sem_destroy(s)
#define CSemPost(s)							sem_post(s)
#define CSemPend(s)							sem_wait(s)

#define CMUTEX								pthread_mutex_t
#define CMutexCreate(m)						pthread_mutex_init(m,NULL)
#define CMutexDel(m)						pthread_mutex_destroy(m)
#define CMutexLock(m)						pthread_mutex_lock(m)
#define CMutexUnlock(m)						pthread_mutex_unlock(m)

#define CKNL_CR								unsigned int
#define CKNL_CRTICAL_INIT()					do{G_KNRCR_MASK=0;G_KNR_CR=0;}while(0)
#define CKNL_ENTER_CRITICAL()				do{if(!G_KNRCR_MASK){local_irq_save(&G_KNR_CR);}G_KNRCR_MASK++;}while(0)
#define CKNL_EXIT_CRITICAL()				do{G_KNRCR_MASK--;if(!G_KNRCR_MASK){local_irq_restore(&G_KNR_CR);}}while(0)

#define CAS __sync_bool_compare_and_swap
#elif defined(UCOS) 
#include  <Source\ucos_ii.h>

#define thread_t							INT8U
#define CThreadCreate_t(func,threadpara)	OSTaskCreate(func,&threadpara->param,threadpara->stksize,threadpara->prio)  //?
#define CThreadDel_t(t)						OSTaskDel(t)

#define CSEM								OS_EVENT*
#define CSemCreate(SemNum)					OSSemCreate(SemNum)
#define CSemDel(s)							OSSemDel(s,OS_DEL_NO_PEND,&perr) //?
#define CSemPost(s)							OSSemPost(s)
#define CSemPend(s)							OSSemPend(s,0,&perr)//?

#define CMUTEX								OS_EVENT*
#define CMutexCreate(m)						OSMutexCreate(prio,&perr)
#define CMutexDel(m)						OSMutexDel(m,OS_DEL_NO_PEND,&perr)//?
#define CMutexLock(m)						OSMutexPend(m,0,&perr)//?
#define CMutexUnlock(m)						OSMutexPost(m)

#define CKNL_CR								INT32U
#define CKNL_CRTICAL_INIT()					do{G_KNRCR_MASK=0;G_KNR_CR=0;}while(0)
#define CKNL_ENTER_CRITICAL()				do{if(!G_KNRCR_MASK){G_KNR_CR=OSCPUSaveSR();}G_KNRCR_MASK++;}while(0)
#define CKNL_EXIT_CRITICAL()				do{G_KNRCR_MASK--;if(!G_KNRCR_MASK){OSCPURestoreSR(G_KNR_CR);}}while(0)

#elif defined(SYSBIOS) 
#include <ti/sysbios/BIOS.h>

#define thread_t							Task_Handle
#define CThreadCreate_t(func,threadpara)	Task_create((Task_FuncPtr)func,threadpara,&eb)  //?
#define CThreadDel_t(t)						OSTaskDel(t)

#define CSEM								Semaphore_Handle
#define CSemCreate(SemNum)					Semaphore_create(SemNum,&semParams, null) //?
#define CSemDel(s)							Semaphore_delete(s) 
#define CSemPost(s)							Semaphore_post(s)
#define CSemPend(s)							Semaphore_pend(s,BIOS_WAIT_FOREVER)//?

#define CMUTEX								GateMutexPri_Handle
#define CMutexCreate(f)						GateMutexPri_Create(&semParams, &eb)
#define CMutexDel(m)						GateMutexPri_delete(m)
#define CMutexLock(m)						GateMutexPri_enter(m)//?
#define CMutexUnlock(m)						GateMutexPri_leave(m,key)//?

#define CKNL_CR								UInt

#define CKNL_CRTICAL_INIT()					do{G_KNRCR_MASK=0;G_KNR_CR=0;}while(0)
#define CKNL_ENTER_CRITICAL()				do{if(!G_KNRCR_MASK){G_KNR_CR=Hwi_disable();}G_KNRCR_MASK++;}while(0)
#define CKNL_EXIT_CRITICAL()				do{G_KNRCR_MASK--;if(!G_KNRCR_MASK){Hwi_restore(G_KNR_CR);}}while(0)

#else

#endif

#ifdef __cplusplus
}
#endif

#endif /* KNLMAP_H */