
#include <gtest/gtest.h>
#include <alcommon-ng/common/common.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>
#include <alcommon-ng/exceptions/exceptions.hpp>
#include <boost/timer.hpp>
#include <string>

using namespace AL::Common;
using namespace AL::Messaging;

std::string gMasterAddress = "127.0.0.1:5555";
std::string gServerName = "server";
std::string gServerAddress = "127.0.0.1:5556";

static int gGlobalResult = 0;

void vfun0()                                                                                      { gGlobalResult = 0; }
void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }

int fun0()                                                                                      { return 0; }
int fun1(const int &p0)                                                                         { return p0; }
int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }

struct Foo {
  void voidCall()                                          { return; }
  int intStringCall(const std::string &plouf)              { return plouf.size(); }

  int fun0()                                                                                      { return 0; }
  int fun1(const int &p0)                                                                         { return p0; }
  int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
  int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
  int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
  int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
  int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }

  void vfun0()                                                                                      { gGlobalResult = 0; }
  void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
  void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
  void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
  void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
  void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
  void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }
};

MasterNode master(gMasterAddress);
ServerNode server(gServerName, gServerAddress, gMasterAddress);
ClientNode client("client", gMasterAddress);

TEST(NodeSignatures, allFunctorsBindAndCall)
{
  server.addService("vfun0", &vfun0);
  server.addService("vfun1", &vfun1);
  server.addService("vfun2", &vfun2);
  server.addService("vfun3", &vfun3);
  server.addService("vfun4", &vfun4);
  server.addService("vfun5", &vfun5);
  server.addService("vfun6", &vfun6);

  server.addService("fun0", &fun0);
  server.addService("fun1", &fun1);
  server.addService("fun2", &fun2);
  server.addService("fun3", &fun3);
  server.addService("fun4", &fun4);
  server.addService("fun5", &fun5);
  server.addService("fun6", &fun6);

  Foo f;
  server.addService("foo.vfun0", &f, &Foo::vfun0);
  server.addService("foo.vfun1", &f, &Foo::vfun1);
  server.addService("foo.vfun2", &f, &Foo::vfun2);
  server.addService("foo.vfun3", &f, &Foo::vfun3);
  server.addService("foo.vfun4", &f, &Foo::vfun4);
  server.addService("foo.vfun5", &f, &Foo::vfun5);
  server.addService("foo.vfun6", &f, &Foo::vfun6);

  server.addService("foo.fun0", &f, &Foo::fun0);
  server.addService("foo.fun1", &f, &Foo::fun1);
  server.addService("foo.fun2", &f, &Foo::fun2);
  server.addService("foo.fun3", &f, &Foo::fun3);
  server.addService("foo.fun4", &f, &Foo::fun4);
  server.addService("foo.fun5", &f, &Foo::fun5);
  server.addService("foo.fun6", &f, &Foo::fun6);

  client.callVoid("vfun0");
  client.callVoid("vfun1", 1);
  client.callVoid("vfun2", 1, 2);
  client.callVoid("vfun3", 1, 2, 3);
  client.callVoid("vfun4", 1, 2, 3, 4);
  client.callVoid("vfun5", 1, 2, 3, 4, 5);
  client.callVoid("vfun6", 1, 2, 3, 4, 5, 6);

  int i0 = client.call<int>("fun0");
  int i1 = client.call<int>("fun1", 1);
  int i2 = client.call<int>("fun2", 1, 2);
  int i3 = client.call<int>("fun3", 1, 2, 3);
  int i4 = client.call<int>("fun4", 1, 2, 3, 4);
  int i5 = client.call<int>("fun5", 1, 2, 3, 4, 5);
  int i6 = client.call<int>("fun6", 1, 2, 3, 4, 5, 6);

  client.callVoid("foo.vfun0");
  client.callVoid("foo.vfun1", 1);
  client.callVoid("foo.vfun2", 1, 2);
  client.callVoid("foo.vfun3", 1, 2, 3);
  client.callVoid("foo.vfun4", 1, 2, 3, 4);
  client.callVoid("foo.vfun5", 1, 2, 3, 4, 5);
  client.callVoid("foo.vfun6", 1, 2, 3, 4, 5, 6);

  int fi0 = client.call<int>("foo.fun0");
  int fi1 = client.call<int>("foo.fun1", 1);
  int fi2 = client.call<int>("foo.fun2", 1, 2);
  int fi3 = client.call<int>("foo.fun3", 1, 2, 3);
  int fi4 = client.call<int>("foo.fun4", 1, 2, 3, 4);
  int fi5 = client.call<int>("foo.fun5", 1, 2, 3, 4, 5);
  int fi6 = client.call<int>("foo.fun6", 1, 2, 3, 4, 5, 6);
}


TEST(NodeSignatures, MethodOverloading)
{
  server.addService("overload.fun1", &fun1);
  server.addService("overload.fun1", &fun2);
  int r2 = client.call<int>("overload.fun1", 1, 2);
  ASSERT_EQ(3, r2);
  int r1 = client.call<int>("overload.fun1", 1);
  ASSERT_EQ(1, r1);
}

TEST(NodeSignatures, MultipleBind)
{
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
  server.addService("multiple.fun1", &fun1);
}

TEST(NodeSignatures, paramTypeChecking)
{
  server.addService("typechecking.fun1", &fun1);
  int r2 = client.call<int>("typechecking.fun1", 1);
  try {
    int r3 = client.call<int>("typechecking.fun1", std::string("anything"));
  } catch (const AL::Transport::ServiceNotFoundException& e) {
    std::cout << "Service not found: what():" << e.what() << std::endl;
  }
}

//TEST(NodeSignatures, paramNumChecking)
//{
//  server.addService("paramnumchecking.fun1", &fun1);
//  int r2 = client.call<int>("paramnumchecking.fun1", 1);
//  int r3 = client.call<int>("paramnumchecking.fun1", 1, 2);
//}

//TEST(NodeSignatures, ReturnTypeChecking)
//{
//  server.addService("returntype.fun1", &fun1);
// KABOOOM!
//  int r2 = client.call<int>("returntype.fun1", 1);
//  client.call<std::string>("returntype.fun1", 1);
//}