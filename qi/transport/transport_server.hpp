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
      virtual ~TransportServer();

      TransportServer(const TransportServer& rhs);

      /// <summary>Constructor. </summary>
      /// <param name="context"> The context.</param>
      TransportServer(TransportContext &context);

      /// <summary>Serves</summary>
      /// <param name="endpoint">The endpoint.</param>
      void serve(const std::string &endpoint);

      /// <summary>Serves.</summary>
      /// <param name="endpoints">The endpoints.</param>
      void serve(const std::vector<std::string> &endpoints);

      /// <summary>Runs this object. </summary>
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

      /// <summary>Query if this object is initialized. </summary>
      /// <returns>true if initialized, false if not.</returns>
      bool isInitialized();

    protected:
      /// <summary> true if is initialized </summary>
      bool                                  _isInitialized;

      /// <summary> Context for the transport </summary>
      qi::transport::TransportContext      &_transportContext;

      /// <summary> The transport server </summary>
      qi::transport::detail::ServerBackend *_transportServer;
    };

  }

}

#endif  // _QI_TRANSPORT_TRANSPORT_SERVER_HPP_
