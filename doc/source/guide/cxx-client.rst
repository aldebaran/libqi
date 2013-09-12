.. _guide-cxx-client:

.. cpp:namespace:: qi

.. cpp:auto_template:: True

.. default-role:: cpp:guess

How to write a qimessaging client in C++
========================================

Introduction
-------------

This guide will teach you how to write a qimessaging client in C++, to
interact with the various services offered by the robot.

Prerequisites
-------------

- An installed NAOqi SDK for your operating system.
- A C++ project setup in your favorite C++ development environment, ready
  to use the headers and libraries provided by the NAOqi SDK.


Creating a session
------------------

The first step to interact with qimessaging services is to connect a
:cpp:class:`qi::Session` to the ::ref:`Service Directory<api-ServiceDirectory>` of the robot.
The Service Directory address is represented by a standard URL.

One simple way to achive this is to use the helper class _`ApplicationSession`,
which will fetch the URL from the command line (using the *--qi-url* option),
or use a default value of *localhost*:

.. code-block:: cpp

  #include <iostream>
  #include <qimessaging/applicationsession.hpp>

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    qi::Session& session = app.session();
    std::cerr << "Ready to roll!" << std::endl;
  }


Calling a method on a service
-----------------------------

Each service comes with it's own header in the SDK, that defines the set of
methods, signals and properties available (more on this later). Let us make the
robot speak by using the *say* method of the *ALTextToSpeech* service. For this
we need to include the altexttospeech header, request an *Object<ALTextToSpeech>*
from the session, and call its say function. The complete code becomes:

.. code-block:: cpp

  #include <qimessaging/applicationsession.hpp>
  #include <qicore/altexttospeech.hpp>

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    qi::Session& session = app.session();
    qi::Object<ALTextToSpeech> tts = session.service("ALTextToSpeech");
    tts->say("Hello world!");
  }

Note here that the name of the interface *ALTextToSpeech* is the same as the
name of the service that implements it, but it might not always be the case.
Multiple services can provide the same interface, allowing you to chose the
one that better suits your needs.

The `qi::Object` template that wraps the ALTextToSpeech instance is used here to
provide reference-counting: the tts object will get disposed of as soon as
all references to it are removed. The *->* operator gives you access to
the underlying interface, whereas `Object` itself exposes a :ref:` generic interface<guide-generic-interface>`

Making asynchronous calls using qi::async
-----------------------------------------

Most of the API are designed to block until the operation started by the call
completes. For example with this code inside our main:

.. code-block:: cpp

  qi::Object<ALTextToSpeech> tts = session.service("ALTextToSpeech");
  qi::Object<ALMotion> motion = session.service("ALMotion");
  tts->say("This is a very very very very long sentence.");
  motion->moveTo(1, 0, 0); // go forward one meter

The robot will only start moving when he finishes speaking.

To perform both actions simultaneously, the API provides `qi::async` that
performs a call in an asynchronous manner, and notifies you when the call
finishes using a :cpp:class:`qi::Future`:

.. code-block:: cpp

  qi::Object<ALTextToSpeech> tts = session.service("ALTextToSpeech");
  qi::Object<ALMotion> motion = session.service("ALMotion");
  qi::Future<void> sayOp = qi::async(tts, "say", "This is a very very very very long sentence.");
  qi::Future<void> moveOp = motion->async("moveTo", 1, 0, 0); // alternate version in _`qi::AnyObject`
  // Wait for both operations to terminate.
  sayOp.wait();
  moveOp.wait();

Look at the `qi::Future` for more complete documentation, but here is what you
most definitely need to know:

- If the method throws an exception, it is stored in the *Future*, and can be
  accessed using *error()*.
- Use *wait()* to wait for the future to complete. It can accept a timeout duration
  as argument, and will return the state of the future.
- Use *value()* and *error()* to get the stored value or error.
- You can regiter a callback to be notified when the future finishes with
  *qi::Future::connect()*.


Using signal and properties
---------------------------

Instances of `Signal` and `Property` defined by service interfaces can be used
throug an `Object<T>` as if they were local signal and properties.

Passing an object as argument
-----------------------------

Some methods in the services you will use expect an object as argument, for
instance *Logger::addListener(Object<LogListener> listener);*. To call this
method, you must first implement the *LogListener* interface into your own
class, and then wrap a pointer to an instance of this class into an
*Object<LogListener>* that will take ownership of the pointer:

.. code-block:: cpp

  class MyLogListener: public LogListener
  {
    // Implement LogListener interface
  };

  void someFunction()
  {
    Object<LogListener> o(new MyLogListener());
    Object<Logger> logger = session.service("Logger");
    logger->addListener(o);
  }

In the example above, your instance of *MyLogListener* will be kept alive as
long as the logger service holds an *Object<LogListener>* on it.


Generic api
-----------

.. _guide-generic-interface

If you wish to use a service for which no interface is available, but for
which you know the API, you can use the `qi::AnyObject` generic API made
available through *qi::Object<Empty>*:

.. code-block:: cpp

  qi::Anyobject obj = session.service("ALTextToSpeech");
  obj.call("say", "Hello once more.");

Nota that this generic API is available on all `Object<T>`.

Methods are also provided to emit and connect signals, read/write properties,
and access the service API.
  
  

