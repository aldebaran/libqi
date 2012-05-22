qi::Functor
===========

.. code-block:: c++

  #include <qimessaging/functor.hpp>

.. cpp:namespace:: qi


Brief
-----

.. cpp:brief::

Details
-------

Functor are crappy

Example
-------

.. todo:: literalinclude:: /examples/example_qi_datastream.cpp
   :language: c++

Classes
-------


.. cpp:class:: qi::FunctorParameters

  Functor parameters

  .. cpp:function:: explicit FunctorParameters(const qi::Buffer &buffer)

    :param buffer: buffer

  .. cpp:function:: const qi::Buffer &buffer()

    :return: buffer

.. cpp:class:: qi::FunctorResult

  Functor result

    .. cpp:function:: FunctorResult()

      Constructor

    .. cpp:function:: virtual ~FunctorResult()

      Destructor

    .. cpp:function:: void setValue(const qi::Buffer &buffer)

      virtual

      Set the result value

    .. cpp:function:: void setError(const qi::Buffer &msg)

      virtual

      Set an error message

    .. cpp:function:: bool isValid() const

      :return: true if a value or an error has been set


.. cpp:class:: qi::Functor

  Functor that support serialization/deserialization using qi::DataStream.

  .. cpp:function:: void call(const qi::FunctorParameters &params, qi::FunctorResult result) const

    virtual

    :param params: function parameters
    :param result: function result

  .. cpp:function:: virtual ~Functor()

    Destructor


.. cpp::function: void makeFunctorResult(qi::FunctorResult *promise, qi::Future<T> *future)

  Create a pair of FunctorResult and Future that are linked together.

  .. todo:: internal?
