/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>

#include <qi/application.hpp>
#include <qi/atomic.hpp>
#include <qi/shared_ptr.hpp>

class TestSharedPtr
{
public:
    TestSharedPtr(bool* state)
        : _state(state)
    {
        *_state = true;
    }

    ~TestSharedPtr()
    {
        *_state = false;
    }

private:
    bool* _state;
};

/* NOTE: This whole code can and will receive SIGSEGV if SharedPtr is not coded
 *       correctly. This is INTENDED. Be careful when fixing segmentation fault
 *       problems here. Do it if you are SURE the code of the test is not
 *       correct.
 */

TEST(QiSharedPtr, simple_test)
{
    bool state = false;
    {
        qi::SharedPtr<TestSharedPtr> s(new TestSharedPtr(&state));
        ASSERT_TRUE(state);
        *s;
    }
    ASSERT_FALSE(state);
}

TEST(QiSharedPtr, simple_copy)
{
    bool state = false;
    {
        qi::SharedPtr<TestSharedPtr> s1(new TestSharedPtr(&state));
        ASSERT_TRUE(state);
        *s1;
        {
            qi::SharedPtr<TestSharedPtr> s2(s1);
            *s2;
        }
        ASSERT_TRUE(state);
    }
    ASSERT_FALSE(state);
}

TEST(QiSharedPtr, simple_equality)
{
    bool states[2] = {false, false};
    {
        qi::SharedPtr<TestSharedPtr> s1(new TestSharedPtr(states));
        ASSERT_TRUE(states[0]);
        *s1;
        {
            qi::SharedPtr<TestSharedPtr> s2(new TestSharedPtr(states + 1));
            ASSERT_TRUE(states[1]);
            *s2;

            // Check call to operator=
            s1 = s2;
            ASSERT_FALSE(states[0]);
            ASSERT_TRUE(states[1]);
            *s1;
            *s2;
        }
        ASSERT_TRUE(states[1]);
        *s1;
    }
    ASSERT_FALSE(states[0]);
    ASSERT_FALSE(states[1]);
}

int main(int argc, char** argv)
{
    qi::Application app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
