/*
 * Copyright (c) 2017 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef QI_TEST_HELPTEXT_HPP___
#define QI_TEST_HELPTEXT_HPP___

#include <string>
#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/applicationsession.hpp>

namespace {

  static const char* someText = "This is a text";

  class TestHelpText : public ::testing::Test
  {
  protected:
    // The following arguments are voluntarily write-enabled to allow passing them to application construction.
    int argc = 1;
    std::string argvStorage = someText;
    char* args = &argvStorage[0];
    char** argv = &args;
  };

}

#endif
