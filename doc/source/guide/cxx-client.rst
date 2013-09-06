.. _guide-cxx-client:

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

loadservice is cool :cpp:func:`qi::Session::loadService`
The first step to interact with qimessaging services is to connect a
:cpp:class:`qi::Session`


to the _`Service Directory` of the robot. The Service Directory address is
represented by an _`URL`.

One simple way to achive this is to use the helper class _`ApplicationSession`,
which will fetch the URL from the command line (using the *--qi-url* option),
or use a default value of *localhost*::

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
from the session, and call its say function. The complete code becomes::

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

The _`qi::Object` template that wraps the ALTextToSpeech instance is used here to
provide reference-counting: the tts object will get disposed of as soon as
all references to it are removed.

Making asynchronous calls using qi::async
-----------------------------------------

Most of the API are designed to block until the operation started by the call
completes. For example with this code inside our main::

  qi::Object<ALTextToSpeech> tts = session.service("ALTextToSpeech");
  qi::Object<ALMotion> motion = session.service("ALMotion");
  tts->say("This is a very very very very long sentence.");
  motion->moveTo(1, 0, 0); // go forward one meter

The robot will only start moving when he finishes speaking.

To perform both actions simultaneously, the API provides _`qi::async` that
performs a call in an asynchronous manner, and notifies you when the call
finishes using a _`qi::Future`::

  qi::Object<ALTextToSpeech> tts = session.service("ALTextToSpeech");
  qi::Object<ALMotion> motion = session.service("ALMotion");
  qi::Future<void> sayOp = qi::async(tts, "say", "This is a very very very very long sentence.");
  qi::Future<void> moveOp = motion->async("moveTo", 1, 0, 0); // alternate version in _`qi::AnyObject`
  // Wait for both operations to terminate.
  sayOp.wait();
  moveOp.wait();

Look at the :cpp:class:`qi::Future` for more complete documentation, but here is what you
most definitely need to know:

- If the method throws an exception, it is stored in the *Future*, and can be
  accessed using *error()*.
- Use *wait()* to wait for the future to complete. It can accept a timeout duration
  as argument, and will return the state of the future.
- Use *value()* and *error()* to get the stored value or error.
- You can regiter a callback to be notified when the future finishes with
  *qi::Future::connect()*.


Using signal
------------

Using properties
---

passing an object as argument
-----------------------------


Generic api
-----------

If you wish to use a service for which no interface is available, but for
which you know the API, you can use the _`qi::AnyObject` generic API made
available throug *qi::Object<Empty>*::

  qi::Object<Empty> obj = session.service("ALTextToSpeech");
  obj.call("say", "Hello once more.");

Methods are also provided to emit and connect signals, read/write properties,
and access the service API.
  
  

