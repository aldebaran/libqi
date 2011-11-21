#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_SSH_SSH_POLL_CLIENT_HPP_
#define _QI_TRANSPORT_SRC_SSH_SSH_POLL_CLIENT_HPP_

# include <qimessaging/transport/buffer.hpp>
# include "src/transport/client_backend.hpp"
# include <zmq.hpp>

namespace qi {
namespace transport {
namespace detail {

/// <summary> Simple zmq helper, that will handle receive timeout nicely
/// </summary>
class SSHPollClient {
public:
  SSHPollClient(zmq::socket_t &socket);

  virtual ~SSHPollClient();

  /// <summary> zmq::recv with timeout </summary>
  /// <param name="msg"> a pointer to a zmq message </param>
  /// <param name="msg"> a microsecond timeout </param>
  void recv(zmq::message_t *msg, long usTimeout);

  /// <summary> zmq::recv with no timeout </summary>
  /// <param name="msg"> a pointer to a zmq message </param>
  void recv(zmq::message_t *msg);

protected:
  int pollRecv(long usTimeout);

protected:
  zmq::socket_t   &_zsocket;
  zmq_pollitem_t   _items[1];
  bool             _firstTime;
  bool             _shuttingDown;
};

} // namespace detail
} // namespace transport
} // namespace qi

#endif  // _QI_TRANSPORT_SRC_SSH_SSH_POLL_CLIENT_HPP_
