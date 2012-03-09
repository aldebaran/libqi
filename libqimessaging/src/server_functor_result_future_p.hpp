/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  SERVER_FUNCTOR_RETURN_FUTURE_P_HPP_
# define SERVER_FUNCTOR_RETURN_FUTURE_P_HPP_

namespace qi {

  class ServerFunctorResultPromise : public FunctorResultPromiseBase {
  public:
    ServerFunctorResultPromise(TransportSocket *client, const qi::Message &req)
      : _client(client)
      , _retval(qi::Message::Create_WithoutBuffer)
    {
      _retval.buildReplyFrom(req);
    };

    inline virtual void setValue(qi::FunctorResult &result) {
      _retval.setBuffer(result.buffer());
      _client->send(_retval);
    }

  private:
    qi::Message          _retval;
    qi::TransportSocket *_client;
  };

}

#endif
