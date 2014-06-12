.. _api-trackable:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::Trackable
*************

Rationale
=========

.. code-block:: c++

  class MyClass {
    void callback();
  };

  qi::Signal<void> signal;

  MyClass* my = new MyClass();
  signal.connect(boost::bind(&MyClass::callback, my));
  delete my;

  signal(); // CRASH

In this code, the program will probably crash because the callback will be
called even though `my` has been destroyed.

Usage
=====

In this case, you need to use `qi::Trackable` with `qi::bind` so that the
callback won't be called when your object dies.

MyClass should inherit from `qi::Trackable<MyClass>` and the constructor must
call `qi::Trackable`'s constructor with `this`. Then, you *must* call
`destroy()` at the *beginning* of your destructor. `destroy()` ensures that all
callbacks are finished before you start destructing your object. Then you must
use qi::bind instead of boost::bind which will do nothing if its first argument
is not valid anymore.

.. code-block:: c++

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
  signal.connect(qi::bind(&MyClass::callback, my));
  delete my;

  signal(); // callback won't be called, no crash

.. cpp:autoclass:: qi::Trackable
