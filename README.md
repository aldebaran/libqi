libqi
=====

libqi is a middle-ware framework that provides RPC, type-erasure,
cross-language interoperability, OS abstractions, logging facilities,
asynchronous task management, dynamic module loading.

This library is published as part of the NAOqi SDK: http://doc.aldebaran.com/

Compilation
-----------

To compile libqi you need qibuild which give some cmake functions used
in libqi's CMakeLists.txt.

~~~~ sh

pip2 install --user qibuild

git clone git@github.com:aldebaran/libqi.git
cd libqi

mkdir BUILD && cd BUILD
cmake .. -DQI_WITH_TESTS=OFF
make
make install DESTDIR=./output

~~~~

Example
-------

The following example shows some features of the framework, please refer to the
documentation for further details.

~~~~ cpp

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

~~~~


You can then run the program with:

~~~~ sh

./myservice --qi-standalone # for a standalone server
./myservice --qi-url tcp://somemachine:9559 # to connect to another galaxy of sessions

~~~~

Versioning & Branching Strategy
--------------------------------

The current version is described in the CHANGELOG file.

We follow Semantic Versioning v2.0.0 : http://semver.org/

We follow Git Flow branching strategy: https://datasift.github.io/gitflow/IntroducingGitFlow.html
but for historical reasons we use the following branch names:

 - master: last stable release;
 - team/platform/dev: development branch;
 - tags: libqi-vX.Y.Z;
 - tags if published with a NAOqi release: NAOqi-vX.Y.Z;
 - next: compatibility breaking long-term changes branch;

All branches not following this naming strategy are legacy branches.
All tags not following the naming strategy are legacy tags
and matches NAOqi release version numbers.

Links
-----

git repository:
http://github.com/aldebaran/libqi

Documentation:
http://doc.aldebaran.com/libqi/

IRC Channel:
#qi on freenode.

Maintainers
-----------

 - Joël Lamotte <jlamotte@aldebaran.com>
 - Jérémy Monnon <jmonnon@aldebaran.com>
 - Matthieu Paindavoine <matthieu.paindavoine@softbankrobotics.com>
 - Vincent Palancher <vincent.palancher@external.softbankrobotics.com>
