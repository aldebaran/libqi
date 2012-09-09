/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  SERVER_FUNCTOR_RETURN_FUTURE_P_HPP_
# define SERVER_FUNCTOR_RETURN_FUTURE_P_HPP_

#include <boost/shared_ptr.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/metafunction.hpp>
#include <qimessaging/transport_socket.hpp>

namespace qi {

  class ServerResult: public FutureInterface<MetaFunctionResult>
  {
  public:
    ServerResult(TransportSocket *client, const qi::Message &req)
      : _client(client)
      {
        _retval.buildReplyFrom(req);
      };
    virtual void onFutureFinished(const MetaFunctionResult &value, void *data)
    {
      _retval.setBuffer(value.getBuffer());
      _client->send(_retval);
      delete this;
      }
    virtual void onFutureFailed(const std::string &error, void *data)
    {
      qiLogDebug("Future") << "Future finished in error " << error;
      qi::Buffer result;
      qi::ODataStream ods(result);
      ods << error;
      _retval.setBuffer(result);
      _client->send(_retval);
      delete this;
    }

  private:
    qi::Message          _retval;
    qi::TransportSocket *_client;
  };

}

#endif
