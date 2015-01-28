#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGE_HPP_
#define _SRC_MESSAGE_HPP_

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>
#include <qi/buffer.hpp>
#include <qi/binarycodec.hpp>
#include <qi/anyfunction.hpp>
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
      qi::uint8_t  type;
      qi::uint8_t  flags;
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
    static unsigned int currentVersion()
    {
      return 0;
    }


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
      BoundObjectFunction_Properties        = 7,
      BoundObjectFunction_RegisterEventWithSignature = 8,
    };

    enum ServerFunction
    {
      ServerFunction_Connect           = 4,
      ServerFunction_Authenticate      = 8,
    };

    enum ServiceDirectoryAction {
      ServiceDirectoryAction_Service             = 100,
      ServiceDirectoryAction_Services            = 101,
      ServiceDirectoryAction_RegisterService     = 102,
      ServiceDirectoryAction_UnregisterService   = 103,
      ServiceDirectoryAction_ServiceReady        = 104,
      ServiceDirectoryAction_UpdateServiceInfo   = 105,
      ServiceDirectoryAction_ServiceAdded        = 106,
      ServiceDirectoryAction_ServiceRemoved      = 107,
      ServiceDirectoryAction_MachineId           = 108,
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
      // Advertise capabilities, Server<->Client
      Type_Capability = 6,
      // Cancel request for ongoing call
      Type_Cancel = 7,
      // Method call was cancelled
      Type_Canceled = 8,
    };
    // If flag set, payload is of type m instead of expected type
    static const unsigned int TypeFlag_DynamicPayload = 1;
    /* If flag is set, message payload also contains a type into which
     * the return value must be converted.
     * In that case, Type_DynamicPayload will be set on return message,
     * and signature will be repeated.
     * NOT IMPLEMENTED
     */
    static const unsigned int TypeFlag_ReturnType = 2;

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

    void         setFlags(qi::uint8_t flags);
    void         addFlags(qi::uint8_t flags);
    qi::uint8_t  flags() const;

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


    AnyReference value(const Signature &signature, const qi::TransportSocketPtr &socket) const;
    void setValue(const AutoAnyReference& value, const Signature& signature, ObjectHost* context = 0, StreamContext* streamContext = 0);
    void setValues(const std::vector<qi::AnyReference>& values, ObjectHost* context = 0, StreamContext* streamContext = 0);
    /// Convert values to \p targetSignature and assign to payload.
    void setValues(const std::vector<qi::AnyReference>& values, const qi::Signature& targetSignature, ObjectHost* context = 0, StreamContext* streamContext = 0);
    /// Append additional data to payload
    void appendValue(const AutoAnyReference& value, ObjectHost* context = 0, StreamContext* streamContext = 0);
    MessageAddress address() const;

    bool         isValid();

  public:
    boost::shared_ptr<MessagePrivate> _p;
    void         cow();
  };

  std::ostream& operator<<(std::ostream& os, const qi::MessageAddress &address);
  std::ostream& operator<<(std::ostream& os, const qi::Message& msg);
}

QI_TYPE_CONCRETE(qi::Message);

#endif  // _SRC_MESSAGE_HPP_
