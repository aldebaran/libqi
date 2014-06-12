.. _api-object:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::Object API
**************

qi::AnyObject
=============

`qi::AnyObject` is a specialization of qi::Object that provides type erasure on
objects, similar to what `qi::AnyValue` does on values. It can only work on
registered objects. We'll use `Graph::Drawer` from
:ref:`the registering guide<guide-cxx-register-types>`. You can make an `qi::Object` from a
`boost::shared_ptr`.

.. code-block:: c++

  qi::AnyObject obj = boost::make_shared<Graph::Drawer>();

`qi::Object` uses a `shared_ptr` semantics, so the object will be destroyed when
there are no more references to it. You can also get a `qi::AnyObject` from a
session, another service, etc.

You can call a function with call or async if you want the call to be
asynchronous. You can also connect signals and change properties.

.. code-block:: c++

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

.. code-block:: c++

  qi::Object<Graph::Drawer> obj = boost::make_shared<Graph::Drawer>();
  obj->draw(Graph::Point(11, 12), Graph::Green);
