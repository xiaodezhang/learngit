#include"..\\kernel\cknl.h"
#include"gtest\gtest.h"
#ifdef WIN32
#include<WinBase.h>
#endif
#include<stdlib.h>

#define TIMERTESTTIMES 10
#define CHECKDIFF 1000
#define RUNPERIOD (-1000)

unsigned long starttime;
unsigned long runstarttime;
int period;
volatile int stopflag;
volatile int runstopflag;



class TimerTest : public ::testing::Test,
    public ::testing::WithParamInterface<int>{

protected:
    virtual void SetUp(){

    }
    virtual void TearDown(){

    }
public:
    
};

static u1 timerfun(void *){

    unsigned long stoptime;

    stoptime = GetTickCount();
    stopflag = 1;
    EXPECT_TRUE(labs(stoptime-starttime-period) < CHECKDIFF);
    return 0;
}

static u1 runtimerfun(void *){

    unsigned long runstoptime;

    runstoptime = GetTickCount();
    runstopflag = 1;
    EXPECT_TRUE(labs(runstoptime-runstarttime-RUNPERIOD) < CHECKDIFF);
    return 0;
}

#if 1
TEST_P(TimerTest,OneShotTimeTest){
    
    CTIMER *ctimer;
    stopflag = 0;
    period = GetParam();
    starttime = GetTickCount();
    ctimer = CTimerCreate(GetSysTimerGroup(),timerfun,NULL,period,CTIMER_OPERATOR_ONESHOT);
    ASSERT_NE((void *)0,ctimer);
    CTimerUserLink(ctimer,GetSysTimerWheel());
    while(1){
        if(stopflag){
            break;
        }
    }
}
#endif

#if 1
TEST_F(TimerTest,RunTest){

    CTIMER *ctimer;
    runstopflag = 0;
    runstarttime = GetTickCount();
    ctimer = CTimerCreate(GetSysTimerGroup(),runtimerfun,NULL,RUNPERIOD,CTIMER_OPERATOR_RUN);
    ASSERT_NE((void *)0,ctimer);
    CTimerUserLink(ctimer,GetSysTimerWheel());
    while(1){
        if(runstopflag){
            CTimerCtrl(ctimer,CTIMER_OPERATOR_STOP);
            CTimerUserUnlinkDel(ctimer, GetSysTimerWheel());
            break;
        }
    }
}

#endif
INSTANTIATE_TEST_CASE_P(TTIMER,TimerTest,
        ::testing::Range(1000,TIMERTESTTIMES*1000,1000));

