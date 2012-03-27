qi::Buffer
==========

.. code-block:: c++

  #include <qimessaging/buffer.hpp>

.. cpp:namespace:: qi

Brief
-----

.. cpp:brief::

Details
-------

Buffer suck because I do not write them :P

Example
-------

.. todo:: literalinclude:: /examples/example_qi_buffer.cpp
   :language: c++


Classes
-------


.. cpp:class:: qi::Buffer

  Store a buffer

  .. cpp:function:: Buffer()

  .. cpp:function:: int    write(const void *data, size_t size)

    :param data: the data
    :param size: the size
    :return: the write size

  .. cpp:function:: int    read(void *data, size_t size)

    :param data: the data
    :param size: the size
    :return: the read size

  .. cpp:function:: void  *read(size_t size)

    :param size: the size
    :return: the data

  .. cpp:function:: size_t size() const

  .. cpp:function:: void  *reserve(size_t size)

    :param size: the size
    :return: the data


  .. cpp:function:: size_t seek(long offset)

    :param offset: offset

  .. cpp:function:: void  *peek(size_t size) const

    :param size: the size
    :return: the data

  .. cpp:function:: void  *data() const

    :return: the data

  .. cpp:function:: void   dump() const

