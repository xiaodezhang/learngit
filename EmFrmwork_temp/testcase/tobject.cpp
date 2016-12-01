#include"..\kernel\cknl.h"
#include"gtest\gtest.h"

#define OBJECTTESTTIMES 20
#define ARG1 1
#define ARG2 2
#define PERIOD 100

typedef struct{
    COBJ_HANDLE_LOOPTHREAD_DEF(1,1);
    i4 data;
}TESTOBJECT;

static int arg1;
static int arg2;

class ObjectTest : public ::testing::Test,
    public ::testing::WithParamInterface<int>{

protected:
    virtual void SetUp(){
        printf("start testing object\n");

    }
public:
    static u1 sendmessage(CMSG*);
    static u1 getmessage(CMSG*);
    void memtest();
    u1 tInit(TESTOBJECT*,u4,const CTHREADPARAM*,CCBFUNC);
    TESTOBJECT* tCreate(u4,const CTHREADPARAM*,CCBFUNC);
    void tDel(TESTOBJECT*);
    const CTHREADPARAM g_testPara[3] = {
        15,0,2048,0,
        11,0,2048,0,
        10,0,2048,0
    };
    TESTOBJECT *sendobj;
    TESTOBJECT *getobj;
};

void ObjectTest::tDel(TESTOBJECT* ptest){

    MyHhandler_DEL(ptest,0);
    MyObjReleaseReq(ptest);
    MyThread_Del(ptest);
    cmem_free(ptest);
}

void ObjectTest::memtest(){

#if 1
    u4 t;
    t = cmem_GetUsedMem();
    ASSERT_EQ(0,t);
    t = csysmem_GetUsedMem();
    ASSERT_EQ(0,t);
    t = CMSG_GetUsedMem();
    ASSERT_EQ(0,t);
    t = CTimer_GetUsedMem();
    ASSERT_EQ(0,t);
#endif
}

u1 ObjectTest::sendmessage(CMSG* pmsg){

    TESTOBJECT *tobj;
    CHANDLER* tar_handler;

    tobj = MyPointer_GET(pmsg,TESTOBJECT);
    tar_handler = tgtHandler_GET(tobj,0);
    tobj->data = MSG_D1_GET(pmsg,i4)+MSG_D2_GET(pmsg,i4);
    EXPECT_EQ(ARG1,MSG_D1_GET(pmsg,i4));
    EXPECT_EQ(ARG2,MSG_D2_GET(pmsg,i4));

    return 0;
}

u1 ObjectTest::getmessage(CMSG* pmsg){

    TESTOBJECT *tobj;
    CHANDLER* tar_handler;

    tobj = MyPointer_GET(pmsg,TESTOBJECT);
    tar_handler = tgtHandler_GET(tobj,0);
    tobj->data = MSG_D1_GET(pmsg,i4)+MSG_D2_GET(pmsg,i4);
    EXPECT_EQ(arg1,MSG_D1_GET(pmsg,i4));
    EXPECT_EQ(arg2,MSG_D2_GET(pmsg,i4));
    return 0;
}

TESTOBJECT* ObjectTest::tCreate(u4 id,const CTHREADPARAM* threadparam,CCBFUNC pfun){

    TESTOBJECT* pTemp;
    pTemp = (TESTOBJECT*)cmem_alloc(sizeof(TESTOBJECT));
    tInit(pTemp,id,threadparam, pfun);
    return pTemp;
}

u1 ObjectTest::tInit(TESTOBJECT* obj,u4 id,const CTHREADPARAM* threadparam,
        CCBFUNC pfun){

    MyObj_Init(obj,id);
    MyLoopThread_Init(obj,threadparam);
    MyHandler_Init_MyThread(obj,0,id,pfun);
    MyHandler_InsertSys(obj,0);
    tgtHandler_Init(obj,0);
    obj->data = 0;

    return 0;
}


TEST_P(ObjectTest,SendAndGetTest){

    CMSG *msg;

    {
        SCOPED_TRACE("");
        memtest();
    }
    if(HasFatalFailure())
        return;
    arg1 = GetParam();
    arg2 = GetParam();
    sendobj = tCreate(0,&g_testPara[0],sendmessage);
    getobj = tCreate(1,&g_testPara[1],getmessage);
    tgtHandler_Conect_Temp(sendobj,0,MyHandler_GET(getobj,0));
    msg = CobtainMsgStatic(MyHandler_GET(getobj,0),0);
    CWriteMessage(msg,arg1,arg2);
    CsendMessage(msg);
    CrecycleMessage(msg);
    tDel(sendobj);
    tDel(getobj);
    {
        SCOPED_TRACE("");
        memtest();
    }
}

#if 1
TEST_F(ObjectTest,TimingSendTest){

    CMSG *msg;

    {
        SCOPED_TRACE("");
        memtest();
    }
    if(HasFatalFailure())
        return;
    sendobj = tCreate(0,&g_testPara[0],sendmessage);
    getobj = tCreate(1,&g_testPara[1],getmessage);
    tgtHandler_Conect_Temp(sendobj,0,MyHandler_GET(getobj,0));
    msg = CobtainMsgStatic(MyHandler_GET(sendobj,0),0);
    CWriteMessage(msg,ARG1,ARG2);
    CsendMessageTiming(msg,PERIOD);
    while(1){
        if(sendobj->data == ARG1+ARG2){
           CdelMessageTiming(msg);
           CctrlMessageTiming(msg,CTIMER_OPERATOR_STOP);
           break;
        }
        else
           CctrlMessageTiming(msg,CTIMER_OPERATOR_RUN);
    }
    CrecycleMessage(msg);
    tDel(sendobj);
    tDel(getobj);
    {
        SCOPED_TRACE("");
        memtest();
    }
}

#endif
INSTANTIATE_TEST_CASE_P(TOBJECT,ObjectTest,
        ::testing::Range(1,OBJECTTESTTIMES));
