
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <boost/timer.hpp>
#include <qi/messaging.hpp>
#include <qimessaging/exceptions.hpp>
#include "alvalue.pb.h"

using namespace qi;

static int gGlobalResult = 0;

char        echo_char(const char& b) {return b;}
bool        echo_bool(const bool& b) {return b;}
int         echo_int(const int& i) {return i;}
float       echo_float(const float& f) {return f;}
double      echo_double(const double& b) {return b;}
std::string echo_string(const std::string& s) {return s;}
std::string echo_string1(const std::string& s1, const std::string& s2) {return s1;}
std::string echo_string2(const std::string& s1, const std::string& s2) {return s2;}
ALCompat::ALValue echo_myawesomeness(const ALCompat::ALValue &crunch) { return crunch; }

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

  // two nasty cases
  std::vector<std::string> fun0vvvvv()                                                              { std::vector<std::string>s; return s; }
  void vfun0c() const                                                                               { gGlobalResult = 0; }
};


Master master;
Server server;
Client client;

TEST(MessagingSignatures, setup)
{
  master.run();
  server.connect();
  client.connect();
}

TEST(MessagingSignatures, echo_bool)
{
  bool b = true;
  server.advertiseService("echo", &echo_bool);
  bool r = client.call<bool>("echo", b);
  ASSERT_EQ(b, r);
}

TEST(MessagingSignatures, echo_int)
{
  int b = 42;
  server.advertiseService("echo", &echo_int);
  int r = client.call<int>("echo", b);
  ASSERT_EQ(b, r);
}

TEST(MessagingSignatures, echo_float)
{
  float b = 42.42f;
  server.advertiseService("echo", &echo_float);
  float r = client.call<float>("echo", b);
  ASSERT_EQ(b, r);
}

TEST(MessagingSignatures, echo_string)
{
  std::string b = "42";
  server.advertiseService("echo", &echo_string);
  std::string r = client.call<std::string>("echo", b);
  ASSERT_EQ(b, r);
}

TEST(MessagingSignatures, echo_string1)
{
  std::string a = "hello";
  std::string b = "world";
  server.advertiseService("echo", &echo_string1);
  std::string r = client.call<std::string>("echo", a, b);
  ASSERT_EQ(a, r);
}

TEST(MessagingSignatures, echo_string2)
{
  std::string a = "hello";
  std::string b = "world";
  server.advertiseService("echo", &echo_string2);
  std::string r = client.call<std::string>("echo", a, b);
  ASSERT_EQ(b, r);
}


//TEST(MessagingSignatures, DISABLE_echo_double)
//{
//  double b = 987986889.87987987979789;
//  server.advertiseService("echo", &echo_double);
//  double r = client.call<double>("echo", b);
//  ASSERT_EQ(b, r);
//}

TEST(MessagingSignatures, echo_char)
{
  char b = 'c';
  server.advertiseService("echo", &echo_char);
  char r = client.call<char>("echo", b);
  ASSERT_EQ(b, r);
}

TEST(MessagingSignatures, echo_myawesomeness)
{
  ALCompat::ALValue miam;
  //miam.PrintDebugString();
  server.advertiseService("echo", &echo_myawesomeness);
  miam.set_type(ALCompat::ALValue::STRING);
  miam.set_stringvalue("I like It");
  ALCompat::ALValue r = client.call<ALCompat::ALValue>("echo", miam);
  //r.PrintDebugString();
  ASSERT_EQ(miam.stringvalue(), r.stringvalue());
  ASSERT_EQ(miam.type(), r.type());
}

TEST(MessagingSignatures, allFunctorsBindAndCall)
{
  server.advertiseService("vfun0", &vfun0);
  server.advertiseService("vfun1", &vfun1);
  server.advertiseService("vfun2", &vfun2);
  server.advertiseService("vfun3", &vfun3);
  server.advertiseService("vfun4", &vfun4);
  server.advertiseService("vfun5", &vfun5);
  server.advertiseService("vfun6", &vfun6);

  server.advertiseService("fun0", &fun0);
  server.advertiseService("fun1", &fun1);
  server.advertiseService("fun2", &fun2);
  server.advertiseService("fun3", &fun3);
  server.advertiseService("fun4", &fun4);
  server.advertiseService("fun5", &fun5);
  server.advertiseService("fun6", &fun6);

  Foo f;
  server.advertiseService("foo.vfun0", &f, &Foo::vfun0);
  server.advertiseService("foo.vfun1", &f, &Foo::vfun1);
  server.advertiseService("foo.vfun2", &f, &Foo::vfun2);
  server.advertiseService("foo.vfun3", &f, &Foo::vfun3);
  server.advertiseService("foo.vfun4", &f, &Foo::vfun4);
  server.advertiseService("foo.vfun5", &f, &Foo::vfun5);
  server.advertiseService("foo.vfun6", &f, &Foo::vfun6);

  server.advertiseService("foo.fun0", &f, &Foo::fun0);
  server.advertiseService("foo.fun1", &f, &Foo::fun1);
  server.advertiseService("foo.fun2", &f, &Foo::fun2);
  server.advertiseService("foo.fun3", &f, &Foo::fun3);
  server.advertiseService("foo.fun4", &f, &Foo::fun4);
  server.advertiseService("foo.fun5", &f, &Foo::fun5);
  server.advertiseService("foo.fun6", &f, &Foo::fun6);

  // KABOOOM!!! std::vector<std::string>
  //server.addService("foo.fun0vvvvv", &f, &Foo::fun0vvvvv);

  // KABOOOM!!! const
  //server.addService("foo.vfun0c", &f, &Foo::vfun0c);

  // KABOOOM!!! char*
  //client.call("foo.echo", "jsalkdjljasd");

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

  //avoid "unused var" warning
  (void) i0;
  (void) i1;
  (void) i2;
  (void) i3;
  (void) i4;
  (void) i5;
  (void) i6;

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

  //avoid "unused var" warning
  (void) fi0;
  (void) fi1;
  (void) fi2;
  (void) fi3;
  (void) fi4;
  (void) fi5;
  (void) fi6;
}


TEST(MessagingSignatures, MethodOverloading)
{
  server.advertiseService("overload.fun1", &fun1);
  server.advertiseService("overload.fun1", &fun2);
  int r2 = client.call<int>("overload.fun1", 1, 2);
  ASSERT_EQ(3, r2);
  int r1 = client.call<int>("overload.fun1", 1);
  ASSERT_EQ(1, r1);
}

TEST(MessagingSignatures, MultipleBind)
{
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
  server.advertiseService("multiple.fun1", &fun1);
}

TEST(MessagingSignatures, paramTypeChecking)
{
  server.advertiseService("typechecking.fun1", &fun1);
  int r2 = client.call<int>("typechecking.fun1", 1);
  (void) r2;
  try {
    int r3 = client.call<int>("typechecking.fun1", std::string("anything"));
    (void) r3;
  } catch (const qi::transport::ServiceNotFoundException& e) {
    std::cout << "ServiceNotFoundException: " << e.what() << std::endl;
  }
}

TEST(MessagingSignatures, paramNumChecking)
{
  server.advertiseService("paramnumchecking.fun1", &fun1);
  int r2 = client.call<int>("paramnumchecking.fun1", 1);
  (void) r2;
  try {
    int r3 = client.call<int>("paramnumchecking.fun1", 1, 2);
    (void) r3;
  } catch (const qi::transport::ServiceNotFoundException& e) {
    std::cout << "ServiceNotFoundException: " << e.what() << std::endl;
  }
}

TEST(MessagingSignatures, returnTypeChecking)
{
  server.advertiseService("returntype.fun1", &fun1);
  int r2 = client.call<int>("returntype.fun1", 1);
  (void) r2;
  try {
    std::string s = client.call<std::string>("returntype.fun1", 1);
    (void) s;
  } catch (const qi::transport::ServiceNotFoundException& e) {
    std::cout << "ServiceNotFoundException: " << e.what() << std::endl;
  }
}
