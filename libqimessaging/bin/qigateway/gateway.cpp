#include "gateway.hpp"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

#include <qi/log.hpp>

#define MAX_LINE 16384
#define BUFFER_SIZE 4096

namespace qi {
namespace gateway {

Gateway::Gateway(const char* host, unsigned short port)
  : host_(host),
    port_(port)
{}

Gateway::~Gateway()
{}

void Gateway::run()
{
  setvbuf(stdout, NULL, _IONBF, 0);
  init();
  socket();
  bind();
  listen();

  qiLogInfo("qigateway", "Gateway: Listening on %s:%i", host_, port_);

  sockEvent_ = event_new(base_, sock_, EV_READ | EV_PERSIST,
      Gateway::accept, (void*)NULL);

  event_add(sockEvent_, NULL);
  event_base_dispatch(base_);
}

void Gateway::launch(const char* host, unsigned short port)
{
  try
  {
    Gateway gateway(host, port);
    gateway.run();
  }
  catch (const std::exception& e)
  {
    qiLogError("qigateway", "Gateway: %s", e.what());
  }
}

void Gateway::accept(evutil_socket_t sock, short events, void* arg)
{
  struct sockaddr addr;
  socklen_t slen = sizeof (addr);

  int client = ::accept(sock, &addr, &slen);

  if (client < 0)
    qiLogError("Gateway: Could not accept client");
  else if (client > FD_SETSIZE)
    ::close(client);
  else
  {
    /* Init socket, callbacks */
    evutil_make_socket_nonblocking(client);
    struct bufferevent* bev = bufferevent_socket_new(NULL/*globalInfo->base*/,
        client, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, NULL, eventcb, NULL);
    bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /* Logging */
    char host[128] = {0};
    char time[128] = {0};
    time_t t = ::time(NULL);
    strftime(time, 127, "%T", localtime(&t));
    if (getnameinfo(&addr, slen, host, 127, NULL, 0, 0) == 0)
      qiLogInfo("qigateway", "Gateway: %s - Accepted connection from %s",
          time, host);
    else
      qiLogInfo("qigateway", "Gateway: %s - Accepted connection", time);
  }
}

void Gateway::readcb(struct bufferevent* bev, void* context)
{
  (void) context;
  char buf[BUFFER_SIZE];
  size_t len;

  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);

  while ((len = evbuffer_remove(input, buf, BUFFER_SIZE)) > 0)
    evbuffer_add(output, buf, len);
}

void Gateway::eventcb(struct bufferevent* bev,
    short event, void* context)
{
  (void) bev;
  (void) event;
  (void) context;
}

void Gateway::init()
{
  qiLogDebug("qigateway", "Gateway: Init");
  if (!(base_ = event_base_new()))
    std::runtime_error("Could not init libevent");
}

void Gateway::socket()
{
  if ((sock_ = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
    throw std::runtime_error("Could not get socket");
  evutil_make_socket_nonblocking(sock_);
}

void Gateway::bind()
{
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);

  if ((addr.sin_addr.s_addr = inet_addr(host_)) == INADDR_NONE)
    throw std::runtime_error("Provided IP is not valid");

  if (::bind(sock_, (struct sockaddr*)&addr, sizeof (addr)) == -1)
    throw std::runtime_error("Could not bind socket");
}

void Gateway::listen()
{
  if (::listen(sock_, SOMAXCONN) == -1)
    throw std::runtime_error("Could not listen on socket");
}

void Gateway::destroy()
{
  ::close(sock_);
}

} // namespace gateway
} // namespace qi
