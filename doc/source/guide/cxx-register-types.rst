.. _guide-cxx-register-types:

.. role:: cpp(code)
   :language: cpp

Registering types in the type system
************************************

Introduction
============

For types to be available in the type system and allow them to be type-erased,
sent over the network or through signals, each one of them needs to be
registered in the type system.

Most of these registration can be done through macros. It is very important
that you call macros made to be used in .cpp only in .cpp files, these macros
should be processed in a single translation unit.

All registration classes and macros are in:

.. code-block:: cpp

  #include <qi/anyobject.hpp>

Structures
==========

To register a structure, you need to call the macro `QI_TYPE_STRUCT()` in the
corresponding .hpp file.

.. code-block:: cpp

  // point.hpp

  namespace Graph {
    struct Point {
      int x;
      int y;
    };
  }

  QI_TYPE_STRUCT(Graph::Point, x, y);

Or you can do the register in the .cpp:

.. code-block:: cpp

  // point.cpp

  // call this outside of any namespace
  QI_TYPE_STRUCT_REGISTER(Graph::Point, x, y);

Using PIMPL in structures
-------------------------

Sometime, you have a structure of data, but you don't want the user to access
it directly because you want to constrain values or to updates multiple values
at the same time so that the structure stays coherent.

.. code-block:: cpp

  class Vector {
    public:
      void setValues(int x, int y)
      {
        _x = x;
        _y = y;
        _length = std::sqrt(x*x + y*y);
      }
      float length() { return _length; }
      // ...

    private:
      int _x;
      int _y;
      float _length;
  };

  // How can I register this??

You can't register this struct because _x, _y and _length are private and you
don't want to make them public because the user could change _x and _y without
changing _length.

To register that to the type system, you need to use pimpl:

.. code-block:: cpp

  // vector.hpp

  class Vector {
    public:
      Vector();
      // you need a copy constructor because qitype makes copies
      Vector(const Vector& other);
      void setValues(int x, int y) { /* ... */ }
      float length() { /* ... */ }

    private:
      boost::scoped_ptr<struct VectorPrivate> _p;

      // you need this for later
      friend struct VectorPrivate* vectorPrivateAccess(Vector*);
  };

  // vector.cpp

  struct VectorPrivate {
    int _x;
    int _y;
    float _length;
  };

  Vector::Vector() : _p(new VectorPrivate) {}
  Vector::Vector(const Vector& other) : _p(new VectorPrivate(*other._p)) {}

Then you can register the private part of the struct and tell qitype how to
access it:

.. code-block:: cpp

  VectorPrivate* vectorPrivateAccess(Vector* vector) {
    return vector->_p;
  }

  // call these outside of any namespace
  QI_TYPE_STRUCT_REGISTER(VectorPrivate, _x, _y, _length);

  QI_TYPE_STRUCT_BOUNCE_REGISTER(Vector, VectorPrivate, vectorPrivateAccess);

Everytime you transfer a Vector, qimessaging will also transfer its private
part and no one can access it without using the accessors.

Enums
=====

Enums are easy to register:

.. code-block:: cpp

  // color.hpp

  namespace Graph {
    enum Color {
      Red,
      Green,
      Blue
    };
  }

  // call this outside of any namespace
  QI_TYPE_ENUM_REGISTER(Graph::Color);

Classes
=======

Using registration helper
-------------------------

Classes can only be registered in .cpp files:

.. code-block:: cpp

  // drawer.hpp

  namespace Graph {
    class Drawer {
      public:
        bool draw(const Point& p, Color color) {
          std::cout << "Drawing point" << std::endl;
          drawDone(p);
          return true;
        }

        qi::Signal<Point> drawDone;

        qi::Property<Point> origin;
    };
  }

  // drawer.cpp

  namespace Graph {
    // call this from inside the namespace of the class
    QI_REGISTER_OBJECT(Drawer, draw, drawDone, origin);
  }

There are two threading models for classes. `Drawer` is registered as
single threaded in the above example. When doing multiple calls of its
methods in parallel, they will be sequenced. If you need your object to support
multithreaded calls, use the MT macro:

.. code-block:: cpp

  // drawer.cpp

  namespace Graph {
    // call this from inside the namespace of the class
    QI_REGISTER_MT_OBJECT(Drawer, draw, drawDone, origin);
  }

Doing it manually
-----------------

The helper won't always allow you to register a class, for example when you
have method overloading in your class. In these cases, you need to register
your type manually so that you can specify the signature of the function.

.. code-block:: cpp

  // drawer.hpp

  namespace Graph {
    class Drawer {
      public:
        void draw(const Point& p, Color color) {
          std::cout << "Drawing point with color" << std::endl;
        }
        void draw(const Point& p) {
          std::cout << "Drawing point" << std::endl;
        }
    };
  }

  // drawer.cpp

  namespace Graph {
    // this won't work because we can't differenciate the two draw methods
    //QI_REGISTER_OBJECT(Drawer, draw, draw);
  }

  namespace Graph {
    static bool _qiregisterDrawer() {
      ::qi::ObjectTypeBuilder<Drawer> builder;
      builder.advertise("draw",
          static_cast<void (Drawer::*)(const Point&, Color)>(&Drawer::draw));
      builder.advertise("draw",
          static_cast<void (Drawer::*)(const Point&)>(&Drawer::draw));
      builder.registerType();
      return true;
    }
    static bool __qi_registrationDrawer = _qiregisterDrawer();
  }

Factories
=========

Sometimes, you may need to create objects from a type-erased context.
Registering classes is not enough to instantiate them through the
type system. For that, you need to register factories in the .cpp file. To
register a factory which will just call the default constructor, use:

.. code-block:: cpp

  // drawer.cpp

  // you can put that in a namespace
  QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(Graph::Drawer);

This will create a factory named `"Graph::Drawer"`. If you want a different
name, you can use:

.. code-block:: cpp

  QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR_FOR("MyDrawer", Graph::Drawer);

.. note::

  Factories are unique. You can't have two factories with the same name!

If you want to pass arguments to the constructor, you need to specify the
signature to the macro:

.. code-block:: cpp

  // drawer.hpp

  class Drawer {
    Drawer(int width, int height) {}
  }

  // drawer.cpp

  QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(Drawer, int, int);
