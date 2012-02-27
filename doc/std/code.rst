.. _std-code-convention:

Code Convention
===============

Backward Compatibility
----------------------

http://wiki.qt-project.org/API_Design_Principles


Naming convention
-----------------

- headers goes in <libname>/*
- sources and private headers goes into <src>/*
- Private headers should be named '<classname>_p.hpp'
- Qt classes are not in a namespace and are prefixed by 'Qi'

Example
+++++++
for the foo class in the bar library we have:

.. code-block:: console

  bar/foo.hpp
  src/foo.cpp
  src/foo_p.hpp

  qt/bar/qt/qifoo.h
  qt/src/qifoo.cpp
  qt/src/qifoo_p.h

This give us the following objects:

STL:

.. code-block:: cpp

  #include <bar/foo.hpp>

  int main(void) {
     qi::Foo foo;
  }

QT:

.. code-block:: cpp

  #include <bar/qt/qifoo.h>

  int main(void) {
     QiFoo foo;
  }


Private Implementation
----------------------

- Use private implementation where applicable.
- Reserve a pointer instead if you dont use it. (for future use, see example two)
- classes should be named <classname>Private

a pointer '_p' should be added into the class:

Example with Pimpl
++++++++++++++++++

bar/foo.hpp:

.. code-block:: cpp

  class FooPrivate;
  class Foo {
    FooPrivate *_p;
  };


Example without Pimpl
+++++++++++++++++++++

.. code-block:: cpp

  class Foo {
  public:

  protected:
    //could be used to create a future pimpl if needed without breaking ABI
    void *_reserved;
    int   _mymember;
  };


Exception
---------

Do not use exception, it's really hard to write code that do not leak using.

Enum
----

enum value should be prefixed by the enum name followed by an underscore.

.. code-block:: c++

  class Message {
  public:

    enum Type {
      Type_Call = 0,
      Type_Error,
      Type_Answer,
      Type_Event
    };

  };


Members
-------

- members should be prefixed by an underscore
