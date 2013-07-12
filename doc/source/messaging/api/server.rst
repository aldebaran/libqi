qi::Server
==========

.. code-block:: c++

  #include <qimessaging/server.hpp>

.. cpp:namespace:: qi

Brief
-----

.. cpp:brief::

Details
-------

Server are so Web 1.0...

Used to


Example
-------

.. literalinclude:: /examples/example_qi_server.cpp
   :language: c++

.. todo::
  fix the content of the example

Classes
-------

.. cpp:class:: Server

  Advertise named services. Advertised Services are registered with the ServiceDirectory so that clients can find them.

  .. cpp:function:: Server()

    Constructor

  .. cpp:function:: virtual ~Server()

    Destructor

  .. cpp:function:: bool listen(qi::Session *session, const std::vector<std::string> &url)

    :param session: the session used to register services on the master
    :param url: url to bind the server on
    :return: true on success, false on error

  .. cpp:function:: void stop()


  .. cpp:function:: int registerService(const std::string &name, qi::GenericObject *obj)

    :param name: name of the service
    :param obj: the service to bind
    :return: the service Id, 0 on error

  .. cpp:function:: void unregisterService(unsigned int idx)

    :param idx: the service id to unregister
