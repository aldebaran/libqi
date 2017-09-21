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
    return status() == qi::MessageSocket::Status::Connected;
  }

  MessageSocketPtr makeMessageSocket(const std::string &protocol, qi::EventLoop *eventLoop)
  {
    return makeTcpMessageSocket(protocol, eventLoop);
  }
}

#if BOOST_COMP_MSVC
# pragma warning(pop)
#endif
