#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_SERVER_HPP_
#define _QI_TRANSPORT_TRANSPORT_SERVER_HPP_

#include <string>
#include <vector>

namespace qi {
  namespace transport {

    namespace detail {
      class ServerBackend;
    };

    class TransportMessageHandler;

    class TransportServer
    {
    public:
      TransportServer();

      void serve(const std::string &endpoint);

      void serve(const std::vector<std::string> &endpoints);

      virtual void run();

      virtual void setMessageHandler(TransportMessageHandler* dataHandler);

      virtual TransportMessageHandler* getMessageHandler();

      bool isInitialized();

    protected:
      bool                                  _isInitialized;
      qi::transport::detail::ServerBackend *_transportServer;
    };

  }

}

#endif  // _QI_TRANSPORT_TRANSPORT_SERVER_HPP_
