/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  SERVER_FUNCTOR_RETURN_FUTURE_P_HPP_
# define SERVER_FUNCTOR_RETURN_FUTURE_P_HPP_

#include <boost/shared_ptr.hpp>
#include <qimessaging/functor.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>

namespace qi {

  namespace detail {
    class ServerFunctorResultPromise : public FunctorResultBase {
    public:
      ServerFunctorResultPromise(TransportSocket *client, const qi::Message &req)
        : _client(client)
      {
        _retval.buildReplyFrom(req);
      };

      inline virtual void setValue(const qi::Buffer &result) {
        _retval.setBuffer(result);
        _client->send(_retval);
      }

      inline virtual void setError(const qi::Buffer &error) {
        _retval.setType(qi::Message::Type_Error);
        _retval.setBuffer(error);
        _client->send(_retval);
      }

    private:
      qi::Message          _retval;
      qi::TransportSocket *_client;
    };
  }

  class ServerFunctorResult : public FunctorResult {
  public:
    ServerFunctorResult(TransportSocket *client, const qi::Message &req) {
      boost::shared_ptr<detail::ServerFunctorResultPromise> p(new detail::ServerFunctorResultPromise(client, req));
      _p = p;
    };
  };

}

#endif
