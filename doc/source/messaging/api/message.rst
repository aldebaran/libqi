qi::Message
===========

.. code-block:: c++

  #include "message.hpp"

.. cpp:namespace:: qi

Brief
-----

.. cpp:brief::

Details
-------

Message are useless

Example
-------

.. todo:: literalinclude:: /examples/example_qi_message.cpp
   :language: c++

Classes
-------

.. cpp:class:: Message

  .. cpp:type:: enum Service

    .. code-block:: cpp

      enum qi::Message::Service
      {
        Service_None             = 0,
        Service_ServiceDirectory = 1
      };

  .. cpp:type:: enum Path

    .. code-block:: cpp

      enum Path
      {
        Path_None = 0,
        Path_Main = 1
      };

  .. cpp:type:: enum Function

    .. code-block:: cpp

      enum Function
      {
        Function_MetaObject = 0
      };

  .. cpp:type:: enum ServiceDirectoryFunction

    .. code-block:: cpp

      enum ServiceDirectoryFunction
      {
        ServiceDirectoryFunction_Service           = 1,
        ServiceDirectoryFunction_Services          = 2,
        ServiceDirectoryFunction_RegisterService   = 3,
        ServiceDirectoryFunction_UnregisterService = 4,
      };

  .. cpp:type:: enum Type

    .. code-block:: cpp

      enum Type
      {
        Type_None  = 0,
        Type_Call  = 1,
        Type_Reply = 2,
        Type_Event = 3,
        Type_Error = 4
      };

    .. cpp:function:: ~Message()

      Destructor

    .. cpp:function:: Message()

      Constructor

    .. cpp:function:: Message(const Message &msg)

      Copy constructor

    .. cpp:function:: Message &operator=(const Message &msg)

      Copy operator

    .. cpp:function:: void         setId(unsigned int id)

      :param id:

    .. cpp:function:: unsigned int id() const

    .. cpp:function:: void         setType(uint32_t type)

    .. cpp:function:: unsigned int type() const

    .. cpp:function:: void         setService(uint32_t service)

    .. cpp:function:: unsigned int service() const

    .. cpp:function:: void         setPath(uint32_t path)

    .. cpp:function:: unsigned int path() const

    .. cpp:function:: void         setFunction(uint32_t function)

    .. cpp:function:: unsigned int function() const

    .. cpp:function:: void          setBuffer(const Buffer &buffer)

    .. cpp:function:: const Buffer &buffer() const

    .. cpp:function:: void         buildReplyFrom(const Message &call)

    .. cpp:function:: void         buildForwardFrom(const Message &msg)

    .. cpp:function:: bool         isValid()


.. cpp:function:: std::ostream&   operator<<(std::ostream&   os, const qi::Message& msg)

  Display a message. Mostly useful for debug.

  :param os: the output stream
  :param msg: the message to display
  :return: the output stream

