/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/session.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>

qiLogCategory("qiservice");

std::string reply(const std::string &msg) {
  qiLogInfo() << "Message recv:" << msg;
  return msg + "bim";
}

qi::GenericValue reply(const qi::GenericValue &myval) {
  static int i = 0;
  qi::GenericValuePtr val(myval);
  qiLogInfo() << i++ << " Message received with the signature: " << myval.signature(false) << " = " << qi::encodeJSON(val) << std::endl;
  return myval;
}

std::string reply(const int &msg) {
  qiLogInfo() << "Message recv:" << msg;
  std::stringstream ss;

  ss << msg << "bim";
  return ss.str();
}

std::string reply(const std::string &msg, const double &value) {
  qiLogInfo() << "Message recv:" << msg << " * " << value;
  std::stringstream ss;

  ss << msg << value;
  return ss.str();
}

std::string reply(const std::string &msg, const float &value) {
  qiLogInfo() << "Message recv:" << msg << " * " << value;
  std::stringstream ss;

  ss << msg << value;
  return ss.str();
}

typedef std::vector<std::string> VS;
typedef std::map<std::string, VS> MSM;

VS replyVector() {
  VS ret;
  return ret;
  ret.push_back("titi");
  ret.push_back("tutu");
  return ret;
}

std::map<std::string, std::string> replyMap() {
  std::map<std::string, std::string> ret;
  return ret;
  ret["titi"] = "42";
  ret["tutu"] = "41";
  return ret;
}


MSM replyMap2() {
  MSM ret;
  VS v;

  return ret;
  v.push_back("titi");
  v.push_back("tutu");

  ret["titi"] = v;
  ret["tutu"] = v;
  return ret;
}

bool slip(unsigned int seconds) {
  qi::os::sleep(seconds);
  return true;
}

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  const std::string serviceName = "serviceTest";

  qi::Application app(argc, argv);
  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:9559")),
       "The master address");

  // allow master address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("master-address", 1);

  // Test hostIPAddrs
  std::map<std::string, std::vector<std::string> > ifsMap = qi::os::hostIPAddrs();

  if (ifsMap.empty())
   qiLogInfo();

  if (ifsMap.empty() == false)
    for (std::map<std::string, std::vector<std::string> >::const_iterator adapter = ifsMap.begin();
         adapter != ifsMap.end();
         ++adapter)
    {
      for (std::vector<std::string>::const_iterator address = (*adapter).second.begin();
           address != (*adapter).second.end();
           ++address)
      {
        qiLogInfoF("%s : %s", (*adapter).first.c_str(), (*address).c_str());
      }
    }

  // parse and store
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      qiLogInfo() << desc;
      return 0;
    }

    if (vm.count("master-address") == 1)
    {
      std::string masterAddress = vm["master-address"].as<std::string>();
      qi::Session       session;
      qi::GenericObjectBuilder ob;
      ob.advertiseMethod<std::string (const std::string&)>("reply", &reply);
      ob.advertiseMethod<std::string (const int&)>("reply", &reply);
      ob.advertiseMethod<std::string (const std::string&, const double &)>("reply", &reply);
      ob.advertiseMethod<std::string (const std::string&, const float &)>("reply", &reply);
      ob.advertiseMethod("replyVector", &replyVector);
      ob.advertiseMethod("replyMap", &replyMap);
      ob.advertiseMethod("replyMap2", &replyMap2);
      ob.advertiseMethod<qi::GenericValue (const qi::GenericValue&)>("reply", &reply);
      ob.advertiseSignal<void (const std::string&)>("testEvent");
      ob.advertiseMethod<bool (unsigned int)>("slip", &slip);
      qi::ObjectPtr obj(ob.object());

      session.connect(masterAddress);

      session.listen("tcp://0.0.0.0:0");
      session.setIdentity("tests/server.key", "tests/server.crt");
      try {
        session.listen("tcps://0.0.0.0:0");
      } catch (std::runtime_error &) {
        qiLogWarning() << "SSL desactivated.";
      }

      unsigned int id = session.registerService(serviceName, obj);

#if 0
      // test unregistration
      session.unregisterService(id);
      id = session.registerService("serviceTest", &obj);
#endif

      if (id)
      {
        qiLogInfo() << "Registered \"" << serviceName << "\" as service (#" << id << ") with the master";
      }
      else
      {
        qiLogError() << "Registration with master failed, aborting...";
        exit(1);
      }
      int i = 0;
      while (true) {
        std::stringstream ss;
        ss << "miam" << i++;
        obj->post("testEvent", ss.str());
        ss.str(std::string());
        qi::os::sleep(1);
      }

      session.unregisterService(id);
      session.close();
    }
    else
    {
      qiLogInfo() << desc;
    }
  }
  catch (const boost::program_options::error&)
  {
    qiLogInfo() << desc;
  }

  return 0;
}
