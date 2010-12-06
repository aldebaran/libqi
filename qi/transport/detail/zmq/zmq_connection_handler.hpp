#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_CONNECTION_HANDLER_HPP_
#define _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_CONNECTION_HANDLER_HPP_

#include <qi/core/runnable.hpp>
#include <qi/transport/buffer.hpp>
#include <qi/transport/message_handler.hpp>
#include <qi/transport/detail/server_response_handler.hpp>
#include <string>

namespace qi {
  namespace transport {
    namespace detail {
      /// <summary>
      /// A connection handler created for each new incoming connection and
      /// pushed to the thread pool.
      /// </summary>
      class ZMQConnectionHandler: public qi::detail::Runnable {
      public:

        /// <summary> Constructor. </summary>
        /// <param name="msg"> The message. </param>
        /// <param name="dataHandler"> The data handler. </param>
        /// <param name="serverResponseHander"> The server response hander </param>
        /// <param name="data"> [in,out] If non-null, the data. </param>
        ZMQConnectionHandler(const std::string               &msg,
                             qi::transport::MessageHandler   *dataHandler,
                             ServerResponseHandler           *serverResponseHander,
                             void                            *data);

        /// <summary> Finaliser. </summary>
        virtual ~ZMQConnectionHandler ();

        /// <summary> Runs this object. </summary>
        virtual void run ();

      private:
        void                             *fData;
        qi::transport::Buffer             fMsg;
        MessageHandler                   *fDataHandler;
        detail::ServerResponseHandler    *fResponseDelegate;
      };

    }
  }
}
#endif  // _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_CONNECTION_HANDLER_HPP_
