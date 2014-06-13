.. _guide-cxx-async:

.. cpp:namespace:: qi

.. cpp:auto_template:: True

.. default-role:: cpp:guess

Asynchronous operations
=======================

Introduction
------------

libqi provides a threadpool for doing small asynchronous operations. Note that
blocking in these threads impact the whole system as a non-working thread will
prevent other work from being scheduled.

With C++ functions
------------------

You can push a task to be executed asynchronously from a simple C++ function.
It is also possible to specify a delay for the task.

.. code-block:: cpp

  #include <libqi/eventloop.hpp>

  void doSomething(int value) {
    std::cout << "value is " << value << std::endl;
  }

  int main(int argc, char* argv[]) {
    qi::Application app(argc, argv);

    // run as soon as possible
    qi::Future<void> future =
      qi::getEventLoop()->async(boost::bind(doSomething, 42));
    // run in 2 seconds
    qi::Future<void> future2 =
      qi::getEventLoop()->async(boost::bind(doSomething, 42), 2000000);

    // do stuff...

    future.wait();
    future2.wait();

    return 0;
  }

If your function should be called periodically, use `qi::PeriodicTask`.

It is also possible to cancel the execution of a task if it hasn't started yet:

.. code-block:: cpp

  qi::Future<void> future =
    qi::getEventLoop()->async(boost::bind(doSomething, 42), 2000000);

  future.cancel();

  // doSomething will never be called

With a qi::AnyObject
--------------------

There are two ways of using an AnyObject asynchronously, with the `async()`
method and the `qi::async()` function.

.. code-block:: cpp

  qi::AnyObject tts = session.service("ALTextToSpeech");
  qi::AnyObject motion = session.service("ALMotion");
  qi::Future<void> sayOp = qi::async(tts, "say", "This is a very very very very long sentence.");
  qi::Future<void> moveOp = motion->async("moveTo", 1, 0, 0);
  // Wait for both operations to terminate.
  sayOp.wait();
  moveOp.wait();

Using the returned future
-------------------------

Look at the `qi::Future` for more complete documentation, but here is what you
most definitely need to know:

- If the method throws an exception, it is stored in the *Future*, and can be
  accessed using *error()*.
- Use *wait()* to wait for the future to complete. It can accept a timeout
  duration as argument, and will return the state of the future.
- Use *value()* and *error()* to get the stored value or error.
- You can register a callback to be notified when the future finishes with
  *qi::Future::connect()*.

With a qi::Future
-----------------

The callbacks connected to the `qi::Future` will also be called from the
threadpool.

.. code-block:: cpp

  void callback() {
    std::cout << "I'm called from a thread" << std::endl;
  }

  qi::Promise<void> promise;
  qi::Future<void> future = promise.future();
  future.connect(callback);
  promise.setValue(0);
