/*
 *  Author(s):
 *  - Pierre Roullon <proullon@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <list>
#include <iostream>

#include <gtest/gtest.h>

#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>

int print(std::list<std::pair<std::string, int> > robots)
{
  for(std::list<std::pair<std::string, int> >::iterator it = robots.begin(); it != robots.end(); ++it)
    std::cout << "Robot " << (*it).first << " has serial ID " << (*it).second << std::endl;

  return robots.size();
}

TEST(TestCall, CallComplexType)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::GenericObjectBuilder ob;
  int serviceID;

  ob.advertiseMethod("print", &print);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::ObjectPtr proxy = p.client()->service("serviceCall");

  robots.push_back(std::make_pair("Gibouna", 1234567));
  robots.push_back(std::make_pair("Wall-E", 2345678));
  robots.push_back(std::make_pair("Billy West", 3456789));
  robots.push_back(std::make_pair("33CL", 4567890));

  ASSERT_ANY_THROW(proxy->call<int>("print", robots));

  p.server()->unregisterService(serviceID);
}
