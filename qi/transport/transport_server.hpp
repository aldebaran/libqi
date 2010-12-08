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
    class TransportContext;

    /// \ingroup Transport
    class TransportServer
    {
    public:
      TransportServer(TransportContext &context);

      void serve(const std::string &endpoint);

      void serve(const std::vector<std::string> &endpoints);

      virtual void run();

      /// <summary>Sets a message handler. </summary>
      /// <param name="dataHandler">A pointer to a class derived from
      /// TransportMessageHandler.</param>
      virtual void setMessageHandler(TransportMessageHandler* dataHandler);

      /// <summary>Gets the message handler. </summary>
      /// <returns>
      /// null if it fails, otherwise a pointer to a class derived from
      /// TransportMessageHandler.
      /// </returns>
      virtual TransportMessageHandler* getMessageHandler();

      bool isInitialized();

    protected:
      bool                                  _isInitialized;
      qi::transport::TransportContext      &_transportContext;
      qi::transport::detail::ServerBackend *_transportServer;
    };

  }

}

#endif  // _QI_TRANSPORT_TRANSPORT_SERVER_HPP_
