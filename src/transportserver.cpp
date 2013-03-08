/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <iostream>
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
#include "transportsocket.hpp"
#include "transportserverasio_p.hpp"
#include "tcptransportsocket.hpp"


qiLogCategory("qimessaging.transportserver");

namespace qi
{
  TransportServer::TransportServer()
    : _p(new TransportServerPrivate)
  {
  }

  TransportServer::~TransportServer()
  {
    close();
    delete _p;
    _p = 0;
  }

  qi::Future<void> TransportServer::listen(const qi::Url &url, qi::EventLoop* ctx)
  {
    TransportServerImpl* impl = 0;

    if (url.protocol() == "tcp")
    {
      impl = new TransportServerAsioPrivate(this, ctx);
    }
#ifdef WITH_SSL
    else if (url.protocol() == "tcps")
    {
      impl = new TransportServerAsioPrivate(this, ctx);
    }
#endif
    else
    {
      const char* s = "Unrecognized protocol to create the TransportServer.";
      qiLogError("TransportServer") << s;
      return qi::makeFutureError<void>(s);
    }

    _p->_impl.push_back(TransportServerImplPtr(impl));
    return impl->listen(url);
  }

  bool TransportServer::setIdentity(const std::string& key, const std::string& crt)
  {
    struct ::stat status;
    if (qi::os::stat(key.c_str(), &status) != 0)
    {
      qiLogError() << "stat:" << key << ": "
                                                 << strerror(errno);
      return false;
    }

    if (qi::os::stat(crt.c_str(), &status) != 0)
    {
      qiLogError() << "stat:" << crt << ": "
                                                 << strerror(errno);
      return false;
    }

    _p->_identityCertificate = crt;
    _p->_identityKey = key;

    return true;
  }

  std::vector<qi::Url> TransportServer::endpoints() const
  {
    std::vector<qi::Url> r;

    for (std::vector<TransportServerImplPtr>::const_iterator it = _p->_impl.begin();
         it != _p->_impl.end();
         it++)
    {
      boost::mutex::scoped_lock((*it)->_endpointsMutex);
      r.insert(r.end(), (*it)->_endpoints.begin(), (*it)->_endpoints.end());
    }

    return r;
  }

  void TransportServer::close() {
    for (std::vector<TransportServerImplPtr>::const_iterator it = _p->_impl.begin();
         it != _p->_impl.end();
         it++)
    {
      (*it)->close(); // this will delete _p->_impl
    }

    _p->_impl.clear();
  }

}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
