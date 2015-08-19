/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

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
      qiLogError() << s;
      return qi::makeFutureError<void>(s);
    }
    TransportServerImplPtr implPtr(impl);
    {
      boost::mutex::scoped_lock l(_implMutex);
      _impl.push_back(implPtr);
    }
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

#ifdef _MSC_VER
# pragma warning( pop )
#endif
