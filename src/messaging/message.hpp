#pragma once

/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGE_HPP_
#define _SRC_MESSAGE_HPP_

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>
#include <qi/anyobject.hpp>
#include <qi/buffer.hpp>
#include <qi/binarycodec.hpp>
#include <qi/anyfunction.hpp>
#include <qi/types.hpp>
#include <ka/macroregular.hpp>
#include <qi/assert.hpp>
#include <qi/messaging/messagesocket_fwd.hpp>
#include <ka/scoped.hpp>
#include <boost/weak_ptr.hpp>

namespace qi {

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
    KA_GENERATE_FRIEND_REGULAR_OPS_4(MessageAddress, messageId, serviceId, objectId, functionId)
  };


  /** \class qi::Message
    * This class represent a network message
    */
  class ObjectHost;

  class Message
  {
  public:
    // Fixed service id numbers
    enum Service
    {
      Service_Server           = 0,
      Service_ServiceDirectory = 1,
    };

    enum GenericObject
    {
      GenericObject_None = nullObjectId,
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

    struct Header
    {
      qi::uint32_t magic = magicCookie;
      qi::uint32_t id = newMessageId();
      qi::uint32_t size = 0;
      qi::uint16_t version = currentVersion();
      qi::uint8_t  type = 0;
      qi::uint8_t  flags = 0;
      qi::uint32_t service = 0;
      qi::uint32_t object = 0;
      qi::uint32_t action = 0;

      friend KA_GENERATE_REGULAR_OP_EQUAL_9(Header,
        magic, id, size, version, type, flags, service, object, action
      )
      friend KA_GENERATE_REGULAR_OP_DIFFERENT(Header)

      static inline qi::uint16_t currentVersion()
      {
        return 0;
      }

      Type messageType() const
      {
        return static_cast<Type>(type);
      }

      MessageAddress address() const
      {
        return MessageAddress(id, service, object, action);
      }

      QI_API static qi::uint32_t newMessageId();

      QI_API static const qi::uint32_t magicCookie;
    };
    static_assert(sizeof(Header) == 28, "Message::Header does not have the right size!");

    QI_API static const char* typeToString(Type t);
    QI_API static const char* actionToString(unsigned int action, unsigned int service);

    Message() = default;

    Message(Type type, const MessageAddress &address)
    {
      setType(type);
      setAddress(address);
    }

    void setAddress(const MessageAddress &address)
    {
      _header.id = address.messageId;
      _header.service = address.serviceId;
      _header.object = address.objectId;
      _header.action = address.functionId;
    }

    void setId(unsigned int id)
    {
      _header.id = id;
    }

    unsigned int id() const
    {
      return _header.id;
    }

    void setVersion(qi::uint16_t version)
    {
      _header.version = version;
    }

    unsigned int version() const
    {
      return _header.version;
    }

    void setType(Type type)
    {
      _header.type = type;
    }

    Type type() const
    {
      return _header.messageType();
    }

    void setFlags(qi::uint8_t flags)
    {
      _header.flags = flags;
    }

    void addFlags(qi::uint8_t flags)
    {
      _header.flags |= flags;
    }

    qi::uint8_t  flags() const
    {
      return _header.flags;
    }

    void setService(qi::uint32_t service)
    {
      _header.service = service;
    }

    unsigned int service() const
    {
      return _header.service;
    }

    void setObject(qi::uint32_t object)
    {
      _header.object = object;
    }

    unsigned int object() const
    {
      return _header.object;
    }

    QI_API void setFunction(qi::uint32_t function);

    QI_API unsigned int function() const;

    QI_API void setEvent(qi::uint32_t event);

    QI_API unsigned int event() const;

    unsigned int action() const
    {
      return _header.action;
    }

    void setBuffer(const Buffer &buffer)
    {
      _buffer = buffer;
      _header.size = static_cast<qi::uint32_t>(_buffer.totalSize());
    }

    void setBuffer(Buffer&& buffer)
    {
      _buffer = std::move(buffer);
      _header.size = static_cast<qi::uint32_t>(_buffer.totalSize());
    }

    const Buffer& buffer() const
    {
      return _buffer;
    }

    Buffer extractBuffer()
    {
      Buffer extracted = std::move(_buffer);
      _buffer.clear();
      return extracted;
    }

    void setError(const std::string &error)
    {
      QI_ASSERT(type() == Type_Error && "called setError on a non Type_Error message");

      // Clear the buffer before setting an error.
      _buffer.clear();
      _header.size = static_cast<qi::uint32_t>(_buffer.totalSize());

      // Error message is of type m (dynamic)
      AnyValue v(AnyReference::from(error), false, false);
      setValue(AnyReference::from(v), "m");
    }

    ///@return signature, set by setParameters() or setSignature()
    QI_API AnyValue value(const Signature &signature, const qi::MessageSocketPtr &socket) const;

    QI_API void setValue(const AutoAnyReference& value,
                  const Signature& signature,
                  boost::weak_ptr<ObjectHost> context = {},
                  MessageSocketPtr socket = {});

    QI_API void setValues(const std::vector<qi::AnyReference>& values,
                   boost::weak_ptr<ObjectHost> context = {},
                   MessageSocketPtr socket = {});

    /// Convert values to \p targetSignature and assign to payload.
    QI_API void setValues(const std::vector<qi::AnyReference>& values,
                   const qi::Signature& targetSignature,
                   boost::weak_ptr<ObjectHost> context = {},
                   MessageSocketPtr socket = {});

    MessageAddress address() const
    {
      return _header.address();
    }

    Header& header() {return _header;}

    const Header& header() const {return _header;}

    friend KA_GENERATE_REGULAR_OP_EQUAL_3(Message, _header, signature, _buffer)
    friend KA_GENERATE_REGULAR_OP_DIFFERENT(Message)

  private:
    Buffer _buffer;
    std::string signature;
    Header _header;

    void encodeBinary(const qi::AutoAnyReference& ref,
                      SerializeObjectCallback onObject,
                      MessageSocketPtr socket)
    {
      auto updateHeaderSize =
          ka::scoped([&] { _header.size = static_cast<qi::uint32_t>(_buffer.totalSize()); });
      qi::encodeBinary(&_buffer, ref, onObject, socket);
    }
  };

  inline std::ostream& operator<<(std::ostream& os, const qi::MessageAddress &address)
  {
    os << "{" << address.serviceId << "." << address.objectId << "." << address.functionId
       << ", id:" << address.messageId << "}";
    return os;
  }

  inline std::ostream& operator<<(std::ostream& os, const qi::Message::Type& type)
  {
    return os << Message::typeToString(type);
  }

  QI_API std::ostream& operator<<(std::ostream& os, const qi::Message& msg);


  /// Status returned by message handlers.
  /// This is used mainly to determine if a message have been handled or not
  /// and how to route it to another handler if it was not handled yet.
  enum class DispatchStatus
  {
    MessageHandled,           ///< The message was understood and processed without error.

    MessageHandled_WithError, ///< The message was understood, but it's processing lead
                              ///< to an error (for example: the requested function call is not found).

    MessageNotHandled,        ///< The message was not understood or not handled  on purpose.
                              ///< (for example: the handler isn't the recipient or don't know how to handle this message).

    MessageHandlingFailure,   ///< An error happened before the message was handled.
                              ///< (for example: an exception thrown in a message handler).
  };

  /// @returns true if the message was handled, with or without error.
  inline bool isMessageHandled(DispatchStatus status)
  {
    return status == DispatchStatus::MessageHandled
        || status == DispatchStatus::MessageHandled_WithError
        ;
  }

}

QI_TYPE_CONCRETE(qi::Message);

#endif  // _SRC_MESSAGE_HPP_
