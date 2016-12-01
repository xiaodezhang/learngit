// EmFrmwork_temp.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include"kernel\cknl.h"
#include"testcase\testcase.h"
#include"gtest\gtest.h"
#if 0
static int Foo(int a,int b){

    int c;

    if(a == 0 || b == 0)
        throw "don't do that";
    if(!(c = a % b))
        return b;

    return Foo(b,c);
}

TEST(FooTest,HandleNoneZeroInput){

    EXPECT_EQ(2,Foo(4,10));
    ASSERT_EQ(2,Foo(4,10));
    EXPECT_EQ(6,Foo(30,18));
}
TEST(FooTest,HandleNoneZeroInput2){

    EXPECT_EQ(2,Foo(4,10));
    ASSERT_EQ(2,Foo(4,10));
    EXPECT_EQ(6,Foo(30,18)) << "卢萨卡地激发卢萨卡地方";
}
#endif

int main(int argc,char *argv[])
{
    CknlInit();
#if 0
	//testcmemory();
	//test_linklist();
	test_object();
    system("pause");
    return 0;
#endif
#if 1
	testing::GTEST_FLAG(output) = "xml:";
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
#endif

}

