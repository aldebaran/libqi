#include <qi/log.hpp>
#include <qi/messaging/sock/option.hpp>
#include "messagesocket.hpp"
#include "tcpmessagesocket.hpp"

// Disable "'this': used in base member initializer list"
#if BOOST_COMP_MSVC
# pragma warning(push)
# pragma warning(disable: 4355)
#endif


qiLogCategory(qi::sock::logCategory());

namespace qi
{
  MessageSocket::~MessageSocket()
  {
    qiLogDebug() << "Destroying transport socket";
    _signalsStrand.join();
  }

  bool MessageSocket::isConnected() const
  {
    return _status == qi::MessageSocket::Status::Connected;
  }

  MessageSocketPtr makeMessageSocket(const std::string &protocol, qi::EventLoop *eventLoop)
  {
    MessageSocketPtr ret;

    if (protocol == "tcp")
    {
      return TcpMessageSocketPtr(new TcpMessageSocket<>(*asIoServicePtr(eventLoop), false));
    }
    if (protocol == "tcps")
    {
      return TcpMessageSocketPtr(new TcpMessageSocket<>(*asIoServicePtr(eventLoop), true));
    }
    qiLogError() << "Unrecognized protocol to create the TransportSocket: " << protocol;
    return ret;
  }
}

#if BOOST_COMP_MSVC
# pragma warning(pop)
#endif
