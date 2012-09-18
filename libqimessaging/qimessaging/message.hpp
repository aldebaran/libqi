/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Laurent Lec   <llec@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_MESSAGE_HPP_
#define _QIMESSAGING_MESSAGE_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/buffer.hpp>
#include <qimessaging/datastream.hpp>
#include <qi/types.hpp>

namespace qi {

  class QIMESSAGING_API MessageAddress {
  public:
    MessageAddress()
      : type(0)
      , messageId(0)
      , serviceId(0)
      , objectId(0)
      , functionId(0)
    {}

    MessageAddress(unsigned int type,
                   unsigned int messageId,
                   unsigned int serviceId,
                   unsigned int objectId,
                   unsigned int functionId)
      : type(type)
      , messageId(messageId)
      , serviceId(serviceId)
      , objectId(objectId)
      , functionId(functionId)
    {}

    unsigned int type;
    unsigned int messageId;
    unsigned int serviceId;
    unsigned int objectId;
    unsigned int functionId;
  };

  /** \class qi::Message
    * This class represent a network message
    */
  class MessagePrivate;
  class QIMESSAGING_API Message {
  public:


    // Fixed service id numbers
    enum Service
    {
      Service_Server           = 0,
      Service_ServiceDirectory = 1,
    };

    enum GenericObject
    {
      GenericObject_None = 0,
      GenericObject_Main = 1
    };

    enum ServiceDirectoryFunction
    {
      ServiceDirectoryFunction_Service           = 0,
      ServiceDirectoryFunction_Services          = 1,
      ServiceDirectoryFunction_RegisterService   = 2,
      ServiceDirectoryFunction_UnregisterService = 3,
      ServiceDirectoryFunction_ServiceReady      = 4,
    };

    enum ServiceDirectoryEvent
    {
      ServiceDirectoryEvent_ServiceRegistered   = 5,
      ServiceDirectoryEvent_ServiceUnregistered = 6,
    };

    enum ServerFunction
    {
      ServerFunction_RegisterEvent     = 0,
      ServerFunction_UnregisterEvent   = 1,
      ServerFunction_MetaObject        = 2,
      ServerFunction_Connect           = 3,
    };

    enum Type
    {
      Type_None  = 0,
      // Method call, Client->Server
      Type_Call  = 1,
      // Method return value, Server->Client
      Type_Reply = 2,
      // Event, or method call without caring about return. Server<->Client
      Type_Event = 3,
      Type_Error = 4,
    };

    ~Message();
    Message();
    Message(const Message &msg);
    Message &operator=(const Message &msg);

    void         setId(unsigned int id);
    unsigned int id() const;

    void         setVersion(qi::uint16_t type);
    unsigned int version() const;

    void         setType(qi::uint16_t type);
    unsigned int type() const;

    void         setService(qi::uint32_t service);
    unsigned int service() const;

    void         setObject(qi::uint32_t object);
    unsigned int object() const;

    void         setFunction(qi::uint32_t function);
    unsigned int function() const;

    void         setEvent(qi::uint32_t event);
    unsigned int event() const;

    unsigned int action() const;

    void          setBuffer(const Buffer &buffer);
    const Buffer &buffer() const;

    void         buildReplyFrom(const Message &call);
    void         buildForwardFrom(const Message &msg);

    MessageAddress address()const;

    bool         isValid();

  public:
    MessagePrivate *_p;
  };

  QIMESSAGING_API std::ostream&   operator<<(std::ostream&   os, const qi::Message& msg);
}


#endif  // _QIMESSAGING_MESSAGE_HPP_
