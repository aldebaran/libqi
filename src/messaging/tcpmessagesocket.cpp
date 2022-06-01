#include <string>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include "sock/networkasio.hpp"
#include "sock/option.hpp"

#if BOOST_OS_WINDOWS
# include <Winsock2.h> // needed by mstcpip.h
# include <Mstcpip.h> // for tcp_keepalive struct
#elif ANDROID
# include <linux/in.h> // for  IPPROTO_TCP
#endif

qiLogCategory(qi::sock::logCategory());

#if BOOST_OS_WINDOWS
static void setSocketNativeOptionsWindows(
  boost::asio::ip::tcp::socket::native_handle_type socketNativeHandle, int timeoutInSeconds)
{
  /* http://msdn.microsoft.com/en-us/library/windows/desktop/dd877220(v=vs.85).aspx
  On Windows Vista and later, the number of keep-alive probes (data retransmissions) is set to 10 and cannot be changed.
  On Windows Server 2003, Windows XP, and Windows 2000, the default setting for number of keep-alive probes is 5.
  The number of keep-alive probes is controllable through the TcpMaxDataRetransmissions and PPTPTcpMaxDataRetransmissions registry settings.
  The number of keep-alive probes is set to the larger of the two registry key values.
  If this number is 0, then keep-alive probes will not be sent.
  If this number is above 255, then it is adjusted to 255.
  */
  tcp_keepalive params;
  params.onoff = 1;
  params.keepalivetime = 30000; // entry is in milliseconds
  // set interval to target timeout divided by probe count
  params.keepaliveinterval = timeoutInSeconds * 1000 / 10;
  DWORD bytesReturned;
  if (WSAIoctl(socketNativeHandle, SIO_KEEPALIVE_VALS, &params, sizeof(params),
    0, 0, &bytesReturned, 0, 0) != 0)
  {
    qiLogWarning() << "Failed to set socket keepalive with code " << WSAGetLastError();
  }
}
#elif BOOST_OS_LINUX || ANDROID
  /// Precondition: timeoutInSeconds >= 0
static void setSocketNativeOptionsLinuxAndroid(
  boost::asio::ip::tcp::socket::native_handle_type socketNativeHandle, int timeoutInSeconds)
{
  int optval = 1;
  if (setsockopt(socketNativeHandle, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
  {
    qiLogWarning() << "Failed to set so_keepalive: " << strerror(errno);
    return;
  }
  // Keepalive mechanism is overriden by TCP data retransmission mechanism.
  // Retransmission cannot be parameterized on a per-socket level.

  /* SOL_TCP level options: unit is seconds for times
  TCP_KEEPCNT: overrides tcp_keepalive_probes 9
    mark dead when that many probes are lost
  TCP_KEEPIDLE: overrides tcp_keepalive_time 7200
    only enable keepalive if that delay ever occurr between data sent
  TCP_KEEPINTVL: overrides  tcp_keepalive_intvl 75
    interval between probes
  */
  optval = timeoutInSeconds / 10;
  static const unsigned int optlen = sizeof(optval);
  if (setsockopt(socketNativeHandle, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0)
    qiLogWarning() << "Failed to set TCP_KEEPINTVL: " << strerror(errno);
  optval = 30;
  if (setsockopt(socketNativeHandle, SOL_TCP, TCP_KEEPIDLE, &optval, optlen) < 0)
    qiLogWarning() << "Failed to set TCP_KEEPIDLE : " << strerror(errno);
  optval = 10;
  if (setsockopt(socketNativeHandle, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0)
    qiLogWarning() << "Failed to set TCP_KEEPCNT  : " << strerror(errno);
 qiLogDebug() << "enabling TCP_USER_TIMEOUT";
 // this is messy: defined in linux/tcp.h only after kernel 2.6.37,
 // which conflicts with netinet/tcp.h
 // We do not want to rely on compile-time flag to enable/disable this,
 // so just try it.
 static bool tcpUserTimeoutWarning = false;
 static const int QI_TCP_USER_TIMEOUT = 18;
 // TCP_USER_TIMEOUT: maximum time in ms data can remain unaknowledged
 optval = timeoutInSeconds * 1000;
 if (setsockopt(socketNativeHandle, SOL_TCP, QI_TCP_USER_TIMEOUT, &optval, optlen) < 0
   && !tcpUserTimeoutWarning)
 {
    qiLogVerbose() << "(Expected on old kernels) Failed to set TCP_USER_TIMEOUT  : " << strerror(errno);
    tcpUserTimeoutWarning = true;
 }
}
#elif BOOST_OS_MACOS
static void setSocketNativeOptionsMacOs(
  boost::asio::ip::tcp::socket::native_handle_type socketNativeHandle)
{
  int optval = 1;
  if (setsockopt(socketNativeHandle, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
  {
    qiLogWarning() << "Failed to set so_keepalive: " << strerror(errno);
    return;
  }
  // MacOs only have TCP_KEEPALIVE wich is linux's TCP_KEEPIDLE.
  // So the best we can do is lower that, which will reduce delay from
  // hours to tens of minutes.
  optval = 30;
  static unsigned int optlen = sizeof(optval);
  if (setsockopt(socketNativeHandle, IPPROTO_TCP, TCP_KEEPALIVE , &optval, optlen) < 0)
    qiLogWarning() << "Failed to set TCP_KEEPALIVE : " << strerror(errno);
}
#endif

namespace qi {

  /// Use the environment variable QI_TCP_PING_TIMEOUT, if set.
  /// Use the passed default timeout otherwise.
  /// Returns 0 if QI_TCP_PING_TIMEOUT is set but its value is invalid.
  boost::optional<Seconds> getTcpPingTimeout(Seconds defaultTimeout)
  {
    static const char* envTimeout = getenv("QI_TCP_PING_TIMEOUT");
    auto timeout = defaultTimeout;
    if (envTimeout)
      timeout = Seconds{strtol(envTimeout, 0, 0)};
    if (timeout.count()) return timeout;
    else                 return {};
  }

  std::uint32_t getMaxPayloadFromEnv(std::uint32_t defaultValue)
  {
    std::string l = os::getenv("QI_MAX_MESSAGE_PAYLOAD");
    return l.empty() ? defaultValue : boost::lexical_cast<std::uint32_t>(l);
  }

} // namespace qi

namespace qi { namespace sock {

  boost::optional<qi::int64_t> getSocketTimeWarnThresholdFromEnv()
  {
    static const auto thresholdEnvVariable = os::getenv("QIMESSAGING_SOCKET_DISPATCH_TIME_WARN_THRESHOLD");
    using Opt = boost::optional<qi::int64_t>;
    static const auto warnThreshold = thresholdEnvVariable.empty()
       ? Opt{}
       : Opt{strtol(thresholdEnvVariable.c_str(), 0, 0)};
    return warnThreshold;
  }

  void NetworkAsio::setSocketNativeOptions(
    boost::asio::ip::tcp::socket::native_handle_type socketNativeHandle, int timeoutInSeconds)
  {
  #if BOOST_OS_WINDOWS
    setSocketNativeOptionsWindows(socketNativeHandle, timeoutInSeconds);
  #elif BOOST_OS_LINUX || ANDROID
    setSocketNativeOptionsLinuxAndroid(socketNativeHandle, timeoutInSeconds);
  #elif BOOST_OS_MACOS
    setSocketNativeOptionsMacOs(socketNativeHandle);
  #else
  # error "Unsupported platform"
  #endif
  }

}} // namespace qi::sock
