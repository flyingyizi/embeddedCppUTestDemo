/*
 * test.cpp
 *
 *  Created on: Sep 24, 2019
 *      Author: tu_xu
 */

#include "CppUTest/TestHarness.h"
TEST_GROUP(FirstTestGroup)
    {};

TEST(FirstTestGroup, FirstTest)
    {
        FAIL("fail me!");
    }
