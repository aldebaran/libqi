.. _api-property:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::Property API
****************

Simple usage
------------

.. code-block:: c++

  #include <qi/property.hpp>

A property is basically a simple variable that you can set and get.

.. code-block:: c++

  qi::Property<int> p;
  p.set(18);
  std::cout << p.get() << std::endl;

Signals
-------

It also allows you to have callbacks called when the property is changed.

.. code-block:: c++

  void callback(const int& value) {
    std::cout << "New value: " << value << std::endl;
  }

  // ...

  qi::Property<int> p;
  p.connect(callback);
  p.set(42);
  p.set(24);
  p.set(12);

This will print::

  New value: 42
  New value: 24
  New value: 12

.. warning::

  This output is not guaranteed. `callback` is called asynchronously, thus the
  multiple calls may be running in parallel and print a different
  implementation-dependent output.

Custom setters and getters
--------------------------

You can set a custom getter and setter on a property.

.. code-block:: c++

  int getter(const int& value) {
    std::cout << "User is requesting value" << std::endl;
    return value;
  }

  bool setter(int& storage, const int& value) {
    std::cout << "User is changing value from " << storage << " to " << value
      << std::endl;
    if (value < 0)
    {
      std::cout << "Cannot set to negative values" << std::endl;
      return false;
    }
    else
    {
      storage = value;
      return true;
    }
  }

  // ...

  qi::Property<int> p(getter, setter);
  p.set(42);
  std::cout << "Property is " << p.get() << std::endl;
  p.set(-12);
  std::cout << "Property is " << p.get() << std::endl;

This will print::

  User is changing value from 0 to 42
  Property is 42
  User is changing value from 42 to -12
  Cannot set to negative values
  Property is 42

.. note::

  If a callback is connected on the property, it is *not* triggered when the
  setter failed to set the new value.

.. cpp:autoclass:: qi::Property
