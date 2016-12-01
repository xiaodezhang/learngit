#include"..\\kernel\cknl.h"
#include"gtest\gtest.h"
#include<stdlib.h>

#define MAX_TEST_LINK_NUM 16
#define TESTTIMES 10
#define DATATIMES 10


typedef struct{
    u4 ObjID;
    u4 data;
}Stestlink;

typedef struct{
    u4 data;
    void* next;
}Simplelink;

class LinklistTest : public ::testing::Test,
    public ::testing::WithParamInterface<int>{

        protected:
            virtual void SetUp(){
                CListCreate(g_testlink, MAX_TEST_LINK_NUM, sizeof(Stestlink));
                printf("start testing linklist\n");
            }
        public:
            Stestlink tempdata[DATATIMES];
            u1 g_testlink[CLINKLIST_SIZE(MAX_TEST_LINK_NUM,
                    sizeof(Stestlink))];
           static u1 CTestIDcompare(const DPARA ObjID, const DPARA handler) {

                  return (u4)ObjID == ((CHANDLER*)handler)->ObjID;
            }


       private:
           u1 TestListTraverse(Stestlink * plinkdata) {

                  printf("ListTraverse ID=%d\r\n",plinkdata->ObjID);
                  return 0;
            }
};

class SimpleListTest : public ::testing::Test,
    public ::testing::WithParamInterface<int>{

        protected:
            virtual void SetUp(){

            }
        public:
            Simplelink gSimpleLink[MAX_TEST_LINK_NUM];
};

TEST_P(LinklistTest,AddAndRemoveTest){

    u2 param = GetParam();
    void *ptr;

    tempdata[0].ObjID = param;
    tempdata[0].data = param*param;
    ptr = CListAdd(g_testlink,&tempdata[0],sizeof(Stestlink),2);
    ASSERT_TRUE(ptr);
    EXPECT_EQ(1,GetListNum(g_testlink));
    EXPECT_EQ(0,CListRemove(g_testlink, (DPARA)tempdata[0].ObjID, (CGcompare)CTestIDcompare));
}

TEST_P(SimpleListTest,AllTest){

#if 0
    u2 param = GetParam();
#endif
    u2 param = 1;
    Simplelink *plist,*listHead;
    u4 i,listnum,err;

    plist = gSimpleLink;
    CSimpleListInit((void**)&listHead);
    for(i = 0;i < DATATIMES;i++,plist++){
        plist->data = i;
        CSimpleListAddTail((void**)&listHead,plist,offsetof(Simplelink,next));
        listnum = GetSimpleListNum((void**)&listHead,offsetof(Simplelink,next));
        EXPECT_EQ(i+1,listnum);
    }
    
    plist = listHead;
    for(i = 0;i < DATATIMES-1;i++){
        ASSERT_TRUE(plist);
        EXPECT_EQ(i,plist->data);
        plist = (Simplelink*)CSimpleListNext(plist,offsetof(Simplelink,next));
        ASSERT_TRUE(plist);
        EXPECT_EQ(i+1,plist->data);
    }

    for(i = 0;i < DATATIMES;i++){
        plist = &gSimpleLink[(i+5)%DATATIMES];
        err = CSimpleListRemovePtr((void**)&listHead,plist,offsetof(Simplelink,next));
        EXPECT_FALSE(err);
    }
}

TEST_P(LinklistTest,FindTest){

    u2 param = GetParam();
    u4 i,randid;
    void *ptr;
    Stestlink* finddata;

    for(i = 0;i < DATATIMES;i++){
        tempdata[i].ObjID = param*i;
        tempdata[i].data = param*param*i*i;
        ptr = CListAdd(g_testlink,&tempdata[i],sizeof(Stestlink),2);
        ASSERT_TRUE(ptr);
    }
    randid = tempdata[(int)((float)rand()/RAND_MAX*DATATIMES)].ObjID;
    finddata = (Stestlink*)CListFind(g_testlink,(DPARA)randid,CTestIDcompare);
    EXPECT_EQ(randid*randid,finddata->data);
}

INSTANTIATE_TEST_CASE_P(TLINKLIST,LinklistTest,
        ::testing::Range(1,TESTTIMES));
INSTANTIATE_TEST_CASE_P(TLINKLIST,SimpleListTest,
        ::testing::Range(1,TESTTIMES));


