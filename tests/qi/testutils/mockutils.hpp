/*
**  Copyright (C) 2017 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_TEST_MOCKUTILS_HPP
#define QI_TEST_MOCKUTILS_HPP

#pragma once

#include <gmock/gmock.h>
#include <qi/scoped.hpp>
#include <functional>

using ScopedVoidFunction = decltype(qi::scoped(std::function<void()>{}));

template <typename Mock>
ScopedVoidFunction scopeMockExpectations(Mock& mock) // TODO C++14: just return auto
{
   // TODO C++14: pass the lambda directly to scoped
  std::function<void()> fun = [&]{ EXPECT_TRUE(testing::Mock::VerifyAndClearExpectations(&mock)); };
  return qi::scoped(std::move(fun));
}

#endif
