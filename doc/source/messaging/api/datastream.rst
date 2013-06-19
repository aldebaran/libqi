qi::DataStream
==============

.. code-block:: c++

  #include <qimessaging/datastream.hpp>

.. cpp:namespace:: qi


Brief
-----

.. cpp:brief::

Details
-------

DataStream sux

Example
-------

.. todo:: literalinclude:: /examples/example_qi_datastream.cpp
   :language: c++

Classes
-------


.. cpp:class:: qi::DataStream

  binary serialization


  .. cpp:function:: explicit DataStream(const qi::Buffer &buffer)

    Constructor

  .. cpp:function:: explicit DataStream(qi::Buffer &buffer)

    Constructor

  .. cpp:function:: size_t read(void *data, size_t len)

    :param data:
    :param len:
    :return:

  .. cpp:function:: void writeString(const char *str, size_t len)

    :param str:
    :param len:


  .. cpp:function:: DataStream& operator<<(bool value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(char value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(int value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(unsigned char value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(unsigned int value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(float value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(double value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(const char *value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator<<(const std::string& value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(bool &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(char &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(int &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(unsigned char &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(unsigned int &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(float &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(double &value)

    :param value:
    :return: the DataStream

  .. cpp:function:: DataStream& operator>>(std::string& value)

    :param value:
    :return: the DataStream


.. cpp:function:: qi::DataStream &operator<<(qi::DataStream &sd, const std::list<T> &v)

  Serialize list into datastream.

  :param sd: the datastream
  :param v: the list to write
  :return: the datastream

.. cpp:function:: qi::DataStream &operator>>(qi::DataStream &sd, std::list<T> &v)

  Deserialize list from datastream.

  :param sd: the datastream
  :param v: the list to read
  :return: the datastream

.. cpp:function:: qi::DataStream &operator<<(qi::DataStream &sd, const std::vector<T> &v)

  Serialize vector into datastream.

  :param sd: the datastream
  :param v: the vector to write
  :return: the datastream

.. cpp:function:: qi::DataStream &operator>>(qi::DataStream &sd, std::vector<T> &v)

  Deserialize vector from datastream.

  :param sd: the datastream
  :param v: the vector to read
  :return: the datastream

.. cpp:function:: qi::DataStream &operator<<(qi::DataStream &sd, const std::map<K, V> &m)

  Serialize map into datastream.

  :param sd: the datastream
  :param v: the map to write
  :return: the datastream

.. cpp:function:: qi::DataStream &operator>>(qi::DataStream &sd, std::map<K, V>  &m)

  Deserialize map from datastream.

  :param sd: the datastream
  :param v: the map to read
  :return: the datastream

.. cpp:function:: qi::DataStream &operator<<(qi::DataStream &sd, const qi::AnyValue &value)

  Serialize qi::AnyValue into datastream.

  :param sd: the datastream
  :param v: the qi::AnyValue to write
  :return: the datastream

.. cpp:function:: qi::DataStream &operator>>(qi::DataStream &sd, qi::AnyValue &value)

  Deserialize qi::AnyValue from datastream.

  :param sd: the datastream
  :param v: the qi::AnyValue to read
  :return: the datastream
