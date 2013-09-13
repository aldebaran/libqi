.. _guide-cxx-service:

.. cpp:namespace:: qi

.. cpp:auto_template:: True

.. default-role:: cpp:guess


How to write a qimessaging service in C++
=========================================

This document will teach you how to write your very own qimessaging service,
so that your new shiny object can be used by other qimessaging services and
client, locally or remotely, and from many different programming languages.

The first step is to check if a standard interface that matches what your
object does already exists. For instance if you are implementing or wrapping
a new Text-To-Speech engine, you should reuse the existing *ALTextToSpeech*
interface. If no corresponding interface exists, you'll have to write your own.

Write your interface
--------------------

The interface is a very important part of your service. It defines the list of
methods, signals and properties that are made available through qimessaging.

Create a new header file, and write a C++ pure interface (meaning all methods
should be pure virtuals), with the addition that you can use public `qi::Signal`
and `qi::Property`. Here is an example exhibiting most of the features:

.. code-block:: cpp

  // Helper struct
  struct Mosquito
  {
    double yaw,theta,distance;
  };
  class Cat
  {
    public:
      virtual ~Cat() {}
      virtual void meow(int volume) = 0;
      virtual bool setTarget(const Mosquito& m) = 0;

      qi::Property<float> hunger;
      qi::Property<float> boredom;
      qi::Property<float> cuteness;
      qi::Signal<Mosquito> onTargetDetected;
  };


There are some restrictions on what you can do:

- Do not use *class mebmer data*.
- Output parameters are not supported: you should not take pointers as arguments,
  or non-const references.
- Do not use methods in transmitted *struct*, as they will not be available in
  other languages: put the methodes in an associated *class*.

To understand those restrictions, keep in mind that qimessaging is a middleware
that knows how to transport *data* (like primitive types, structs, and containers
of the above), and how to make calls to remote methods, signals and properties,
but cannot mix the two.

Implementing an existing interface
----------------------------------

Alternatively if an existing interface suits your need, you can simply reuse it.


Generate code
-------------

IDL
~~~

Once your interface is ready, the next step is to generate an *IDL* file, that is
a standardised *XML* representation of your interface::

  idl --output-mode=idl path/to/myinterface.hpp -o myinterface.idl

If you install your interface header in some distributed package, you should
provide the idl file as well.

Support library
~~~~~~~~~~~~~~~

In order for :ref:`client code<guide_cxx-client>` to use an `Object<T>` on
your interface, some support code needs to be generated::

  idl --output-mode=proxy myinterface.idl -o myinterfaceproxy.cpp

You should then include *myinterfaceproxy* into source distributions, and
a shared-library build of this source *myinterfaceproxy.so* (or *myinterfaceproxy.dll*
depending on your OS) into binary distributions.
Note that this file includes the interface header *myinterface.hpp*.

Client code will then need to link with that library, otherwise creation of an
*qi::Object<MyInterface>* will fail at runtime.


Write your implementation
-------------------------

Service skeleton
~~~~~~~~~~~~~~~~

If you start implementing an interface from scratch, *idl* can optionally help you by
generating an implementation skeleton::

  idl --output-mode=cxxserviceregister myinterface.idl -o myinterfaceimpl.cpp

Manual registration
~~~~~~~~~~~~~~~~~~~

If you do not use the skeleton, two extra steps must be taken when implementing
a service, in the form of two macro calls in your source file:

.. code-block:: cpp

  // Tell the typesystem that class MyInterfaceImpl implements MyInterface
  QI_IMPLEMENT_MYINTERFACE(MyInterfaceImpl)
  // Registers MyIntefaceImpl to the factory, so that your service can be
  // found when the containing shared library is loaded.
  QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(MyInterfaceImpl)

Build your service implementation
---------------------------------

Your code should be built as a shared library.

Starting your service manually
------------------------------

The *qi-launch* utility can be used to instanciate your implementation, and
register it to an existing *ServiceDirectory*::

  qi-launch -s robotIp myinterfaceimpl.so

Deploying your service
----------------------



Graphical view of the standard workflow
---------------------------------------

.. graphviz::

  digraph workflow {
    Interface      [label="Interface\nfoo.hpp"];
    IDL            [label="IDL\nfoo.idl"];
    Support        [label="Client support\nfoo.cpp"];
    ImplSkel       [label="Implementantion Skeleton\nfooimpl.cpp"];
    ImplFull       [label="Implementation\nfooimpl.cpp"];
    ImplSo         [label="Service module\nfooimpl.so" color="blue"];
    SupportSo      [label="Support library\nfoo.so" color="blue"];

    Interface -> IDL      [label = "-m IDL" ];
    IDL -> Support        [label = "-m proxy"];
    IDL -> ImplSkel       [label = "-m cxxserviceregister"];
    ImplSkel -> ImplFull  [color=red];
    ImplFull -> ImplSo    [color=blue];
    Support -> SupportSo  [color=blue];

    node [color="white" fontcolor="white" label=""]; la lb lc ld le lf;
    la -> lb [label = "generation using idl"];
    lc -> ld [color=red label="User-written"];
    le->lf [color=blue label="Compilation"];
    }


Binding an existing class without an interface
----------------------------------------------


