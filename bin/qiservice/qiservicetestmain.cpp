/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <qi/applicationsession.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/jsoncodec.hpp>
#include <qi/anyfunction.hpp>


qiLogCategory("qiservice");

std::string reply(const std::string &msg) {
  qiLogInfo() << "Message recv:" << msg;
  return msg + "bim";
}

qi::AnyValue anyArgs(const qi::AnyVarArguments& aa) {
  (void)aa;
  return qi::AnyValue::from(42);
}

void anyArgs2(const qi::AnyVarArguments& aa) {
  (void)aa;
}

void anyArgs3(int i, const qi::AnyVarArguments& aa) {
  (void)i;
  (void)aa;
}

int anyArgs4(int i, const qi::VarArguments<int>& values) {
  int acc = i;
  for (unsigned i = 0; i < values.args().size(); ++i)
    acc += values.args().at(i);
  return acc;
}

qi::AnyValue reply(const qi::AnyValue &myval) {
  static int i = 0;
  qi::AnyReference val = qi::AnyReference::from(myval);
  qiLogInfo() << i++ << " Message received with the signature: " << myval.signature(false).toString() << " = " << qi::encodeJSON(val) << std::endl;
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

void error() {
  throw std::runtime_error("errrrrror from cpp");
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

  qi::ApplicationSession app(argc, argv);
  // declare the program options
  po::options_description desc("Usage:\n  qi-service --qi-url masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.");

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
              options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      qiLogInfo() << desc;
      return 0;
    }

    qi::SessionPtr           session = app.session();
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod<std::string (const std::string&)>("reply", &reply);
    ob.advertiseMethod<void ()>("error", &error);
    ob.advertiseMethod<std::string (const std::string&, const double &)>("reply", &reply);
    ob.advertiseMethod<std::string (const std::string&, const float &)>("reply", &reply);
    ob.advertiseMethod<qi::AnyValue (const qi::AnyValue&)>("reply", &reply);
    ob.advertiseMethod<std::string (const int&)>("reply", &reply);
    ob.advertiseMethod("replyVector", &replyVector);
    ob.advertiseMethod("replyMap", &replyMap);
    ob.advertiseMethod("replyMap2", &replyMap2);
    ob.advertiseSignal<const std::string&>("testEvent");
    ob.advertiseMethod<bool (unsigned int)>("slip", &slip);
    ob.advertiseMethod("anyArgs", &anyArgs);
    ob.advertiseMethod("anyArgs2", &anyArgs2);
    ob.advertiseMethod("anyArgs3", &anyArgs3);
    ob.advertiseMethod("anyArgs4", &anyArgs4);
    qi::AnyObject obj(ob.object());

    app.startSession();

    app.session()->listen("tcp://0.0.0.0:0");
    session->setIdentity("tests/server.key", "tests/server.crt");
    try {
      app.session()->listen("tcps://0.0.0.0:0");
    } catch (std::runtime_error &) {
      qiLogWarning() << "SSL desactivated.";
    }

    unsigned int id = session->registerService(serviceName, obj);

#if 0
    // test unregistration
    session->unregisterService(id);
    id = session->registerService("serviceTest", &obj);
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
      obj.post("testEvent", ss.str());
      qiLogInfo() << "Posting:" << ss.str();
      ss.str(std::string());
      qi::os::sleep(1);
    }

    session->unregisterService(id);
  }
  catch (const boost::program_options::error&)
  {
    qiLogInfo() << desc;
  }

  return 0;
}
