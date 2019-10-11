/*
** Copyright (C) 2017 SoftBank Robotics
*/

#ifndef QI_TEST_OBJECT_HPP___
#define QI_TEST_OBJECT_HPP___
#pragma once

#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4068, pragmas)
KA_WARNING_DISABLE(, parentheses)
#include <qi/anyfunction.hpp>
KA_WARNING_POP()

namespace {

  qi::GenericFunctionParameters args(
    qi::AutoAnyReference p1 = qi::AutoAnyReference(),
    qi::AutoAnyReference p2 = qi::AutoAnyReference(),
    qi::AutoAnyReference p3 = qi::AutoAnyReference())
  {
    qi::GenericFunctionParameters res;
    if (p1.type()) res.push_back(p1); else return res;
    if (p2.type()) res.push_back(p2); else return res;
    if (p3.type()) res.push_back(p3); else return res;
    return res;
  }

}

#endif

