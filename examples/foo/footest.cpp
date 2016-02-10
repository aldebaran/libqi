#include <qi/applicationsession.hpp>
#include <qi/anymodule.hpp>

using namespace qi;

int main(int argc, char** argv) {
  ApplicationSession app(argc, argv);

  //connect the session
  app.startSession();

  //## register the Foo object as a service
  app.session()->loadService("foo", "Foo");

  //## Or you can do that by hand
  // import the module
  AnyModule foomod = qi::import("foo");
  // create a Foo object
  AnyObject ao = foomod.call<AnyObject>("Foo", app.session());
  // register the object on the Session with the name "Foo"
  app.session()->registerService("Foo", ao);

  app.run();
}
