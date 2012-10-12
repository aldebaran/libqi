qi::Signature
=============

.. code-block:: c++

  #include <qitype/signature.hpp>

.. cpp:namespace:: qi

Brief
-----

.. cpp:brief::

Details
-------

Signature are so old school, prefer SSL certificate.


Example
-------

.. literalinclude:: /examples/example_qi_signature_function.cpp
   :language: c++

.. literalinclude:: /examples/example_qi_signature_instance.cpp
   :language: c++

.. literalinclude:: /examples/example_qi_signature_iterator.cpp
   :language: c++

.. literalinclude:: /examples/example_qi_signature_pp.cpp
   :language: c++

.. literalinclude:: /examples/example_qi_signature_type.cpp
   :language: c++


Classes
-------

  //this is the entry point of all the signature machinery

  /// \ingroup Signature
  /// \include example_qi_signature_type.cpp

.. cpp:function:: std::string &qi::signatureFromType<T>::value(std::string &valueRef)

  Return a signature based on the templated type T, provided in parameter of the struct.
  Ref and const are not computed, those qualifiers are compiler details.

  :param: a string to append the signature to
  :return: a reference to the string passed in parameter

.. cpp:function:: std::string qi::signatureFromType<T>::value()

  Return a signature based on the templated type T, provided in parameter of the struct.
  Ref and const are not computed, those qualifiers are compiler details.

  :return: the signature


.. cpp:function:: std::string &qi::signatureFromObject<T>::value(const T *t, std::string &valueRef)

  :return: a reference to the string passed in parameter

.. cpp:function:: std::string qi::signatureFromObject<T>::value(const T *t)

  :return: the signature

.. cpp:function:: std::string &qi::signatureFromObject<T>::value(const T &t, std::string &valueRef)

  :return: a reference to the string passed in parameter

.. cpp:function:: std::string qi::signatureFromObject<T>::value(const T &t)

  :return: the signature








.. cpp:class:: qi::Signature

  .. cpp:type:: Type

    .. code-block:: c++

      //TODO use the type than "network type"
      enum Type {
        None     = 0,
        Bool     = 'b',
        Char     = 'c',
        UChar    = 'C',
        Void     = 'v',
        Int      = 'i',
        UInt     = 'I',
        Float    = 'f',
        Double   = 'd',
        String   = 's',
        List     = '[',
        Map      = '{',
        Tuple    = '(',
        GenericObject   = '@'
      };

  .. cpp:function:: Signature(const char *fullSignature = 0)

  .. cpp:function:: Signature(const std::string &fullSignature)

  .. cpp:function:: bool isValid() const

  .. cpp:function:: iterator begin() const

  .. cpp:function:: iterator end() const

  .. cpp:function:: std::string toSTLSignature(bool constify = false)

  .. cpp:function:: std::string toQtSignature(bool constify = false)



.. cpp:class:: qi::Signature::iterator

  .. cpp:function:: iterator()

    Constructor

  .. cpp:function:: iterator          &operator++()

  .. cpp:function:: iterator          &operator++(int)

  .. cpp:function:: bool        operator!=(const iterator &rhs) const

  .. cpp:function:: bool        operator==(const iterator &rhs) const

  .. cpp:function:: std::string operator*() const

  .. cpp:function:: std::string operator->() const

  .. cpp:function:: Type        type()const

  .. cpp:function:: std::string signature()const

  .. cpp:function:: bool        isValid()const

  .. cpp:function:: int         pointer()const

  .. cpp:function:: bool        hasChildren()const

  .. cpp:function:: Signature   children()const
