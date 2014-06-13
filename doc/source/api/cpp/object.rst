.. _api-object:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::Object API
**************

qi::AnyObject
=============

`qi::AnyObject` is a specialization of `qi::Object<T>` that provides type
erasure on objects, similar to what `qi::AnyValue` does on values. It can only
work on registered objects. We'll use `Graph::Drawer` from :ref:`the
registering guide<guide-cxx-register-types>`. You can make an `qi::Object` from
a `boost::shared_ptr`.

.. code-block:: cpp

  qi::AnyObject obj = boost::make_shared<Graph::Drawer>();

`qi::Object` uses a `shared_ptr` semantics, so the object will be destroyed when
there are no more references to it. You can also get a `qi::AnyObject` from a
session, another service, etc.

You can call a function with call or async if you want the call to be
asynchronous. You can also connect signals and change properties.

.. code-block:: cpp

  obj.call<bool>("draw", Graph::Point(10, 20), Graph::Green);

  qi::Future<bool> future =
    obj.async<bool>("draw", Graph::Point(10, 20), Graph::Green);
  // do stuff...
  future.wait();

  obj.connect("drawDone", &mycallback);

  obj.setProperty("origin", Graph::Point(0, 12));
  Graph::Point p = obj.property<Graph::Point>("origin");
  std::cout << p.y << std::endl; // 12

qi::Object<T>
=============

`qi::Object` can be specialized with T if the object is T or inherits from T.

.. code-block:: cpp

  qi::Object<Graph::Drawer> obj = boost::make_shared<Graph::Drawer>();
  obj->draw(Graph::Point(11, 12), Graph::Green);

`qi::Object<T>`'s specializations do not work with remote objects yet.

Passing an object as argument
=============================

Some methods in the services you will use expect an object as argument, for
instance *Logger::addListener(Object<LogListener> listener);*. To call this
method, you must first implement the *LogListener* interface into your own
class, and then wrap a pointer to an instance of this class into an
*Object<LogListener>* or a *qi::AnyObject* that will take ownership of the
pointer:

.. code-block:: cpp

  class MyLogListener: public LogListener
  {
    // Implement LogListener interface
  };

  void someFunction()
  {
    qi::AnyObject logger = session.service("Logger");
    qi::AnyObject o(boost::make_shared<LogListener>());
    logger.call("addListener", o);
  }

In the example above, your instance of *MyLogListener* will be kept alive as
long as the logger service holds a *qi::AnyObject* on it. The same holds true
when returning objects.
