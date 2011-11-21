/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/transport/ssh/ssh_poll_client.hpp"
#include <qimessaging/exceptions.hpp>
#include <iostream>
#include <qi/os.hpp>
#include <qi/log.hpp>

namespace qi {
namespace transport {
namespace detail {

SSHPollClient::SSHPollClient(zmq::socket_t &socket)
  : _sshsocket(socket),
    _firstTime(true),
    _shuttingDown(false)
{
  _items[0].socket  = _zsocket;
  _items[0].fd      = 0;
  _items[0].events  = ZMQ_POLLIN;
  _items[0].revents = 0;
}

SSHPollClient::~SSHPollClient() {
  _shuttingDown = true;
  // hmmm, we must allow the current poll to terminate
  // before destroying the items array
  // This sleep is dodgy, but prevents a crash for now
  qi::os::sleep(1);
}

//return -1 on error, 0 for timeout, 1 otherwise
int SSHPollClient::pollRecv(long timeout) {
  int rc = 0;
  if (_shuttingDown || _items[0].socket == NULL) {
    return -1;
  }
  rc = zmq::poll(_items, 1, timeout);
  if (rc < 0)
    return -1;
  if ((rc == 0) || (!(_items[0].revents & ZMQ_POLLIN)))
    return 0;
  return 1;
}

void SSHPollClient::recv(std::string* msg)
{
  int rc = 0;
  // used by subscriber who wants polling, but no timeout
  do {
    rc = pollRecv(10 * 1000); // 10 ms
    if (rc == -1) {
      throw qi::transport::Exception("Terminated: No response received.");
    }
  } while(rc == 0);
  _zsocket.recv(msg);
}

void SSHPollClient::recv(zmq::message_t *msg, long usTimeout)
{
  if (_firstTime) {
    int rc = 0;
    long elapsed = 0;
    _firstTime = false;
    do {
      rc = pollRecv(10 * 1000); // 10 ms
      elapsed += 1;
    } while (rc <= 0 && elapsed < usTimeout);
    if (rc <= 0)
      throw qi::transport::Exception("No response received.");
  }
  else
  {
    if (pollRecv(usTimeout) <= 0)
      throw qi::transport::Exception("No response received.");
  }
  _zsocket.recv(msg);
}
} // namespace detail
} // namespace transport
} // namespace qi

