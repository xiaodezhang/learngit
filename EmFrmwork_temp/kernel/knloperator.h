/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : knloperator.H
* By      : wangchao
* Version : V0.01
* Date    : 2016-11-07
* --------------------
*  define some operators for knl and user
*********************************************************************************************************
*/
#ifndef KNLOPT_H
#define KNLOPT_H


#define TO_STR(X)		#X
#define STR_PASTE(X,Y)	X##Y

#define dim(x)	sizeof(x)/sizeof(x[0])

//使用mask做bit设置
#define SETBIT(dest,mask)     (dest |= mask)
#define CLRBIT(dest,mask)     (dest &= ~mask)
#define TGLBIT(dest,mask)     (dest ^= mask)
#define CHKBIT(dest,mask)     (dest & mask)

//使用第k个bit做bit设置
#define setBit(dest,k)    (dest |= (1<<(k)))
#define clrBit(dest,k)    (dest &= ~(1<<(k)))
#define tglBit(dest,k)	  (dest ^= (1<<(k)))
#define chkBit(dest,k)	  (dest & (1<<(k)))
 


#endif /* KNLTYPE_H */