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
  TransportServer::TransportServer()
  {
  }

  TransportServer::~TransportServer()
  {
    close();
  }

  qi::Future<void> TransportServer::listen(const qi::Url &url, qi::EventLoop* ctx)
  {
    TransportServerImplPtr impl;
    if (url.protocol() == "tcp" || url.protocol() == "tcps")
    {
      impl = TransportServerAsioPrivate::make(this, ctx);
    }
    else
    {
      const char* s = "Unrecognized protocol to create the TransportServer.";
      qiLogError() << s;
      return qi::makeFutureError<void>(s);
    }
    {
      boost::mutex::scoped_lock l(_implMutex);
      _impl.push_back(impl);
    }
    return impl->listen(url);
  }

  bool TransportServer::setIdentity(const std::string& key, const std::string& crt)
  {
    struct ::stat status;
    if (qi::os::stat(key.c_str(), &status) != 0)
    {
      qiLogError() << "stat of \"" << key << "\": " << strerror(errno);
      return false;
    }

    if (qi::os::stat(crt.c_str(), &status) != 0)
    {
      qiLogError() << "stat of \"" << crt << "\": " << strerror(errno);
      return false;
    }

    _identityCertificate = crt;
    _identityKey = key;

    return true;
  }

  std::vector<qi::Url> TransportServer::endpoints() const
  {
    std::vector<qi::Url> r;
    boost::mutex::scoped_lock l(_implMutex);
    for (std::vector<TransportServerImplPtr>::const_iterator it = _impl.begin();
         it != _impl.end();
         it++)
    {
      boost::mutex::scoped_lock l((*it)->_endpointsMutex);
      r.insert(r.end(), (*it)->_endpoints.begin(), (*it)->_endpoints.end());
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
