#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_URL_HPP_
#define _QI_MESSAGING_URL_HPP_

#include <string>
#include <vector>

#include <qi/api.hpp>
#include <qi/uri.hpp>

namespace qi {
  class UrlPrivate;

  /** qi::Url is an address represented by a protocol, a host and a port.
   *  @warning The class isn't compliant with RFC 3986.
   *
   *  qi::Url can parse the following formats:
   *  <ul>
   *    <li>- protocol://host:port</li>
   *    <li>- protocol://host</li>
   *    <li>- host:port</li>
   *    <li>- host</li>
   *    <li>- protocol://:port</li>
   *    <li>- protocol://</li>
   *    <li>- :port</li>
   *    <li>- *empty string*</li>
   *  </ul>
   *
   *  @note This class is copyable.
   */
  class QI_API Url
  {
  public:
    /** Creates an empty url.
     */
    Url();

    /**
     *  @param url The url string, the port and the protocol will be extracted
     *  if they're present.
     */
    Url(const std::string &url);

    /**
     *  @param url The url string, the port and the protocol will be extracted
     *  if they're present.
     *  @param defaultPort The port that will be used if no port had been found
     *  in the url string.
     */
    Url(const std::string &url, unsigned short defaultPort);

    /**
     *  @param url The url string, the port and the protocol will be extracted
     *  if they're present.
     *  @param defaultProtocol The protocol that will be used if no protocol had
     *  been found in the url string.
     */
    Url(const std::string &url, const std::string &defaultProtocol);

    /**
     *  @param url The url string, the port and the protocol will be extracted
     *  if they're present.
     *  @param defaultProtocol The protocol that will be used if no protocol had
     *  been found in the url string.
     *  @param defaultPort The port that will be used if no port had been found
     *  in the url string.
     */
    Url(const std::string &url, const std::string &defaultProtocol, unsigned short defaultPort);

    /**
     * @cond
     */
    Url(const char *url);
    /**
     * @endcond
     */

    virtual ~Url();

    /**
     * @cond
     */
    Url(const qi::Url& url);
    Url& operator= (const Url& rhs);
    bool operator< (const Url& rhs) const;
    /**
     * @endcond
     */

    /**
     *  @return True if the protocol, host and port have been set.
     */
    bool isValid() const;

    /**
     *  @return The url string used by the Url class, the port and/or the
     *  protocol may have been appended if they had been given in the
     *  constructor.
     */
    const std::string& str() const;

    /**
     *  @return The protocol of the url or an empty string if no protocol was
     *  set.
     */
    const std::string& protocol() const;

    /// @return True if the protocol was set.
    bool hasProtocol() const;

    /// Sets the protocol to the given protocol identifier (tcp, tcps, udp, ...).
    void setProtocol(const std::string& protocol);

    /**
     *  @return The host part of the url or an empty string if no host part was
     *  found.
     */
    const std::string& host() const;

    /// @return True if the host was set.
    bool hasHost() const;

    /// Sets the host.
    void setHost(const std::string& host);

    /**
     *  @return The port of the url, 0 if no port were given.
     */
    unsigned short port() const;

    /// @return True if the port was set.
    bool hasPort() const;

    /// Sets the port. You can use 0.
    void setPort(unsigned short port);

  private:
    UrlPrivate* _p;
  };

  /** Compares the url strings.
   */
  QI_API bool operator==(const Url& lhs, const Url& rhs);
  /** Compares the url strings.
   */
  QI_API inline bool operator!=(const Url& lhs, const Url& rhs)
  { return !(lhs == rhs); }

  QI_API std::ostream& operator<<(std::ostream& out, const Url& url);

  using UrlVector = std::vector<Url>;

  /**
   * Use specified parts of the given URL to be set over the given base URL.
   * @param specification An URL that may be incomplete, which specified parts must be kept.
   * @param baseUrl A supposedly complete URL, which parts will be used to fill in the specified URL.
   */
  QI_API Url specifyUrl(const Url& specification, const Url& baseUrl);

  /// Returns an URL constructed from the given URI, according to the following behavior:
  ///   - the URL protocol is set to the URI scheme.
  ///   - the URL host is set to the URI authority host iff it has one.
  ///   - the URL port is set to the URI authority port iff it has one.
  /// Consequently, the resulting URL is not guaranteed to be valid (i.e. `isValid()` may return
  /// false). This is the case if the URI has no authority, in which case `hasHost()` and
  /// `hasPort()` both return false.
  /// Note that an URI with an empty host (such as 'ssh://:22') is valid, and will result in a URL
  /// with a host that is empty (i.e. `hasHost()` and `host().empty()` both return true).
  QI_API Url toUrl(const Uri& uri);

  inline ka::opt_t<Uri> toUri(const Url& url) noexcept
  {
    return uri(url.str());
  }
} // namespace qi

#endif  // _QIMESSAGING_URL_HPP_
