#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_DETAIL_IMPL_BASE_HPP_
#define _QI_MESSAGING_DETAIL_IMPL_BASE_HPP_

#include <string>
#include <qi/messaging/context.hpp>
#include <qi/messaging/src/network/endpoint_context.hpp>
#include <qi/messaging/src/network/machine_context.hpp>

namespace qi {
  namespace detail {
    class ImplBase {
    public:
      ImplBase() : _isInitialized(false) {}

      virtual ~ImplBase() {}

      bool isInitialized() const {
        return _isInitialized;
      }

      const qi::detail::EndpointContext& getEndpointContext() const {
        return _endpointContext;
      }

      const qi::detail::MachineContext& getMachineContext() const {
        return _machineContext;
      }

    protected:
      /// <summary> Indicates if initialization was successful </summary>
      bool _isInitialized;

      qi::Context                 _qiContext;
      qi::detail::MachineContext  _machineContext;
      qi::detail::EndpointContext _endpointContext;
    };
  }
}

#endif  // _QI_MESSAGING_DETAIL_IMPL_BASE_HPP_

