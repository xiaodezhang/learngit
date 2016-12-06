#include"..\kernel\cknl.h"
#include"gtest\gtest.h"

#define GQUEUESIZE 200
#define QUEUETESTTIMES 10
#define QUEUEDATATIMES 10

class QueueTest : public testing::Test,
  public ::testing::WithParamInterface<int>{

    protected:
        virtual void SetUp(){
            printf("start testing queue\n");
        }
    public:
        u1 gQueue[GQUEUESIZE];
        CQUE q;
};

TEST_P(QueueTest,PostAndPendTest){

    u4 i,*geti,posti[QUEUEDATATIMES];
    int param;

    param = GetParam();
    ASSERT_FALSE(CQCreate(&q,&gQueue,GQUEUESIZE));

    for(i = 0;i < QUEUEDATATIMES;i++){
        posti[i] = i*param;
        ASSERT_FALSE(CQPost(&q,(void*)&posti[i]));
    }
    for(i = 0;i < QUEUEDATATIMES;i++){
        geti = (u4*)CQPend(&q);
        EXPECT_EQ(i*param,*geti);
    }
}

INSTANTIATE_TEST_CASE_P(TQUEUE,QueueTest,
        ::testing::Range(1,QUEUETESTTIMES));

