libqi
=====

libqi is a middle-ware framework that provides RPC, type-erasure,
cross-language interoperability, OS abstractions, logging facilities,
asynchronous task management, dynamic module loading.

Example
-------

The following example shows some features of the framework, please refer to the
documentation for further details.

.. code-block:: cpp

  #include <boost/make_shared.hpp>
  #include <qi/log.hpp>
  #include <qi/applicationsession.hpp>
  #include <qi/anyobject.hpp>

  qiLogCategory("myapplication");

  class MyService
  {
  public:
    void myFunction(int val) {
      qiLogInfo() << "myFunction called with " << val;
    }
    qi::Signal<int> eventTriggered;
    qi::Property<float> angle;
  };

  // register the service to the type-system
  QI_REGISTER_OBJECT(MyService, myFunction, eventTriggered, angle);

  void print()
  {
    qiLogInfo() << "print was called";
  }

  int main(int argc, char* argv[])
  {
    qi::ApplicationSession app(argc, argv);

    // connect the session included in the app
    app.start();

    qi::SessionPtr session = app.session();

    // register our service
    session->registerService("MyService", boost::make_shared<MyService>());

    // get our service through the middleware
    qi::AnyObject obj = session->service("MyService");

    // call myFunction
    obj.call<void>("myFunction", 42);

    // call print in 2 seconds
    qi::async(&print, qi::Seconds(2));

    // block until ctrl-c
    app.run();
  }

You can then run the program with:

.. code-block:: console

  ./myservice --qi-standalone # for a standalone server
  ./myservice --qi-url tcp://somemachine:9559 # to connect to another galaxy of sessions

Links
-----

git repository:
http://github.com/aldebaran/libqi

Mailing list:
https://groups.google.com/a/aldebaran-robotics.com/group/qibuild-dev/topics

Documentation:
https://community.aldebaran-robotics.com/doc/libqi/

Maintainers:

- Philippe DAOUADI <pdaouadi@aldebaran.com>
- Cedric GESTES <gestes@aldebaran.com>
