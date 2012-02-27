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
#include <stdint.h>
#include <qimessaging/buffer.hpp>

namespace qi {

  /** \class qi::Message
    * This class represent a network message
    */
  class QIMESSAGING_API Message {
  public:

    typedef struct
    {
      uint32_t magic;
      uint32_t id;
      uint32_t size;
      uint32_t type;
      uint32_t service;
      uint32_t path;
      uint32_t function;
      uint32_t reserved;
    } MessageHeader;

    enum Service
    {
      Service_None             = 0,
      Service_ServiceDirectory = 1,
    };

    enum Path
    {
      Path_none = 0,
      Path_Main = 1,
    };

    enum Function
    {
      Function_MetaObject = 0,
    };

    enum ServiceDirectoryFunction
    {
      ServiceDirectoryFunction_Service         = 1,
      ServiceDirectoryFunction_Services        = 2,
      ServiceDirectoryFunction_RegisterService = 3,
    };

    enum Type
    {
      Type_None  = 0,
      Type_Call  = 1,
      Type_Reply = 2,
      Type_Event = 3,
      Type_Error = 4,
    };

    Message();
    Message(qi::Buffer *buf);

    ~Message();

    size_t size() const;
    void setId(unsigned int id);
    unsigned int id() const;
    void setType(uint32_t type);
    unsigned int type() const;
    void setService(uint32_t service);
    unsigned int service() const;
    void setPath(uint32_t path);
    unsigned int path() const;
    void setFunction(uint32_t function);
    unsigned int function() const;
    Buffer *buffer() const;

    bool complete();
    void buildReplyFrom(const Message &call);
    void buildForwardFrom(const Message &msg);
  public:
    MessageHeader *_header;

  protected:
    qi::Buffer    *_buffer;
    bool           _withBuffer;
  };
  std::ostream& operator<<(std::ostream& os, const qi::Message& msg);

}


#endif  // _QIMESSAGING_MESSAGE_HPP_
