/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <map>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/thread.h>

#include <qi/os.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <qi/log.hpp>

#include <boost/thread.hpp>

#ifdef _WIN32
 #include <winsock2.h> // for socket
 #include <WS2tcpip.h> // for socklen_t
#else
 #include <arpa/inet.h>
#endif

size_t socket_read(struct bufferevent *bev,
                   void               *data,
                   size_t              size)
{
  struct evbuffer *input = bufferevent_get_input(bev);
  return evbuffer_remove(input, data, size);;
}

bool socket_write(struct bufferevent *bev,
                  const void         *data,
                  size_t              size)
{
  evbuffer *evb = evbuffer_new();
  evbuffer_add(evb, data, size);
  return bufferevent_write_buffer(bev, evb) == 0;
}

void readcb(struct bufferevent *bev, void* context)
{
  std::cout << "readcb" << std::endl;
  char buffer[1024];
  size_t read = socket_read(bev, buffer, sizeof(buffer));
  socket_write(bev, buffer, read);
}

void writecb(struct bufferevent *bev, void* context)
{
  std::cout << "writecb" << std::endl;
}

void eventcb(struct bufferevent *bev, short events, void *context)
{
  std::cout << "eventcb" << std::endl;

  if (events & BEV_EVENT_CONNECTED)
  {
    qiLogInfo("qimessaging.TransportSocket") << "BEV_EVENT_CONNECTED" << std::endl;
  }
  else if (events & BEV_EVENT_EOF)
  {
    qiLogInfo("qimessaging.TransportSocket") << "BEV_EVENT_EOF" << std::endl;
  }
  else if (events & BEV_EVENT_ERROR)
  {
    bufferevent_free(bev);
    qiLogError("qimessaging.TransportSocket") << "BEV_EVENT_ERROR" << std::endl;
  }
  else if (events & BEV_EVENT_TIMEOUT)
  {
    qiLogError("qimessaging.TransportSocket") << "BEV_EVENT_TIMEOUT" << std::endl;
  }
}

void accept(int fd, struct event_base *base)
{
  std::cout << "accept" << std::endl;
  bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, readcb, writecb, eventcb, 0);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void accept_cb(struct evconnlistener *listener,
               evutil_socket_t        fd,
               struct sockaddr       *a,
               int                    slen,
               void                  *p)
{
  std::cout << "accept_cb" << std::endl;
  struct event_base *base = evconnlistener_get_base(listener);
  accept(fd, base);
}

int start_server(const qi::Url &url, struct event_base *base)
{
  struct evconnlistener *listener;
  static struct sockaddr_storage listen_on_addr;
  memset(&listen_on_addr, 0, sizeof(listen_on_addr));
  int socklen = sizeof(listen_on_addr);
  struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&listen_on_addr);

  socklen = sizeof(struct sockaddr_in);
  sin->sin_port = htons(url.port());
  sin->sin_family = AF_INET;
  if ((sin->sin_addr.s_addr = inet_addr(url.host().c_str())) == INADDR_NONE)
  {
    qiLogError("qimessaging.transportserver") << "Provided IP is not valid" << std::endl;
    return false;
  }

  listener = evconnlistener_new_bind(base,
                                     accept_cb,
                                     0,
                                     LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE,
                                     -1,
                                     (struct sockaddr*)&listen_on_addr,
                                     socklen);

  return 0;
}

void *network_thread(void *arg)
{
  event_base_dispatch(reinterpret_cast<struct event_base *>(arg));
  return 0;
}

static void errorcb(struct bufferevent *bev,
                    short error,
                    void *context)
{
  std::cout << "errorcb" << std::endl;
}

void *network_thread2(void *arg)
{
  struct event_base *base = reinterpret_cast<struct event_base *>(arg);

  /* hack to keep the loop running */
  struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
  bufferevent_setcb(bev, 0, 0, errorcb, 0);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  event_base_dispatch(reinterpret_cast<struct event_base *>(arg));
  std::cout << "exit" << std::endl;
  return 0;
}

int main()
{
  std::cout << "Starting..." << std::endl;

  #ifdef _WIN32
  // libevent does not call WSAStartup
  WSADATA WSAData;
  // TODO: handle return code
  ::WSAStartup(MAKEWORD(1, 0), &WSAData);
  #endif

  #ifdef EVTHREAD_USE_WINDOWS_THREADS_IMPLEMENTED
  evthread_use_windows_threads();
  #endif
  #ifdef EVTHREAD_USE_PTHREADS_IMPLEMENTED
  evthread_use_pthreads();
  #endif

  struct event_base *server_base = event_base_new();
  qi::Url server_url("tcp://127.0.0.1:9571");
  start_server(server_url, server_base);
  boost::thread server_thread(network_thread, server_base);

  struct event_base *client_base = event_base_new();
  qi::Url client_url("tcp://127.0.0.1:5555");
  boost::thread client_thread(network_thread2, client_base);

  // test send
  qi::os::sleep(1); // wait for the thread to start
  struct bufferevent *bev = bufferevent_socket_new(client_base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
  bufferevent_setcb(bev, readcb, writecb, eventcb, 0);
  bufferevent_socket_connect_hostname(bev, 0, AF_INET,
                                      client_url.host().c_str(), client_url.port());
  socket_write(bev, "CHICHE", 6);

  client_thread.join();
  server_thread.join();

  return 0;
}
