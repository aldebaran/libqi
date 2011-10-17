#include "server.hpp"

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

Server::Server(const char* host, unsigned short port)
  : host_(host),
    port_(port)
{}

Server::~Server()
{}

void Server::run()
{
  try
  {
    qiLogInfo("qigateway", "Launching QiMessaging Gateway");

    setvbuf(stdout, NULL, _IONBF, 0);
    init();
    socket();
    bind();
    listen();

    qiLogInfo("qigateway", "Listening on %s:%i", host_, port_);

    sockEvent_ = event_new(base_, sock_, EV_READ | EV_PERSIST,
        Server::accept, (void*)base_);

    event_add(sockEvent_, NULL);
    event_base_dispatch(base_);
  }
  catch (const std::exception& e)
  {
    qiLogFatal("qigateway", "%s", e.what());
    exit(1);
  }
}

void Server::accept(evutil_socket_t sock, short events, void* arg)
{
  struct event_base* base = (struct event_base*)arg;
  struct sockaddr addr;
  socklen_t slen = sizeof (addr);

  int client = ::accept(sock, &addr, &slen);

  if (client < 0)
    qiLogError("Could not accept client");
  else if (client > FD_SETSIZE)
    ::close(client);
  else
  {
    evutil_make_socket_nonblocking(client);
    struct bufferevent* bev = bufferevent_socket_new(base,
        client, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, NULL, errorcb, NULL);
    bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    char host[128];
    time_t time = ::time(NULL);
    if (getnameinfo(&addr, slen, host, 128, NULL, 0, 0) == 0)
      qiLogInfo("qigateway", "%i - Accepted connection from %s", time, host);
    else
      qiLogInfo("qigateway", "%i - Accepted connection", time);
  }
}

void Server::readcb(struct bufferevent* bev, void* context)
{
  char buf[BUFFER_SIZE];
  size_t len;

  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);

  while ((len = evbuffer_remove(input, buf, BUFFER_SIZE)) > 0)
    evbuffer_add(output, buf, len);
}

void Server::errorcb(struct bufferevent* bev,
    short error, void* context)
{
}

void Server::init()
{
  qiLogDebug("qigateway", "Init");
  if (!(base_ = event_base_new()))
    std::runtime_error("Could not init libevent");
}

void Server::socket()
{
  if ((sock_ = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
    throw std::runtime_error("Could not get socket");
  evutil_make_socket_nonblocking(sock_);
}

void Server::bind()
{
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);

  if ((addr.sin_addr.s_addr = inet_addr(host_)) == INADDR_NONE)
    throw std::runtime_error("Provided IP is not valid");

  if (::bind(sock_, (struct sockaddr*)&addr, sizeof (addr)) == -1)
    throw std::runtime_error("Could not bind socket");
}

void Server::listen()
{
  if (::listen(sock_, SOMAXCONN) == -1)
    throw std::runtime_error("Could not listen on socket");
}

void Server::destroy()
{
  ::close(sock_);
}

} // namespace gateway
} // namespace qi
