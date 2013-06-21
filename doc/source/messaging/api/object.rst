qi::GenericObject
==========

.. code-block:: c++

  #include <qitype/anyobject.hpp>

.. cpp:namespace:: qi

Brief
-----

.. cpp:brief::

Details
-------

GenericObject are bloated

Example
-------

.. todo:: literalinclude:: /examples/example_qi_object.cpp
   :language: c++

Classes
-------

.. cpp:class:: MetaMethod

  .. todo:: clean the class and document

.. cpp:class:: MetaObject

  .. todo:: clean the class and document


.. cpp:class:: GenericObject

  .. cpp:function:: GenericObject()

  .. cpp:function:: virtual ~GenericObject()

  .. cpp:function:: MetaObject &metaObject()

  .. cpp:function:: unsigned int advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)

  .. cpp:function:: unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function)

  .. cpp:function:: qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0 = None, const P1 &p1 = None, const P2 &p2 = None, const P3 &p3 = None, const P4 &p4 = None, const P5 &p5 = None, const P6 &p6 = None, const P7 &p7 = None, const P8 &p8 = None)

  .. cpp:function:: void metaCall(int method, const FunctorParameters &in, FunctorResult out)

    virtual

    Call a metamethod.

    :param method: the method id to call
    :param in: call parameters
    :param out: return value of the call
