// souitest.cpp
// main在gtest.lib里
//
//参考 http://www.cnblogs.com/coderzh/archive/2009/04/10/1432789.html
/*
--gtest_filter 用法

对执行的测试案例进行过滤，支持通配符
?    单个字符
*    任意字符
-    排除，如，-a 表示除了a
:    取或，如，a:b 表示a或b
比如下面的例子：

./foo_test 没有指定过滤条件，运行所有案例
./foo_test --gtest_filter=* 使用通配符*，表示运行所有案例
./foo_test --gtest_filter=FooTest.* 运行所有“测试案例名称(testcase_name)”为FooTest的案例
./foo_test --gtest_filter=*Null*:*Constructor* 运行所有“测试案例名称(testcase_name)”或“测试名称(test_name)”包含Null或Constructor的案例。
./foo_test --gtest_filter=-*DeathTest.* 运行所有非死亡测试案例。
./foo_test --gtest_filter=FooTest.*-FooTest.Bar 运行所有“测试案例名称(testcase_name)”为FooTest的案例，但是除了FooTest.Bar这个案例
*/

#include <gtest/gtest.h>  

int Add(int a,int b){return a+b;}

TEST(Add, 负数) {  
	EXPECT_EQ(Add(-1,-2), -3);  
	EXPECT_GT(Add(-4,-5), -6); // 故意的  
}  

TEST(Add, 正数) {  
	EXPECT_EQ(Add(1,2), 3);  
	EXPECT_GT(Add(4,5), 6);  
} 
