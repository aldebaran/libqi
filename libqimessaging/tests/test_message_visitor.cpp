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

#include <qimessaging/signature.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/serialization/message_visitor.hpp>
#include <qimessaging/serialization/message_copy_visitor.hpp>
#include <gtest/gtest.h>

typedef std::vector<std::string>          StringVect;
typedef std::map<std::string, StringVect> StringVectMap;

TEST(TestMessageVisitor, Basic)
{
  qi::DataStream msg;
  int           i = 0;
  std::string   s = "paf";
  StringVectMap m;
  StringVect    vs;
  std::string   sig;

  qi::signature<int>::value(sig);
  qi::signature<std::string>::value(sig);
  qi::signature<StringVectMap>::value(sig);

  vs.push_back(std::string("titi"));
  m["titi"] = vs;

  msg << i;
  msg << s;
  msg << m;

  qi::serialization::MessageVisitor mv(msg, sig.c_str());
  mv.visit();
  //EXPECT_EQ()
}

TEST(TestMessageCopyVisitor, Basic)
{
  qi::DataStream msg1;
  qi::DataStream msg2;

  int           i = 42;
  std::string   s = "paf";
  StringVectMap m;
  StringVect    vs;
  std::string   sig;
  qi::signature<int>::value(sig);
  qi::signature<std::string>::value(sig);
  qi::signature<StringVectMap>::value(sig);

  EXPECT_EQ("is{s[s]}", sig);
  std::cout << "signature:" << sig << std::endl;
  vs.push_back(std::string("bar1"));
  vs.push_back(std::string("bar2"));
  m["foo"] = vs;
  msg1 << i;
  msg1 << s;
  msg1 << m;

  qi::serialization::MessageCopyVisitor<qi::DataStream, qi::DataStream> mv(msg1, msg2, sig.c_str());
  mv.visit();

  std::cout << "finished visit" << std::endl;
  int i2;
  std::string s2;
  std::map<std::string, std::vector<std::string> > m2;

  msg2 >> i2;
  msg2 >> s2;
  msg2 >> m2;

  EXPECT_EQ(42, i2);
  EXPECT_EQ("paf", s2);

  ASSERT_TRUE(m2.begin() != m2.end());
  EXPECT_EQ("foo", m2.begin()->first);
  EXPECT_EQ("bar1", m2.begin()->second[0]);
  EXPECT_EQ("bar2", m2.begin()->second[1]);
}
