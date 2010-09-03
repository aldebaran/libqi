/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_
#define AL_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_

#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/transport/common/runnable.hpp>
#include <alcommon-ng/transport/common/server_response_delegate.hpp>
#include <alcommon-ng/transport/common/datahandler.hpp>
#include <string>

namespace AL {
  namespace Transport {

    /// <summary>
    /// A connection handler created for each new incoming connection and
    /// pushed to the thread pool.
    /// </summary>
    class ZMQConnectionHandler : public Runnable {
    public:

      /// <summary> Constructor. </summary>
      /// <param name="msg"> The message. </param>
      /// <param name="sdelegate"> [in,out] If non-null, the sdelegate. </param>
      /// <param name="rdelegate"> [in,out] If non-null, the rdelegate. </param>
      /// <param name="data"> [in,out] If non-null, the data. </param>
      ZMQConnectionHandler(
        const std::string &msg,
        DataHandler *sdelegate,
        internal::ServerResponseDelegate* rdelegate,
        void *data);

      /// <summary> Finaliser. </summary>
      virtual ~ZMQConnectionHandler ();

      /// <summary> Runs this object. </summary>
      virtual void run ();

    private:
      void                             *_data;
      std::string                       _msg;
      DataHandler                      *_dataHandler;
      internal::ServerResponseDelegate *_responsedelegate;
    };

  }
}

#endif  // AL_TRANSPORT_ZMQ_CONNECTION_HANDLER_HPP_
