.. _api-trackable:
.. cpp:namespace:: qi::Trackable
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::Trackable
*************

Rationale
=========

.. code-block:: cpp

  class MyClass {
    void callback();
  };

  qi::Signal<void> signal;

  MyClass* my = new MyClass();
  signal.connect(&MyClass::callback, my);
  delete my;

  signal(); // CRASH

In this code, the program will probably crash because the callback will be
called even though `my` has been destroyed.

Usage
=====

In this case, you need to use `qi::Trackable` with `qi::bind` so that the
callback won't be called when your object dies. Note that you usually don't
need to call `qi::bind` explicitely, `qi::Future::connect` and
`qi::Signal::connect` do it for you.

MyClass should inherit from `qi::Trackable<MyClass>` and the constructor must
call `qi::Trackable`'s constructor with ``this``. Then, you *must* call
`destroy()` at the *beginning* of your destructor. `destroy()` ensures that all
callbacks are finished before you start destructing your object. You must *not*
use ``boost::bind`` if you want your object to be tracked!

.. code-block:: cpp

  #include <qi/trackable.hpp>

  class MyClass : qi::Trackable<MyClass> {
    MyClass() : qi::Trackable<MyClass>(this)
    {}
    ~MyClass() {
      destroy();
    }

    void callback();
  };

  qi::Signal<void> signal;

  MyClass* my = new MyClass();
  signal.connect(&MyClass::callback, my);
  delete my;

  signal(); // callback won't be called, no crash

.. cpp:autoclass:: qi::Trackable

.. cpp:autofunction:: qi::bind
