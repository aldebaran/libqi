/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
*/

#include <iostream>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <boost/shared_ptr.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qiperf/dataperfsuite.hpp>
#include <testsession/testsessionpair.hpp>

int iteration = 100000;
static qi::Atomic<int> callbackValue;

void callback()
{
  return; //this is a dummy callback.
}

void cb()   //used for callback test
{
  ++callbackValue;
}

class Service { //This is a test Class.
public:
  Service() {
  }
public:
  qi::Signal<void ()> ping;
};

void advertise_event(int iteration) //test  event advertising.
{
  qi::GenericObjectBuilder myObjectPointerBuilder;
  std::stringstream eventName;
  eventName << "testEvent";

  for(int i = 0; i < iteration; i++)
  {
    eventName << i;
    myObjectPointerBuilder.advertiseSignal<void (int)>(eventName.str());
    eventName.str(std::string());
  }
}

void advertise_method(int iteration)// test method advertising.
{
  for (int i = 1; i < iteration; i++)
  {
    qi::GenericObjectBuilder myObjectPointerBuilder;
    myObjectPointerBuilder.advertiseMethod("callback", &callback);
  }
}

void connect_event(int iteration)//test event connection perf.
{
  qi::GenericObjectBuilder myObjectPointerBuilder;
  qi::Link eventId = myObjectPointerBuilder.advertiseSignal<void (int)>("testEvent");
  qi::Link callbackID = myObjectPointerBuilder.advertiseMethod("callback", &callback);
  qi::ObjectPtr myObjectPointer = myObjectPointerBuilder.object();
  qi::Link myLinkId;

  for (int i = 1; i < iteration; i++)
  {
    myLinkId = myObjectPointer->connect(eventId, myObjectPointer, callbackID);
    myObjectPointer->disconnect(myLinkId);
  }
}

void emit_event_without_session(int iteration)//test emit-event perf without session.
{
  qi::ObjectTypeBuilder<Service> obt;
  obt.advertiseSignal("ping", &Service::ping);

  qi::ObjectPtr myObjectPointer = qi::ObjectPtr(new qi::GenericObject(obt.type(), new Service));

  for (int i = 1; i < iteration; i++)
  {
    myObjectPointer->post("ping");
  }
}

struct MyTuple
{
 int i;
 float f;
 std::string st;
 double d;
};

QI_TYPE_STRUCT(MyTuple, i, f, st, d); //Allow Mytuple to be sent with events.

template <typename T>
void emit_event(int iteration, const T &param)//test int emit-event perf.
{
  TestSessionPair  p;

  qi::GenericObjectBuilder myObjectPointerBuilder;
  myObjectPointerBuilder.advertiseSignal<void (T)>("testEvent");
  qi::ObjectPtr myObjectPointer = myObjectPointerBuilder.object();

  if(!p.server()->registerService("service", myObjectPointer).wait())
  {
    throw std::runtime_error("Impossible to register Service");
  }

  for (int i = 1; i < iteration; i++)
  {
    myObjectPointer->post("testEvent", param);
  }
}

void emit_event(int iteration)//test emit empty events.
{
  TestSessionPair  p;

  qi::ObjectTypeBuilder<Service> obt;
  obt.advertiseSignal("ping", &Service::ping);
  qi::ObjectPtr myObjectPointer = qi::ObjectPtr(new qi::GenericObject(obt.type(), new Service));

  if(!p.server()->registerService("service", myObjectPointer).wait())
  {
    throw std::runtime_error("Impossible to register Service");
  }

  for (int i = 1; i < iteration; i++)
  {
    myObjectPointer->post("ping");
  }
}

void test_callback()  //test callbacks performances without session
{
  qi::ObjectTypeBuilder<Service> obt; //advertising Event
  obt.advertiseSignal("ping", &Service::ping);

  qi::ObjectPtr myObjectPointer = qi::ObjectPtr(new qi::GenericObject(obt.type(), new Service));

  myObjectPointer->connect("ping", &cb); //connecting event to callback
  for (int i=0; i < iteration; i++) //Emiting Events
  {
    myObjectPointer->post("ping");
  }
  while(*callbackValue < iteration)//waiting callbacks
  {
  }
  return;
}

void test_callback_session(qi::DataPerfSuite &out ,std::string testname) //testing callback with session.
{
  TestSessionPair  p;
  qi::DataPerf dp;
  qi::ObjectPtr oclient;

  qi::ObjectTypeBuilder<Service> obt;  //building service2
  obt.advertiseSignal("ping", &Service::ping);
  qi::ObjectPtr oserver = qi::ObjectPtr(new qi::GenericObject(obt.type(), new Service));
  if(!p.server()->registerService("service", oserver).wait())
  {
    throw std::runtime_error("Impossible to register Service");
  }
  oserver->connect("ping", &cb);
  oclient = p.client()->service("service"); //geting the proxy

  dp.start(testname, iteration);

  for (int i=0; i < iteration; i++) //Emiting Events
  {
    oclient->post("ping");
  }

  while (*callbackValue < iteration) //waiting callbacks.
  {
  }

  dp.stop();
  out << dp;

  return;
}

int main(int argc, char* argv[])
{
  qi::log::setVerbosity(qi::log::fatal);
  std::string fname;
  if (argc > 2)
    fname = argv[2];

  if (argc == 2) //If you want to adjust the number juste use ./perf_event [iteration]
    iteration = atoi(argv[1]);

  qi::DataPerfSuite out("qimessaging", "event_functions", qi::DataPerfSuite::OutputData_MsgPerSecond, fname);
  qi::DataPerf dp;

 //~~~~~~~~~~~~~~~~~~~~~Basic Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  dp.start("advertise-event", iteration);
  advertise_event(iteration);
  dp.stop();
  out << dp;

  dp.start("advertise-method", iteration);
  advertise_method(iteration);
  dp.stop();
  out << dp;

  dp.start("connect-event", iteration);
  connect_event(iteration);
  dp.stop();
  out << dp;

  //~~~~~~~~~~~~~~~ Event tests(SD and direct mode)~~~~~~~~~~~~~~~~~
  // TODO: Add other mode.

  //empty event without session
  dp.start("emit-event-without-SD", iteration);
  emit_event_without_session(iteration);
  dp.stop();
  out << dp;

  //empty event
  TestMode::forceTestMode(TestMode::Mode_Direct);
  dp.start("emit-empty-event-Direct", iteration);
  emit_event(iteration);
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_SD);
  dp.start("emit-empty-event-SD", iteration);
  emit_event(iteration);
  dp.stop();
  out << dp;

  //int event
  TestMode::forceTestMode(TestMode::Mode_Direct);
  dp.start("emit-int-event-direct", iteration);
  emit_event(iteration, 0);
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_SD);
  dp.start("emit-int-event-SD", iteration);
  emit_event(iteration, 0);
  dp.stop();
  out << dp;

  //string event
  TestMode::forceTestMode(TestMode::Mode_Direct);
  dp.start("emit-string-event-Direct", iteration);
  emit_event(iteration, "okidoki");
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_SD);
  dp.start("emit-string-event-SD", iteration);
  emit_event(iteration, "okidoki");
  dp.stop();
  out << dp;

  //tuple event
  MyTuple foo = {1, 2.0, "Test", 1};

  TestMode::forceTestMode(TestMode::Mode_Direct);
  dp.start("emit-tuple-event-Direct", iteration);
  emit_event(iteration, foo);
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_SD);
  dp.start("emit-tuple-event-SD", iteration);
  emit_event(iteration, foo);
  dp.stop();
  out << dp;

  // ~~~~~~~~~~~ CallBacks Tests ~~~~~~~~~~
  dp.start("callback", iteration);
  test_callback();
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_Direct);
  callbackValue = 0;
  test_callback_session(out, "session_callback_direct");

  TestMode::forceTestMode(TestMode::Mode_SD);
  callbackValue = 0;
  test_callback_session(out, "session_callback_SD");

  // ~~~~~~~~~~~~~~~This is not Functional Yet ~~~~~~~~~~~~~~~~~
  /*TestMode::forceTestMode(TestMode::Mode_Gateway);
  callbackValue = 0;
  dp.start("session_callback_Gateway",iteration);
  test_session_cb();
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_RemoteGateway);
  callbackValue = 0;
  dp.start("session_callback_Remote_Gateway",iteration);
  test_session_cb();
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_ReverseGateway);
  callbackValue = 0;
  dp.start("session_callback_Reverse_Gateway",iteration);
  test_session_cb();
  dp.stop();
  out << dp;


  TestMode::forceTestMode(TestMode::Mode_SSL);
  callbackValue = 0;
  dp.start("session_callback_SSL",iteration);
  test_session_cb();
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_Nightmare); // Not Functional
  callbackValue = 0;
  dp.start("session_callback_nightmare",iteration);
  test_session_cb();
  dp.stop();
  out << dp;

  TestMode::forceTestMode(TestMode::Mode_NetworkMap);
  callbackValue = 0;
  dp.start("session_callback_networkmap",iteration);
  test_session_cb();
  dp.stop();
  out << dp;
  */
  out.close();
}
