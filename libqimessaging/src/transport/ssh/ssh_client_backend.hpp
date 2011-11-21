/*
 * Author(s):
 * - Nicolas Hureau <nhureau@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */


#ifndef _QI_TRANSPORT_SRC_SSH_SSH_CLIENT_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_SSH_SSH_CLIENT_BACKEND_HPP_

#include <qimessaging/transport/buffer.hpp>
#include "src/transport/client_backend.hpp"

#include <qissh/tunnel.hpp>
#include <zmq.hpp>

namespace qi {
namespace transport {
namespace detail {

class SSHClientBackend : public ClientBackend
{
public:
  /**
   * Creates a ZMQClientBackend for a server
   *
   * \param serverAddress The protocol-qualified address of the server
   *                      e.g. ipc:///tmp/naoqi/paf or tcp://127.0.0.1:5555
   * \param context A zmq context
   */
  //explicit SSHClientBackend(const std::string &serverAddress);
  SSHClientBackend(const std::string &serverAddress, zmq::context_t &context);

  /**
   * Sends data
   * \param tosend The data to send
   * \param result [in,out] The result
   */
  virtual void send(const std::string &tosend, std::string &result);


protected:
  /**
   * Connects to the server
   * this is a little bit tricky, zmq does asynchronous connect/bind. Inproc will fail if
   * bind is not ready
   */
  void connect();

protected:
  zmq::context_t& _zcontext; // Unused
  qi::ssh::Tunnel _ssh;
};

} // namespace detail
} // namespace transport
} // namespace qi

#endif  // _QI_TRANSPORT_SRC_ZMQ_ZMQ_CLIENT_BACKEND_HPP_
