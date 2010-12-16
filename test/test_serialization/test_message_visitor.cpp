/*
** test_message_visitor.cpp
** Login : <ctaf42@cgestes-de2>
** Started on  Fri Dec 10 13:43:33 2010 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>

#include <qi/serialization.hpp>
#include <qi/serialization/message_visitor.hpp>
#include <qi/serialization/message_copy_visitor.hpp>
#include <gtest/gtest.h>

TEST(TestMessageVisitor, Basic)
{
  qi::serialization::Message msg;
  qi::serialization::MessageVisitor mv(msg, "is");

  int i = 0;
  std::string s = "paf";
  std::map<std::string, std::vector<std::string> > m;
  std::vector<std::string> vs;

  vs.push_back(std::string("titi"));

  m["titi"] = vs;
  qi::serialization::serialize<int>::write(msg, i);
  qi::serialization::serialize<std::string>::write(msg, s);
  qi::serialization::serialize< std::map<std::string, std::vector<std::string> > >::write(msg, m);

  mv.visit();
  //EXPECT_EQ()
}

TEST(TestMessageCopyVisitor, Basic)
{
  qi::serialization::Message msg1;
  qi::serialization::Message msg2;

  int i = 42;
  std::string s = "paf";
  qi::serialization::serialize<int>::write(msg1, i);
  qi::serialization::serialize<std::string>::write(msg1, s);

  qi::serialization::MessageCopyVisitor<qi::serialization::Message, qi::serialization::Message> mv(msg1, msg2, "is");
  mv.visit();

  std::cout << "finished visit" << std::endl;
  int i2;
  std::string s2;

  qi::serialization::serialize<int>::read(msg2, i2);
  qi::serialization::serialize<std::string>::read(msg2, s2);

  EXPECT_EQ(42, i2);
  EXPECT_EQ("paf", s2);
}
