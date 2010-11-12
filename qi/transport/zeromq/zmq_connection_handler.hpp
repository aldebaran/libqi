/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_
#define QI_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_

#include <qi/transport/common/i_runnable.hpp>
#include <qi/transport/common/i_server_response_handler.hpp>
#include <qi/transport/common/i_data_handler.hpp>
#include <string>

namespace qi {
  namespace transport {

    /// <summary>
    /// A connection handler created for each new incoming connection and
    /// pushed to the thread pool.
    /// </summary>
    class ZMQConnectionHandler : public IRunnable {
    public:

      /// <summary> Constructor. </summary>
      /// <param name="msg"> The message. </param>
      /// <param name="dataHandler"> The data handler. </param>
      /// <param name="serverResponseHander"> The server response hander </param>
      /// <param name="data"> [in,out] If non-null, the data. </param>
      ZMQConnectionHandler(
        const std::string &msg,
        IDataHandler* dataHandler,
        Detail::IServerResponseHandler* serverResponseHander,
        void *data);

      /// <summary> Finaliser. </summary>
      virtual ~ZMQConnectionHandler ();

      /// <summary> Runs this object. </summary>
      virtual void run ();

    private:
      void                             *fData;
      std::string                       fMsg;
      IDataHandler                     *fDataHandler;
      Detail::IServerResponseHandler   *fResponseDelegate;
    };

  }
}

#endif  // QI_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_
