/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4355, )

#include <string>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/messaging/ssl/ssl.hpp>
#include <cerrno>

#ifdef _WIN32
#include <winsock2.h> // for socket
#include <WS2tcpip.h> // for socklen_t
#else
#include <arpa/inet.h>
#endif

#include "transportserver.hpp"
#include "messagesocket.hpp"
#include "transportserverasio_p.hpp"

qiLogCategory("qimessaging.transportserver");

namespace qi
{
  TransportServer::TransportServer(ssl::ServerConfig sslConfig)
    : _sslConfig(std::move(sslConfig))
  {
  }

  TransportServer::~TransportServer()
  {
    close();
  }

  qi::Future<void> TransportServer::listen(const qi::Url &url, qi::EventLoop* ctx)
  {
    const auto scheme = tcpScheme(url);
    if (!scheme)
    {
      const std::string s = "cannot create TCP server from unrecognized scheme in URL '" + url.str() + "'";
      qiLogError() << s;
      return qi::makeFutureError<void>(s);
    }

    auto impl = TransportServerAsioPrivate::make(this, ctx);
    {
      boost::mutex::scoped_lock l(_implMutex);
      _impl.push_back(impl);
    }
    return impl->listen(url);
  }

  std::vector<qi::Uri> TransportServer::endpoints() const
  {
    std::vector<qi::Uri> r;
    boost::mutex::scoped_lock l(_implMutex);
    for (const auto& impl : _impl)
    {
      boost::mutex::scoped_lock l(impl->_endpointsMutex);
      r.insert(r.end(), impl->_endpoints.begin(), impl->_endpoints.end());
    }

    return r;
  }

  void TransportServer::close() {
    boost::mutex::scoped_lock l(_implMutex);
    for (std::vector<TransportServerImplPtr>::const_iterator it = _impl.begin();
         it != _impl.end();
         it++)
    {
      (*it)->close(); // this will delete _impl
    }

    _impl.clear();
  }

}

KA_WARNING_POP()
