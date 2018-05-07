/*
**  Copyright (C) 2017 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_TEST_MOCKUTILS_HPP
#define QI_TEST_MOCKUTILS_HPP

#pragma once

#include <gmock/gmock.h>
#include <ka/scoped.hpp>
#include <functional>

using ScopedVoidFunction = decltype(ka::scoped(std::function<void()>{}));

template <typename Mock>
ScopedVoidFunction scopeMockExpectations(Mock& mock) // TODO C++14: just return auto
{
   // TODO C++14: pass the lambda directly to scoped
  std::function<void()> fun = [&]{ EXPECT_TRUE(testing::Mock::VerifyAndClearExpectations(&mock)); };
  return ka::scoped(std::move(fun));
}

#endif
