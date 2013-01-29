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

#include <qimessaging/transportserver.hpp>
#include <qimessaging/transportsocket.hpp>
#include "transportserverasio_p.hpp"
#include "tcptransportsocket.hpp"

namespace qi
{


  static TransportServerImplPrivate * newTSP(TransportServer* self,
    const qi::Url &url,
    qi::EventLoop* ctx) {
    if (url.protocol() == "tcp")
    {
      return new TransportServerAsioPrivate(self, url, ctx, false);
    }

    if (url.protocol() == "tcps")
    {
      return new TransportServerAsioPrivate(self, url, ctx, true);
    }

    qiLogError("TransportServer") << "Unrecognized protocol to create the TransportServer."
                                  << " TransportServer create with dummy implementation.";
    return 0;
  }

  TransportServer::TransportServer()
    : _p(new TransportServerPrivate)
  {
    _p->_impl = 0;
  }


  TransportServer::TransportServer(const qi::Url &url, qi::EventLoop* ctx)
    : _p(new TransportServerPrivate)
  {
    _p->_impl = newTSP(this, url, ctx);
  }

  TransportServer::~TransportServer()
  {
    close();
    delete _p;
    _p = 0;
  }

  bool TransportServer::listen(const qi::Url &url, qi::EventLoop* ctx)
  {
    if ((_p->_impl = newTSP(this, url, ctx)))
    {
      return _p->_impl->listen();
    }
    else
    {
      return false;
    }
  }

  bool TransportServer::listen()
  {
    if (_p->_impl)
    {
      return _p->_impl->listen();
    }
    else
    {
      return false;
    }
  }

  bool TransportServer::setIdentity(const std::string& key, const std::string& crt)
  {
    struct ::stat status;
    if (qi::os::stat(key.c_str(), &status) != 0)
    {
      qiLogError("TransportServer::setIdentity") << "stat:" << key << ": "
                                                 << strerror(errno);
      return false;
    }

    if (qi::os::stat(crt.c_str(), &status) != 0)
    {
      qiLogError("TransportServer::setIdentity") << "stat:" << crt << ": "
                                                 << strerror(errno);
      return false;
    }

    _p->_identityCertificate = crt;
    _p->_identityKey = key;

    return true;
  }

  qi::Url TransportServer::listenUrl() const {
    if (_p->_impl)
    {
      return _p->_impl->listenUrl;
    }
    else
    {
      return "";
    }
  }

  std::vector<qi::Url> TransportServer::endpoints() const
  {
    if (_p->_impl)
    {
      return _p->_impl->_endpoints;
    }
    else
    {
      return std::vector<qi::Url>();
    }
  }

  void TransportServer::close() {
    if (_p->_impl)
    {
      _p->_impl->destroy(); // this will delete _p->_impl
      _p->_impl = 0;
    }
  }

}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
