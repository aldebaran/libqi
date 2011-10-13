#include "server.hpp"

#include <cstring>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
  socket();
  bind();
  listen();

  int client = accept();
  write(client, "Hello world!\n", 13);
  close(client);

  destroy();
}

void Server::socket()
{
  if ((sock_ = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
    throw std::runtime_error("Could not get socket");
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

int Server::accept()
{
  int client;

  if ((client = ::accept(sock_, 0, 0)) == -1)
    throw std::runtime_error("Could not accept client");

  return client;
}

int Server::write(int sock, const void* data, unsigned len)
{
  return ::write(sock, data, len);
}

int Server::read(int sock, void* data, unsigned len)
{
  return ::read(sock, data, len);
}

void Server::close(int sock)
{
  if (::close(sock) == -1)
    throw std::runtime_error("Could not close socket");
}

void Server::destroy()
{
  this->close(sock_);
}

} // namespace gateway
} // namespace qi
