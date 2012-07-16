qi::Future
==========


.. code-block:: c++

  #include <qimessaging/future.hpp>

.. cpp:namespace:: qi


Brief
-----

.. cpp:brief::

Details
-------

Future and Promise are used to get a value asynchronously. A Future is always linked to a Promise. The Promise allow setting the value, while a Future allow to wait for the value to be available and fetch it.

Example
-------

.. literalinclude:: /examples/example_qi_future.cpp
   :language: c++


Classes
-------

.. cpp:class:: qi::FutureInterface<T>

  Pure abstract Callback interface to be notified when something occur to a Future<T>.

  .. cpp:function:: void onFutureFinished(const Future<T> &future, void *data)

    pure virtual

    Called when a value has been set to the Future<T> being watched.

    :param future: the future
    :param data: pointer to user data


  .. cpp:function:: void onFutureFailed(const Future<T> &future, void *data)

    pure virtual

    Called when an error occured to the Future<T> being watched.

    :param future: the future
    :param data: pointer to user data



.. cpp:class:: qi::Future<T>

  Represent a future value of type T

  .. cpp:function:: Future()

    Constructor


  .. cpp:function:: const T &value() const

    Wait (an infinite time) and return the value of the Future.

    :return: value of the future


  .. cpp:function:: T &value()

    Wait (an infinite time) and return the value of the Future.

    :return: value of the future


  .. cpp:function:: operator const T&() const

    Wait (an infinite time) and return the value of the Future.

    :return: value of the future


  .. cpp:function:: operator T&()

    Wait (an infinite time) and return the value of the Future.

    :return: value of the future


  .. cpp:function:: bool wait(int msecs = 30000) const

    :param msecs: timeout in milliseconds
    :return: true on success

  .. cpp:function:: bool isReady() const

    :return: true if the value is available or an error have been reported

  .. cpp:function:: bool hasError() const

    :return: true if error occured

  .. cpp:function:: const std::string &error() const

    :return: the error

  .. cpp:function:: void addCallbacks(FutureInterface<T> *interface, void *data = 0)

    :param interface: the callback interface
    :param data: pointer to user data





.. cpp:class:: qi::Promise<T>

  Represent a future value of type T


  .. cpp:function:: Promise()

    Constructor

  .. cpp:function:: void setValue(const T &value)

    Set the value of the associated Future<T>

    :param value: the value

  .. cpp:function:: void setError(const std::string &msg)

    :param msg: the error message

  .. cpp:function:: Future<T> future()

    :return: the associated Future<T>
