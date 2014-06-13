.. _guide-cxx-service:

.. cpp:namespace:: qi

.. cpp:auto_template:: True

.. default-role:: cpp:guess

How to write a qimessaging service in C++
=========================================

This document will teach you how to write your very own qimessaging service,
so that your new shiny object can be used by other qimessaging services and
client, locally or remotely, and from many different programming languages.

Limitations
-----------

There are some restrictions on what you can do:

- Do not use *class member data*.
- Output parameters are not supported: you should not take pointers as arguments,
  or non-const references.
- Do not use methods in transmitted *struct*, as they will not be available in
  other languages: put the methods in an associated *class*.

To understand those restrictions, keep in mind that qimessaging is a middleware
that knows how to transport *data* (like primitive types, structs, and containers
of the above), and how to make calls to remote methods, signals and properties,
but cannot mix the two.

Write your service
------------------

Your service can be a simple class which is registered in the type system (see
:ref:`type registration<guide-cxx-register-types>`

.. code-block:: cpp

  struct Mosquito
  {
    double yaw,theta,distance;
  };
  QI_TYPE_STRUCT(Mosquito, yaw, theta, distance)

  class Cat
  {
    public:
      void meow(int volume);
      bool setTarget(const Mosquito& m);

      qi::Property<float> hunger;
      qi::Property<float> boredom;
      qi::Property<float> cuteness;
      qi::Signal<Mosquito> onTargetDetected;
  };
  QI_REGISTER_OBJECT(Cat, meow, setTarget, hunger, boredom, cuteness,
      onTargetDetected);

Sessions and ServiceDirectory
-----------------------------

Sessions may be listening and/or connected to other sessions. In a group of
sessions, one and only one ServiceDirectory must be registered which will keep
track of all registered services. Usually, one session is used as a server with
`listenStandalone()` and other sessions are used as client with `connect()`.

`listenStandalone()` is the same function as `listen()` but it will also
register a ServiceDirectory on the session.

Registering your service on a ServiceDirectory
----------------------------------------------

You can then instantiate your object and expose it on a session. It does not
matter whether your session is a listener or a client. As the client session is
already shown in the :ref:`client session guide<guide-cxx-client>`, we will do
the listener example here.

.. code-block:: cpp

  int main(int argc, char** argv)
  {
    qi::Application app(argc, argv);

    qi::SessionPtr session = qi::makeSession();
    session->listenStandalone("tcp://localhost:9559");

    boost::shared_ptr<Cat> cat = boost::make_shared<Cat>();
    session->registerService("CatService", cat);

    app.run();
  }

The session will keep a reference on cat until it's closed.
