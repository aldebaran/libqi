#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_CONTEXT_HPP_
#define _QI_TRANSPORT_TRANSPORT_CONTEXT_HPP_

#include <string>
#include <qi/transport/buffer.hpp>

namespace qi {
  namespace transport {

    class TransportContext {
    public:
      TransportContext(const std::string &address = "");
      ~TransportContext();

    //TODO: protected:
      template <typename T>
      T &getContext() {
        return *static_cast<T*>(_ctx);
      }

    protected:
      void *_ctx;
    };

  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_
