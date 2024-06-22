/* file: test.c */

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>


/* ia + ib */
int ret_int(int ia, int ib)
{
    return ia + ib;
}

/* 定义个 TEST_GROUP, 名称为 sample */
TEST_GROUP(sample)
{};

/* 定义一个属于 TEST_GROUP 的 TEST, 名称为 ret_int_success */
TEST(sample, ret_int_success)
{
    int sum = ret_int(1, 2);
    CHECK_EQUAL(sum, 3);
}

/* 定义一个属于 TEST_GROUP 的 TEST, 名称为 ret_int_failed */
TEST(sample, ret_int_failed)
{
    int sum = ret_int(1, 2);
    CHECK_EQUAL(sum, 4);
}

int main(int argc, char *argv[])
{
    CommandLineTestRunner::RunAllTests(argc, argv);
    return 0;
}