#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_MESSAGE_HPP_
#define _QIMESSAGING_MESSAGE_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
#include <qitype/genericvalue.hpp>
#include <qi/buffer.hpp>
#include <qitype/functiontype.hpp>
#include <qi/types.hpp>


namespace qi {

  class MessagePrivate
  {
  public:
    typedef struct
    {
      qi::uint32_t magic;
      qi::uint32_t id;
      qi::uint32_t size;
      qi::uint16_t version;
      qi::uint16_t type;
      qi::uint32_t service;
      qi::uint32_t object;
      qi::uint32_t action;
    } MessageHeader;

    MessagePrivate();
    MessagePrivate(const MessagePrivate& b);
    ~MessagePrivate();

    inline void                complete() { header.size = buffer.totalSize(); }
    inline void               *getHeader() { return reinterpret_cast<void *>(&header); }

    Buffer        buffer;
    std::string   signature;
    MessageHeader header;

    static const unsigned int magic = 0x42adde42;
  };

  class MessageAddress {
  public:
    MessageAddress()
      : messageId(0)
      , serviceId(0)
      , objectId(0)
      , functionId(0)
    {}

    MessageAddress(unsigned int messageId,
                   unsigned int serviceId,
                   unsigned int objectId,
                   unsigned int functionId)
      : messageId(messageId)
      , serviceId(serviceId)
      , objectId(objectId)
      , functionId(functionId)
    {}

    unsigned int messageId;
    unsigned int serviceId;
    unsigned int objectId;
    unsigned int functionId;
  };


  /** \class qi::Message
    * This class represent a network message
    */
  class TransportSocket;
  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;
  class ObjectHost;

  class Message {
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

    enum BoundObjectFunction {
      BoundObjectFunction_RegisterEvent     = 0,
      BoundObjectFunction_UnregisterEvent   = 1,
      BoundObjectFunction_MetaObject        = 2,
      BoundObjectFunction_Terminate         = 3,
      BoundObjectFunction_GetProperty       = 5,
      BoundObjectFunction_SetProperty       = 6,
      BoundObjectFunction_Properties        = 7
    };

    enum ServerFunction
    {
      ServerFunction_Connect           = 4,
    };

    enum ServiceDirectoryAction {
      ServiceDirectoryAction_Service             = 10,
      ServiceDirectoryAction_Services            = 11,
      ServiceDirectoryAction_RegisterService     = 12,
      ServiceDirectoryAction_UnregisterService   = 13,
      ServiceDirectoryAction_ServiceReady        = 14,
      ServiceDirectoryAction_UpdateServiceInfo   = 15,
      ServiceDirectoryAction_ServiceAdded        = 16,
      ServiceDirectoryAction_ServiceRemoved      = 17,
    };

    enum Type
    {
      Type_None  = 0,
      // Method call, Client->Server (wait for a Type_Reply or Type_Error)
      Type_Call  = 1,
      // Method return value, Server->Client (only to answer to a Type_Call)
      Type_Reply = 2,
      // Method return error, Server->Client (only to answer to a Type_Call)
      Type_Error = 3,
      // Method call, Client->Server (No answer needed)
      Type_Post  = 4,
      // Event Server->Client
      Type_Event = 5,
    };

    static const char* typeToString(Type t);
    static const char* actionToString(unsigned int action, unsigned int service);

    ~Message();
    Message();
    Message(Type type, const MessageAddress &address);
    Message(const Message &msg);
    Message &operator=(const Message &msg);

    void setAddress(const MessageAddress &address);

    void         setId(unsigned int id);
    unsigned int id() const;

    void         setVersion(qi::uint16_t type);
    unsigned int version() const;

    void         setType(Type type);
    Type         type() const;

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

    void          setError(const std::string &error);

    ///@return signature, set by setParameters() or setSignature()

    GenericValuePtr value(const std::string &signature, const qi::TransportSocketPtr &socket) const;
    void setValue(const qi::GenericValuePtr &value, ObjectHost* context = 0);
    qi::GenericValuePtr setValues(const std::vector<qi::GenericValuePtr> &value, ObjectHost* context = 0);
    MessageAddress address() const;

    bool         isValid();

  public:
    boost::shared_ptr<MessagePrivate> _p;
    void         cow();
  };

  QIMESSAGING_API std::ostream& operator<<(std::ostream& os, const qi::MessageAddress &address);
  QIMESSAGING_API std::ostream& operator<<(std::ostream& os, const qi::Message& msg);
}


#endif  // _QIMESSAGING_MESSAGE_HPP_
