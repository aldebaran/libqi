/*
* Copyright (c) 2017 Aldebaran Robotics. All rights reserved.
* Use of this source code is governed by a BSD-style license that can be
* found in the COPYING file.
*/

#include "test_helptext.hpp"

TEST_F(TestHelpText, ApplicationSession)
{
  qi::ApplicationSession app(argc, argv);
  const auto helpText = app.helpText();
  EXPECT_FALSE(helpText.empty());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

