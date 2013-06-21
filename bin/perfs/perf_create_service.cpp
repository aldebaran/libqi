/**
 * Aldebaran Robotics (c) 2010 All Rights Reserved
 *
 */

#include <iostream>
#include <vector>
#include <string>

#include <qimessaging/session.hpp>
#include <qitype/anyobject.hpp>

//#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qiperf/dataperfsuite.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

qi::AnyObject genObject() {
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply1", &reply);
  ob.advertiseMethod("reply2", &reply);
  ob.advertiseMethod("reply3", &reply);
  ob.advertiseMethod("reply4", &reply);
  ob.advertiseMethod("reply5", &reply);
  ob.advertiseMethod("reply6", &reply);
  ob.advertiseMethod("reply7", &reply);
  return ob.object();
}

int main(int argc, char **argv) {
  qi::Session session;
  qi::ServiceDirectory sd;
  sd.listen("tcp://127.0.0.1:0");

  std::string url = sd.endpoints()[0].str();
  session.connect(url);
  session.listen("tcp://0.0.0.0:0");

  std::string fname;
  if (argc > 2)
    fname = argv[2];
  qi::DataPerfSuite out("qimessaging", "perf_create_service", qi::DataPerfSuite::OutputData_MsgPerSecond, fname);
  qi::DataPerf dp;

  dp.start("create_service", 5000);

  for (unsigned int i = 0; i < 5000; ++i) {
    std::stringstream ss;
    ss << "servicetest-"  << i;
    std::cout << "Trying to register " << ss.str() << std::endl;
    // Wait for service id, otherwise register is asynchronous.
    qi::Future<unsigned int> idx = session.registerService(ss.str(), genObject());
    if (idx == 0)
      exit(1);
    std::cout << "registered " << ss.str() << " on " << idx << std::endl;
  }

  dp.stop();
  out << dp;
  out.close();
}
