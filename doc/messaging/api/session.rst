qi::Session
===========

.. code-block:: c++

  #include <qimessaging/session.hpp>

.. cpp:namespace:: qi

Brief
-----

A session is an object used to connect to other services.

Details
-------

Sessions are communication channels to a service directory, used both by services and clients.

A service will need a session to register itself to a service directory.

.. code-block:: c++

  qi::Object obj;
  obj.advertiseMethod("reply", &reply);

  qi::Session session;
  session.connect(serviceDirectoryURL);
  session.waitForConnected();
  ...
  qi::Server srv;
  srv.listen(&session, endpoints);
  ...
  session.join();
  ...
  session.disconnect();


A client will use a session to retrieve the list of surrounding services, and get a qi::Object bound to their methods.

.. code-block:: c++

  qi::Session session;
  session.connect(serviceDirectoryURL);
  session.waitForConnected();

  qi::Object *obj = session.service("serviceTest");

  std::string result = obj->call<std::string>("reply", "plaf");
  std::cout << "answer:" << result << std::endl;

  session.disconnect();


Classes
-------

.. cpp:class:: Session

  .. cpp:function:: Session()

    Initialize a new session.

  .. cpp:function:: virtual ~Session()

    Destroy a session.

  .. cpp:function:: bool connect(const qi::Url &serviceDirectoryURL)

    Connect the session to the service directory using the *serviceDirectoryURL*.

    :param serviceDirectoryURL: URL of the service directory
    :return: true if successful, false otherwise

  .. cpp:function:: bool disconnect()

    Disconnect the session.

    :return: true if successful, false otherwise

  .. cpp:function:: bool join()

    Block until session terminates.

  .. cpp:function:: bool waitForConnected(int msecs = 30000)

    Block until session is connected.

    :param msecs: length of the timeout

  .. cpp:function:: bool waitForDisconnected(int msecs = 30000)

    Block until session is disconnected.

    :param msecs: length of the timeout

  .. cpp:function:: qi::Future< std::vector<ServiceInfo> > services()

    Get the list of services registered to the service directory.

    :return: a vector of ServiceInfo

  .. cpp:function:: qi::Future< qi::TransportSocket* > serviceSocket(const std::string &name, unsigned int *idx, qi::Url::Protocol type = qi::Url::Protocol_Any)

    Create a new TransportSocket, connect it to the service *name* and return it. The service id is also stiored in *idx*.
    If *type* is provided, the socket will use the given protocol if available, or fail otherwise.

    :param name: the service to connect to
    :param idx: a pointer to the integer which will hold the service id
    :param type: type of connection which must be used
    :return: a TransportSocket connected to the service, 0 if it failed

    .. note::

      This function is provided to those who directly want to work with a TransportSocket. Its use is not recommended, unless you really know what you are doing.

  .. cpp:function:: qi::Future< qi::Object* > service(const std::string &service, qi::Url::Protocol  type = qi::Url::Protocol_Any)

    Connect to the service *name* and return its corresponding qi::Object.
    If *type* is provided, the socket will use the given protocol if available, or fail otherwise.

    :param service: the service to connect to
    :param type: type of connection which must be used
    :return: a qi::Object corresponding to the service

