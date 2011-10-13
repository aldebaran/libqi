#ifndef QIMESSAGING_BIN_QIGATEWAY_SERVER_HPP
#define QIMESSAGING_BIN_QIGATEWAY_SERVER_HPP

#include <qi/macro.hpp>

namespace qi {
namespace gateway {

class Server
{
  QI_DISALLOW_COPY_AND_ASSIGN(Server);

public:
  Server(const char* host, unsigned short port);
  virtual ~Server();

  void run();

private:
  void socket();
  void bind();
  void listen();
  int accept();
  int write(int sock, const void* data, unsigned len);
  int read(int sock, void* data, unsigned len);
  void close(int sock);
  void destroy();

  const char* host_;
  unsigned short port_;
  int sock_;
};

} // namespace gateway
} // namespace qi

#endif // QIMESSAGING_BIN_QIGATEWAY_SERVER_HPP
