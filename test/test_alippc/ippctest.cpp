/**
 * This executable tries to use naoqiclient functions.
 *
 *
 */

#include <vector>
#include <iostream>

#include <alippc/ippc.hpp>
#include <stdlib.h> // For rand.
#include <alcore/alptr.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <alfindippc/ippc_service.hpp>
//#include <alproxies/almotionproxy.h>

#ifdef WIN32
  #include "Windows.h"
  #define SleepSeconds(s) Sleep(1000 * s);
#else
  #include <unistd.h>
  #define SleepSeconds(s) sleep(s);
#endif


using namespace AL;

/**
 * Parse arguments to build a ALNaoSimulationInterface object.
 * Call tests methods with the NaoQi object1
 */

class forCallBack :  public ippc::ServerCommandDelegate
{
public:
  void fonctionQuiPoutre()
  {
  }
  // to call function on current process
  AL::ALPtr<ippc::ResultDefinition> ippcCallback(const ippc::CallDefinition & def)
  {
    // receive fonctionQuiPoutre
    if (def.getMethodName() == "fonctionQuiPoutre")
    {
      fonctionQuiPoutre();
      return AL::ALPtr<ippc::ResultDefinition>();
    }
  }
};


int main(int argc, char *argv[] )
{
  // for result
  forCallBack callbackObject;

  // to receive call in ippc
  ALPtr<ippc::Server> fIppcServer = ALPtr<ippc::Server>(new ippc::Server("clientIPPC"));
  fIppcServer->setCommandDelegate(&callbackObject);
  boost::thread threadServer(boost::bind(&ippc::Server::run, fIppcServer.get()));

  // declare your process clientIPPC
  ALPtr<ippc::IppcService>  fIppcService=  ALPtr<ippc::IppcService>(new ippc::IppcService("clientIPPC"));
  boost::thread threadService(boost::bind(&ippc::IppcService::run, fIppcService.get()));

  // connect to parent
  fIppcService->setParentBroker("mainBroker");

  static ALValue listValName; // static to not allocate at each call
  listValName.arrayPush( "LHipPitch" );
  listValName.arrayPush( "RHipPitch" );
  listValName.arrayPush( "LKneePitch" );
  listValName.arrayPush( "RKneePitch" );
  listValName.arrayPush( "LAnklePitch" );
  listValName.arrayPush( "RAnklePitch" );
  listValName.arrayPush( "LHipRoll" );
  listValName.arrayPush( "LHipYawPitch" );

  static AL::ALValue listVal; // static to not allocate at each call
  //listVal = fMotionProxy->getAngles( listValName, true );

  // call insertData("yoippc",42)
  ippc::CallDefinition def(0);
  AL::ALPtr<ippc::ResultDefinition> res;
  def.setMethodName("getAngles");
  def.setModuleName("ALMotion");
  // for result but useless with void return
  def.setSender("clientIPPC");
  def.push(listValName);
  def.push(true);
  //def.push(false);
  // call function
  ippc::Client connection("mainBroker", fIppcServer->getResultHandler());
  res = connection.send(def);
  //ippc::VariableValue vcrash = res.value();
  //std::vector < float > valtest=  vcrash.as< std::vector < float >  >();

  ippc::VariableValue v = res->value();
  ALValue test = v.as< ALValue >();


  // int call
  ippc::CallDefinition def3(0);
  AL::ALPtr<ippc::ResultDefinition> res3;
  def3.setMethodName("getBrokerName");
  def3.setModuleName("ALMotion");
  def3.setSender("clientIPPC");
  ippc::Client connection3("mainBroker", fIppcServer->getResultHandler());
  res3 = connection3.send(def3);
  ippc::VariableValue v3 = res3->value();
  std::string strres3 =  v3.as< std::string >();

  // insert data
  // int call
  ippc::CallDefinition def4(0);
  AL::ALPtr<ippc::ResultDefinition> res4;
  def4.setMethodName("insertData");
  def4.setModuleName("ALMemory");
  def4.setSender("clientIPPC");
  def4.push(std::string("yoippc"));
  def4.push(42);
  ippc::Client connection4("mainBroker", fIppcServer->getResultHandler());
  res4 = connection4.send(def4);

  //ALValue aa =
  //std::vector<float> truc =  v.as< std::vector<float> >();
  /*
  /// call a function of our process but with ippc, just to show we receive fonctionQuiPoutre in callback ippcCallback
  ippc::CallDefinition def2(0);
  ippc::ResultDefinition res2;
  def2.setMethodName("fonctionQuiPoutre");
  def2.setModuleName("");
  ippc::Connection connection2("clientIPPC", fIppcServer->getResultHandler());
  connection2.send(def2);*/
  return 1;
}
