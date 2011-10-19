#ifndef QIMESSAGING_BIN_QIGATEWAY_GATEWAY_HPP
#define QIMESSAGING_BIN_QIGATEWAY_GATEWAY_HPP

#include <qi/macro.hpp>
#include <boost/unordered_map.hpp>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

namespace qi {
namespace gateway {

class Gateway
{
  QI_DISALLOW_COPY_AND_ASSIGN(Gateway);

public:
  Gateway(const char* host, unsigned short port);
  virtual ~Gateway();

  void run();

private:
  static void accept(evutil_socket_t fd, short events, void* arg);
  static void readcb(struct bufferevent* bev, void* context);
  static void eventcb(struct bufferevent* bev, short event, void* context);

  void init();
  void socket();
  void bind();
  void listen();
  void destroy();

private:
  const char* host_;
  unsigned short port_;

  evutil_socket_t sock_;
  struct event_base* base_;
  struct event* sockEvent_;
};

} // namespace gateway
} // namespace qi

#endif // QIMESSAGING_BIN_QIGATEWAY_GATEWAY_HPP
