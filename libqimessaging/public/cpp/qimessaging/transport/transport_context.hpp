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

namespace qi {
  namespace transport {

    /// <summary> The context used for Transport. This is typically a
    /// shared resources such as io threads</summary>
    /// \ingroup Transport
    class TransportContext {
    public:
      TransportContext();
      ~TransportContext();

    protected:
      friend class TransportClient;
      friend class TransportServer;
      friend class TransportSubscriber;
      friend class TransportPublisher;
      friend class TransportForwarder;

      /// <summary>Gets a context. The method is protected because,
      /// the return implementation is only relevant to friends
      /// </summary>
      /// <param name="address">An address.</param>
      /// <returns>The context.</returns>
      template <typename T>
      T &getContext(const std::string &address) {
        // we only have one context type, so we don't need address ATM
        (void) address; // prevent warning
        return *static_cast<T*>(_ctx);
      }

    protected:
      /// <summary> The context </summary>
      void *_ctx;

    private:
      //private copy constructor
      TransportContext(const TransportContext &) {;}
    };

  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_CONTEXT_HPP_
