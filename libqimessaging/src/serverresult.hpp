#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVERRESULT_HPP_
#define _SRC_SERVERRESULT_HPP_

#include <boost/shared_ptr.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/metafunction.hpp>
#include <qimessaging/transportsocket.hpp>

namespace qi {

  class ServerResult: public FutureInterface<MetaFunctionResult>
  {
  public:
    ServerResult(TransportSocketPtr client, const qi::Message &req)
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
    qi::Message            _retval;
    qi::TransportSocketPtr _client;
  };

}

#endif  // _SRC_SERVERRESULT_HPP_
