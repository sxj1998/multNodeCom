/**
 * @file cppUtest_serial_test.cpp
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>


/**
 * @brief 测试GROUP bus_serial_driver_test
 * 
 */
TEST_GROUP(bus_serial_driver_test)
{};

/**
 * @brief 
 * 
 */
TEST(bus_serial_driver_test, ret_int_success)
{

    CHECK_EQUAL(3, 3);
}

TEST(bus_serial_driver_test, ret_int_failed)
{

    CHECK_EQUAL(4, 4);
}

int main(int argc, char *argv[])
{
    CommandLineTestRunner::RunAllTests(argc, argv);
    return 0;
}