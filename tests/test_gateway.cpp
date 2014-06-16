/*
 ** Author(s):
 **  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <string>

#include <gtest/gtest.h>

#include <qi/session.hpp>
#include <qi/messaging/servicedirectory.hpp>
#include <qi/messaging/gateway.hpp>

TEST(QiGateway, testConnection)
{
  qi::Session session;
  qi::ServiceDirectory sd;
  qi::Gateway gw;

  sd.listen("tcp://127.0.0.1:0");

  bool attached = gw.attachToServiceDirectory(sd.endpoints()[0]);
  ASSERT_TRUE(attached);

  gw.listen("tcp://127.0.0.1:0");

  bool connect = session.connect(gw.endpoints()[0]);
  ASSERT_TRUE(connect);
}
