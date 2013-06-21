/*
 ** Author(s):
 **  - Laurent Lec <llec@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>

int    _argc;
char** _argv;

TEST(Test, TestApplicationDestruction)
{
  qi::ServiceDirectory     sd;
  qi::Session              server, client;
  qi::DynamicObjectBuilder ob1;
  qi::AnyObject            oclient1;

  {
    qi::Application app(_argc, _argv);

    sd.listen("tcp://127.0.0.1:0");
    server.connect(sd.endpoints()[0].str());
    server.listen("tcp://0.0.0.0:0");
    client.connect(sd.endpoints()[0].str());

    qi::AnyObject    oserver1(ob1.object());

    server.registerService("coin1", oserver1).wait();

    oclient1 = client.service("coin1");
    // will fall with the others client.close();
    server.close();
    sd.close();
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  _argc = argc;
  _argv = argv;
  return RUN_ALL_TESTS();
}
