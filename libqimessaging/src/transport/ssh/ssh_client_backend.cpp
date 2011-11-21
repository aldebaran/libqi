/*
*  Author(s):
*  - Nicolas Hureau <nhureau@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/transport/ssh/ssh_client_backend.hpp"
#include "src/messaging/network/ip_address.hpp"
#include <qimessaging/exceptions.hpp>
#include <iostream>
#include <qi/log.hpp>
#include <qi/os.hpp>

namespace qi {
namespace transport {
namespace detail {

//SSHClientBackend::SSHClientBackend(const std::string &serverAddress)
SSHClientBackend::SSHClientBackend(const std::string &serverAddress, zmq::context_t& ctx)
  : ClientBackend(serverAddress),
    _zcontext(ctx)
{
  connect();
}

void SSHClientBackend::connect()
{
  qi::detail::Address address;
  std::vector<std::string> parts;

  if (!qi::detail::splitAddress(_serverAddress, address, parts))
    throw qi::transport::Exception("SSHClientBackend has a wrong address.");

  for (int i = 3; i > 0; ++i)
  {
    try
    {
      _ssh.connect(address.address.c_str(), address.port);
      _ssh.authPassword("test", "test");
      _ssh.setTunnel("127.0.0.1", 12789, "127.0.0.1", 9876);

      return;
    }
    catch(const std::exception& e)
    {
      qiLogDebug("qimessaging") << "SSHClientBackend failed to create client for address \""
          << _serverAddress << "\" Reason: " << e.what() << std::endl;
      qiLogDebug("qimessaging") << "retrying.." << std::endl;
    }

    qi::os::msleep(100);
  }
  throw qi::transport::Exception("SSHClientBackend can't connect.");
}


void SSHClientBackend::send(const std::string &tosend, std::string &result)
{
  //Message msg;

  /* TODO: send message */
  _ssh.writeRemote(tosend.c_str(), tosend.size());

  // we leave the possibility to timeout, pollRecv will throw and avoid the call to recv
  //_poller.recv(&msg, 60 * 1000 * 1000);

  // TODO optimize this
  // boost could serialize from msg.data() and size,
  // without making a string
  //result.assign((char *)msg.data(), msg.size());
}

} // namespace detail
} // namespace transport
} // namespace qi
