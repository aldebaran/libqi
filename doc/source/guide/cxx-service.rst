.. _guide-cxx-service:

.. cpp:namespace:: qi

.. cpp:auto_template:: True

.. default-role:: cpp:guess

.. warning::

   WORK IN PROGRESS.

   This is not usable right now.


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

  #ifndef CAT_HPP
  # define CAT_HPP

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
  #endif

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

Implementing an existing interface
----------------------------------

Alternatively if an existing interface suits your need, you can simply reuse it.


Generate code
-------------

IDL
~~~

Once your interface is ready, the next step is to generate an *IDL* file, that is
a standardised *XML* representation of your interface::

  idl --output-mode=idl path/to/cat.hpp -o cat.xml

If you install your interface header in some distributed package, you should
install the idl file as well.

Support library
~~~~~~~~~~~~~~~

In order for :ref:`client code<guide_cxx-client>` to use an `Object<T>` on
your interface, some support code needs to be generated::

  idl --output-mode=client cat.idl -o cat_client_support.cpp

You should then include *cat_client_support.cpp* into source distributions, and
a shared-library build of this source *cat_client_support.so* (or *cat_client_support.dll*
depending on your OS) into binary distributions.
Note that this file includes the interface header *cat.hpp*.

Client code will then need to link with that library, otherwise creation of a
*qi::Object<Cat>* will fail at runtime.

  .. warning::

    Under some linux distributions (including Ubuntu)
    the compiler is patched to pass the *--as-needed* option by default to the linker.
    You need to pass *-Wl,--no-as-needed* when linking with the client support library.
    If you are using CMake here is the way to do so::

      if(NOT WIN32)
        set_target_properties(my_target_name PROPERTIES
          LINK_FLAGS "-Wl,--no-as-needed")
      endif()

Write your implementation
-------------------------

A service implementation is simply an implementation of the C++ interface
you defined, plus a few macro calls to register it.


Service skeleton
~~~~~~~~~~~~~~~~

If you start implementing an interface from scratch, *idl* can optionally help you by
generating an implementation skeleton::

  idl --output-mode=cxxskel cat.idl -o catimpl.cpp

Manual registration
~~~~~~~~~~~~~~~~~~~

If you do not use the skeleton, two extra steps must be taken when implementing
a service, in the form of two macro calls in your source file:

.. code-block:: cpp

  // Tell the typesystem that class CatImpl implements Cat
  QI_REGISTER_IMPLEMENTATION(Cat, CatImpl);
  // Registers CatImpl to the factory, so that your service (renamed CatService)
  // can be found when the containing shared library is loaded.
  QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR_FOR(CatService, CatImpl)

Build your service implementation
---------------------------------

Your code should be built as a shared library.

Starting your service manually
------------------------------

The *qi-launch* utility can be used to instanciate your implementation, and
register it to an existing *ServiceDirectory*::

  qi-launch -s robotIp catimpl.so

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
    IDL -> Support        [label = "-m client"];
    IDL -> ImplSkel       [label = "-m cxxskel"];
    ImplSkel -> ImplFull  [color=red];
    ImplFull -> ImplSo    [color=blue];
    Support -> SupportSo  [color=blue];

    node [color="white" fontcolor="white" label=""]; la lb lc ld le lf;
    la -> lb [label = "generation using idl"];
    lc -> ld [color=red label="User-written"];
    le->lf [color=blue label="Compilation"];
    }

CMake functions
---------------

If you use qibuild as your build system (which you should), some
functions are provided to integrate the various code generation steps.

- *qi_create_idl(sources... CLASSES classnames... PREFIX dir)*: Will parse
  given sources, and output idl xml files for classes in *classnames* and their
  dependencies.
- *qi_create_client_lib(targetname CLASSES classnames... INCLUDE includes... SRC extrasrcs... PREFIX dir)*
  create the client support library under name *targetname*, with support for
  classes listed in *classnames* and their dependencies. Will try to find XML
  files and headers automatically. *includes* if given is a list of header
  files to include, and overrides autodetection. Extra sources to compile in
  can be given in *extrasrcs*.
- *qi_create_skeleton(output CLASS className INCLUDE includes)*
  create a skeleton implementation for interface *className*.
- *qi_create_interface(_out_filename className)* creates an interface header from
  an idl file. Produced file name will be written in variable *_out_filename*.


Binding an existing class without an interface
----------------------------------------------


