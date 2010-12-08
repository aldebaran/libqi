#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_IMPL_BASE_HPP_
#define _QI_MESSAGING_SRC_IMPL_BASE_HPP_

#include <string>
#include <qi/messaging/context.hpp>
#include <qi/messaging/src/network/endpoint_context.hpp>
#include <qi/messaging/src/network/machine_context.hpp>

namespace qi {
  namespace detail {
    class ImplBase {
    public:
      ImplBase() : _isInitialized(false), _qiContextPtr(NULL) {}

      virtual ~ImplBase() {}

      bool isInitialized() const {
        return _isInitialized;
      }

      qi::Context* getQiContextPtr() const {
        return _qiContextPtr;
      }

      /// <summary>Gets the endpoint context. </summary>
      /// <returns>The endpoint context.</returns>
      const qi::detail::EndpointContext& getEndpointContext() const {
        return _endpointContext;
      }

      /// <summary>Gets the machine context. </summary>
      /// <returns>The machine context.</returns>
      const qi::detail::MachineContext& getMachineContext() const {
        return _machineContext;
      }

    protected:
      /// <summary> Indicates if initialization was successful </summary>
      bool _isInitialized;

      /// <summary> The qi Context</summary>
      qi::Context*                _qiContextPtr;

      /// <summary> Context for the machine </summary>
      qi::detail::MachineContext  _machineContext;

      /// <summary> Context for the endpoint </summary>
      qi::detail::EndpointContext _endpointContext;
    };
  }
}

#endif  // _QI_MESSAGING_SRC_IMPL_BASE_HPP_

