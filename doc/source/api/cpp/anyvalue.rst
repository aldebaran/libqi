.. _api-anyvalue:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::AnyValue API
****************

The class `qi::AnyValue` is a dynamic value which can contain anything. It
works like a normal variable and allows you to store any type and get it back,
but also use the variable without converting it back to its real type.

Storing and retrieving a value
==============================

Storing and retrieving a value is easy enough:

.. code-block:: c++

  struct MyStruct {
    int canard;
  };

  MyStruct s;
  s.canard = 12;
  // this creates a copy and stores it in value
  qi::AnyValue value = qi::AnyValue::from(s);

  MyStruct& s2 = value.as<MyStruct>();
  std::cout << s2.canard << std::endl; // 12

  // would throw
  //value.as<int>();

.. warning::

  When you use `as()`, you must specify the exact same type that was passed to
  the AnyValue, you can't store and `int` and retrieve an `unsigned int`.

.. note::

  As you can see, the type does not need to be registered in the type
  system for this to work.

If you don't know the exact type stored, you can still use `to()` which
supports standard conversions:

.. code-block:: c++

  // type can be changed
  value = qi::AnyValue::from(42.3);
  std::cout << value.to<int>() << std::endl; // 42

Using the value with standard types
===================================

You can use a value without extracting its contents when it's built from
standard types like built-in types, vectors, maps. It also supports common
operators.

.. code-block:: c++

  std::vector<int> v;
  v.push_back(12);
  v.push_back(24);
  v.push_back(42);

  // creates a copy
  qi::AnyValue value = qi::AnyValue::from(v);
  std::cout << value[2].as<int>() << std::endl; // 42

Using the value with registered types
=====================================

The same thing can be achieved with registered types.

.. code-block:: c++

  Graph::Point p(12, 42);
  qi::AnyValue value = qi::AnyValue::from(p);
  // you can't use names (x and y) here because structs are registered as
  // tuples with annotations in the type system
  std::cout << value[0].as<int>() << ", " << value[1].as<int>()
    << std::endl; // 12, 42

.. cpp:autoclass:: qi::AnyValue
