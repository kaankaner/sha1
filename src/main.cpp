#include <gtest/gtest.h>
#include <iostream>

static int runAllTests(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 

int main(int argc, char **argv)
{
    std::cout << __FUNCTION__ << " begin..." << std::endl;

    int retCode = runAllTests(argc, argv);

    std::cout << __FUNCTION__ << " end..." << std::endl;

    return retCode;
}
