#include"..\kernel\cknl.h"
#include"gtest\gtest.h"

#define BLOCKSIZE 1024

class CmemTest : public ::testing::Test,
    public ::testing::WithParamInterface<int>{

        public:
            u1* testmemfast;
            u1* testmemslow;
        protected:
            virtual void SetUp(){
                printf("Start testing mem\n");
            }
};

TEST_P(CmemTest,AllocTest){

    int memsize = GetParam();
    ASSERT_NE((u1*)NULL,(testmemfast = (u1*)cmem_alloc(memsize)));
    EXPECT_EQ(memsize,cmem_GetUsedMem());
    ASSERT_EQ(0,cmem_free(testmemfast));
    EXPECT_EQ(0,cmem_GetUsedMem());
}


#if 1
typedef CmemTest CmemDeathTest;

TEST_F(CmemDeathTest,NegativeTest){
    ASSERT_DEATH(cmem_alloc(-1),"");
    ASSERT_DEATH(cmem_alloc(-1024),"");
    ASSERT_DEATH(cmem_alloc(-0.1),"");
//    ASSERT_DEATH(cmem_alloc(0),"");
}
#endif
#if 0
TEST_P(CmemTest,GetusedTest){

}
#endif

INSTANTIATE_TEST_CASE_P(Tmem,CmemTest,
        ::testing::Range(1*BLOCKSIZE,10*BLOCKSIZE,BLOCKSIZE));

