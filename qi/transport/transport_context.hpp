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
      TransportContext();
      ~TransportContext();

    //TODO: protected:
      template <typename T>
      T &getContext(const std::string &address) {
        //we only have one context type, so we dont need address ATM
        (void) address;
        return *static_cast<T*>(_ctx);
      }

    protected:
      void *_ctx;
    };

  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_
