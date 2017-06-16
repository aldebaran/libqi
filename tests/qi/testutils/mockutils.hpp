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

template <typename Mock>
auto scopeMockExpectations(Mock& mock)
  -> decltype(qi::scoped<std::function<void()>>({})) // TODO C++14: remove this line
{
   // TODO C++14: pass the lambda directly to scoped
  std::function<void()> fun = [&]{ EXPECT_TRUE(testing::Mock::VerifyAndClearExpectations(&mock)); };
  return qi::scoped(std::move(fun));
}

#endif
