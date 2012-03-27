qi::Session
===========

.. code-block:: c++

  #include <qimessaging/session.hpp>

.. cpp:namespace:: qi

Brief
-----

.. cpp:brief::

Details
-------

Session are good for paid cabin.


Example
-------

.. todo:: literalinclude:: /examples/example_qi_session.cpp
   :language: c++

Classes
-------

.. cpp:class:: Session

  .. cpp:function:: Session()

  .. cpp:function:: virtual ~Session()

  .. cpp:function:: bool connect(const qi::Url &serviceDirectoryURL)

  .. cpp:function:: bool disconnect()

  .. cpp:function:: bool join()

  .. cpp:function:: bool waitForConnected(int msecs = 30000)

  .. cpp:function:: bool waitForDisconnected(int msecs = 30000)

  .. cpp:function:: qi::Future< std::vector<ServiceInfo> > services()

  .. cpp:function:: qi::Future< qi::TransportSocket* > serviceSocket(const std::string &name, unsigned int *idx, qi::Url::Protocol type = qi::Url::Protocol_Any)

  .. cpp:function:: qi::Future< qi::Object* > service(const std::string &service, qi::Url::Protocol  type = qi::Url::Protocol_Any)
