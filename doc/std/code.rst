.. _std-code-convention:

Coding Convention
=================

Backward Compatibility
----------------------

http://wiki.qt-project.org/API_Design_Principles


Naming convention
-----------------

- Headers go into <libname>/*
- Sources and private headers only go into <src>/*
- Private headers should be named '<classname>_p.hpp'
- Qt classes are not in a namespace and are prefixed with 'Qi'

Example
+++++++
For the *foo* class in the *bar* library we have:

.. code-block:: console

  bar/foo.hpp
  src/foo.cpp
  src/foo_p.hpp

  qt/bar/qt/qifoo.h
  qt/src/qifoo.cpp
  qt/src/qifoo_p.h

This gives us the following objects:

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

Headers
--------------

Public headers must not include other public headers from our libraries. This
ensures binary compatibility.

Public headers must be enclosed within brackets <> when included.

On the other hand, private headers must be enclosed within double quotes "" when
included.

Private Implementation
----------------------

- Use private implementation where applicable.
- Still reserve a pointer instead if you dont use it. (for future use, see
  example two).
- Classes should be named <classname>Private.
- A pointer '_p' should be added into the class.

When a class has a private implementation, the copy constructor *must* be either
implemented, either disabled - *ie.* defined in the private section of the class.

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

Exceptions are prohibited, it's really hard to write code that do not leak using.

Enum
----

One must used the singular when naming an enumeration.

Enum values should be prefixed by the enum name followed by an underscore.

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

- Private members names should be prefixed with underscores.
