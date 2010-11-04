/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_
#define AL_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_

//#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/transport/common/i_runnable.hpp>
#include <alcommon-ng/transport/common/i_server_response_handler.hpp>
#include <alcommon-ng/transport/common/i_data_handler.hpp>
#include <string>

namespace AL {
  namespace Transport {

    /// <summary>
    /// A connection handler created for each new incoming connection and
    /// pushed to the thread pool.
    /// </summary>
    class ZMQConnectionHandler : public IRunnable {
    public:

      /// <summary> Constructor. </summary>
      /// <param name="msg"> The message. </param>
      /// <param name="sdelegate"> [in,out] If non-null, the sdelegate. </param>
      /// <param name="rdelegate"> [in,out] If non-null, the rdelegate. </param>
      /// <param name="data"> [in,out] If non-null, the data. </param>
      ZMQConnectionHandler(
        const std::string &msg,
        IDataHandler* dataHandler,
        Detail::IServerResponseHandler* rdelegate,
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

#endif  // AL_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_
