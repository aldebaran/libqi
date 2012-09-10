/*
 ** transport-server.cpp
 ** Login : <hcuche@hcuche-de>
 ** Started on  Wed Jan 11 10:19:42 2012 Herve Cuche
 ** $Id$
 **
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2012 Herve Cuche
 */

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <qi/log.hpp>
#include <cerrno>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#ifdef _WIN32
#include <winsock2.h> // for socket
#include <WS2tcpip.h> // for socklen_t
#else
#include <arpa/inet.h>
#endif

#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>

#include <qimessaging/event_loop.hpp>

#include "src/event_loop_p.hpp"
#include "src/transport_server_p.hpp"
#include "src/transport_socket_libevent_p.hpp"
#include "src/transport_server_libevent_p.hpp"

namespace qi
{
  void accept_cb(struct evconnlistener *listener,
                 evutil_socket_t        fd,
                 struct sockaddr       *QI_UNUSED(a),
                 int                    QI_UNUSED(slen),
                 void                  *p)
  {
    TransportServerLibEventPrivate *_p = static_cast<TransportServerLibEventPrivate *>(p);
    _p->accept(fd, listener);
  }

  void accept_error_cb(struct evconnlistener *listener,
                       void                  *p)
  {
    TransportServerLibEventPrivate *_p = static_cast<TransportServerLibEventPrivate *>(p);
    _p->accept_error(listener);
  }

  void TransportServerLibEventPrivate::accept(evutil_socket_t        fd,
                                              struct evconnlistener *listener)
  {
    qi::TransportSocket *ts = new qi::TransportSocket();
    if (ts->_p)
      delete ts->_p;
    ts->_p = new qi::TransportSocketLibEvent(ts, fd, context);

    std::vector<TransportServerInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(mutexCallback);
      localCallbacks = tsi;
    }
    std::vector<TransportServerInterface *>::const_iterator it;
    for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
      (*it)->onTransportServerNewConnection(self, ts, 0);
  }

  void TransportServerLibEventPrivate::accept_error(struct evconnlistener *listener) {
    std::cout << "accept error" << std::endl;
    int err = errno;
    qiLogError("qimessaging.transportserver", "Got an error %d (%s) on the listener.", err, evutil_socket_error_to_string(err));
    std::vector<TransportServerInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(mutexCallback);
      localCallbacks = tsi;
    }
    std::vector<TransportServerInterface *>::const_iterator it;
    for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
      (*it)->onTransportServerError(self, err, 0);
  }

  void TransportServerLibEventPrivate::close() {
    if (_listener)
      evconnlistener_free(_listener);
    _listener = 0;
  }

  bool TransportServerLibEventPrivate::listen()
  {
    struct event_base     *base = context->_p->getEventBase();

    struct evutil_addrinfo *ai = NULL;
    int                     err;
    struct evutil_addrinfo  hint;
    char                    portbuf[10];

    bool findPort = listenUrl.port() == 0;
    unsigned short port = findPort ? qi::os::findAvailablePort(0) : listenUrl.port();

    // Convert the port to decimal.
    evutil_snprintf(portbuf, sizeof(portbuf), "%d", (int)port);

    // Build the hints to tell getaddrinfo how to act.
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC; // v4 or v6 is fine.
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP; // We want a TCP socket
    // Only return addresses we can use.
    hint.ai_flags = EVUTIL_AI_PASSIVE | EVUTIL_AI_ADDRCONFIG;

    // Look up the hostname.
    err = evutil_getaddrinfo(listenUrl.host().c_str(), portbuf, &hint, &ai);
    if (err < 0)
    {
      qiLogError("qimessaging.transportserver") << "Error while resolving '"
                                                << listenUrl.host()  << "': "
                                                << evutil_gai_strerror(err)
                                                << std::endl;
      if (ai)
        evutil_freeaddrinfo(ai);
      return false;
    }

    do
    {
      reinterpret_cast<struct sockaddr_in *>(ai->ai_addr)->sin_port = htons(port);
      _listener = evconnlistener_new_bind(base,
                                          accept_cb,
                                          this,
                                          LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE,
                                          -1,
                                          ai->ai_addr,
                                          ai->ai_addrlen);

      if (!findPort || (findPort && _listener))
      {
        break;
      }

      port = qi::os::findAvailablePort(port + 1);
    }
    while (port != 0);

    if (_listener)
    {
      evconnlistener_set_error_cb(_listener, accept_error_cb);
      std::stringstream out;
      out << port;

      listenUrl = "tcp://" + listenUrl.host() + ":" + out.str();
      qiLogVerbose("qimessaging.transportserver") << "Started TransportServer at "
                                                  << listenUrl.str();
    }
    else
    {
      qiLogError("qimessaging.transportserver") << "Could not start TransportServer at "
                                                << listenUrl.str();
    }

    if (ai)
      evutil_freeaddrinfo(ai);


    /* Set endpoints */
    if (listenUrl.host() != "0.0.0.0")
    {
      _endpoints.push_back(listenUrl);
    }
    else // need available ip addresses
    {
      std::string protocol;
      std::map<std::string, std::vector<std::string> > ifsMap = qi::os::hostIPAddrs();
      if (ifsMap.empty())
      {
        qiLogWarning("qimessaging.server.listen") << "Cannot get host addresses";
        return false;
      }
  #ifdef WIN32 // hostIPAddrs doesn't return loopback on windows
      ifsMap["Loopback"].push_back("127.0.0.1");
  #endif

      protocol = "tcp://";

      for (std::map<std::string, std::vector<std::string> >::iterator interfaceIt = ifsMap.begin();
           interfaceIt != ifsMap.end();
           ++interfaceIt)
      {
        for (std::vector<std::string>::iterator addressIt = (*interfaceIt).second.begin();
             addressIt != (*interfaceIt).second.end();
             ++addressIt)
        {
          std::stringstream ss;
          ss << protocol;
          ss << (*addressIt);
          ss << ":";
          ss << listenUrl.port();
          qiLogVerbose("qimessaging.server.listen") << "Adding endpoint : " << ss.str();
          _endpoints.push_back(ss.str());
         }
      }
    }

    return _listener != 0;
  }

  void server_deletor(TransportServerLibEventPrivate* ptr)
  {
    delete ptr;
  }

  void TransportServerLibEventPrivate::destroy()
  {
    close();
    context->asyncCall(200000, boost::bind(&server_deletor, this));
  }

  TransportServerLibEventPrivate::TransportServerLibEventPrivate(TransportServer* self,
                                                                 const qi::Url &url,
                                                                 EventLoop* ctx)
    : TransportServerPrivate(self, url, ctx)
  {
  }

  TransportServerLibEventPrivate::~TransportServerLibEventPrivate()
  {
  }
}
