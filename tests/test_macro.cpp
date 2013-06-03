/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <qi/macro.hpp>
#include <qi/preproc.hpp>


class CloneOk
{
public:
  int i;
};

class CloneKo
{
public:
  int i;
private:
  QI_DISALLOW_COPY_AND_ASSIGN(CloneKo);
};

class CloneKo2: boost::noncopyable
{
  public:
  int i;
};
class CloneKo3: public boost::noncopyable
{
  public:
  int i;
};

TEST(Macro, clonable)
{
  EXPECT_TRUE(qi::isClonable<CloneOk>());
  EXPECT_TRUE(qi::isClonable((CloneOk*)0));
  EXPECT_TRUE(!qi::isClonable<CloneKo>());
  EXPECT_TRUE(!qi::isClonable((CloneKo*)0));
  EXPECT_TRUE(!qi::isClonable<CloneKo2>());
  EXPECT_TRUE(!qi::isClonable((CloneKo2*)0));
  EXPECT_TRUE(!qi::isClonable<CloneKo3>());
  EXPECT_TRUE(!qi::isClonable((CloneKo3*)0));
}
