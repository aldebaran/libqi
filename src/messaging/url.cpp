/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/url.hpp>
#include <sstream>
#include <qi/type/typeinterface.hpp>

qiLogCategory("qi.url");

namespace qi {

  class UrlPrivate {
  public:
    UrlPrivate();
    UrlPrivate(const UrlPrivate* url_p);
    UrlPrivate(const std::string& url);
    UrlPrivate(const std::string& url, unsigned short defaultPort);
    UrlPrivate(const std::string& url, const std::string& defaultProtocol);
    UrlPrivate(const std::string& url, const std::string& defaultProtocol, unsigned short defaultPort);
    UrlPrivate(const char* url);

    void updateUrl();
    const std::string& str() const;
    bool isValid() const;

    std::string    url;
    std::string    protocol;
    std::string    host;
    unsigned short port;

    enum UrlComponents {
      SCHEME = 2,
      HOST   = 4,
      PORT   = 1,
      None   = 0,
    };

    int components;

  private:
    // Explodes the url in different part and fill the fields of the class.
    //@return a bitmask of UrlComponents with the elements that were found
    int split_me(const std::string& url);
  };

  Url::Url()
    : _p(new UrlPrivate())
  {
  }

  Url::Url(const std::string &url)
    : _p(new UrlPrivate(url))
  {
  }

  Url::Url(const std::string &url, unsigned short defaultPort)
    : _p(new UrlPrivate(url, defaultPort))
  {
  }

  Url::Url(const std::string &url, const std::string &defaultProtocol)
    : _p(new UrlPrivate(url, defaultProtocol))
  {
  }

  Url::Url(const std::string &url, const std::string &defaultProtocol, unsigned short defaultPort)
    : _p(new UrlPrivate(url, defaultProtocol, defaultPort))
  {
  }

  Url::Url(const char *url)
    : _p(new UrlPrivate(url))
  {
  }

  Url::~Url()
  {
    delete _p;
  }

  Url::Url(const qi::Url& url)
    : _p(new UrlPrivate(url._p))
  {
  }

  Url& Url::operator= (const Url &rhs) {
    *_p = *rhs._p;
    return *this;
  }

  bool Url::operator< (const Url &rhs) const {
    return (this->str() < rhs.str());
  }

  bool Url::isValid() const {
    return _p->isValid();
  }

  const std::string& Url::str() const {
    return _p->url;
  }

  const std::string& Url::protocol() const {
    return _p->protocol;
  }

  bool Url::hasProtocol() const {
    return (_p->components & UrlPrivate::SCHEME) != 0;
  }

  void Url::setProtocol(const std::string &protocol) {
    _p->protocol = protocol;
    _p->components |= UrlPrivate::SCHEME;
    _p->updateUrl();
  }

  const std::string& Url::host() const {
    return _p->host;
  }

  bool Url::hasHost() const {
    return (_p->components & UrlPrivate::HOST) != 0;
  }

  void Url::setHost(const std::string &host) {
    _p->host = host;
    _p->components |= UrlPrivate::HOST;
    _p->updateUrl();
  }

  unsigned short Url::port() const {
    return _p->port;
  }

  bool Url::hasPort() const {
    return _p->components & UrlPrivate::PORT;
  }

  void Url::setPort(unsigned short port) {
    _p->port = port;
    _p->components |= UrlPrivate::PORT;
    _p->updateUrl();
  }

  Url specifyUrl(const Url& specification, const Url& baseUrl)
  {
    Url url;

    if(specification.hasProtocol())
      url.setProtocol(specification.protocol());
    else if(baseUrl.hasProtocol())
      url.setProtocol(baseUrl.protocol());

    if(specification.hasHost())
      url.setHost(specification.host());
    else if(baseUrl.hasHost())
      url.setHost(baseUrl.host());

    if(specification.hasPort())
      url.setPort(specification.port());
    else if(baseUrl.hasPort())
      url.setPort(baseUrl.port());

    return url;
  }

  UrlPrivate::UrlPrivate()
    : url()
    , protocol()
    , host()
    , port(0)
    , components(0)
  {
  }

  UrlPrivate::UrlPrivate(const UrlPrivate* url_p)
    : url(url_p->url)
    , protocol(url_p->protocol)
    , host(url_p->host)
    , port(url_p->port)
    , components(url_p->components)
  {
  }

  UrlPrivate::UrlPrivate(const char* url)
    : url(url)
    , protocol()
    , host()
    , port(0)
    , components(0)
  {
    split_me(url);
    updateUrl();
  }

  UrlPrivate::UrlPrivate(const std::string& url)
    : url(url)
    , protocol()
    , host()
    , port(0)
    , components(0)
  {
    split_me(url);
    updateUrl();
  }

  UrlPrivate::UrlPrivate(const std::string& url, unsigned short defaultPort)
    : url(url)
    , protocol()
    , host()
    , port(defaultPort)
    , components(0)
  {
    if (!(split_me(url) & PORT)) {
      port = defaultPort;
      components |= PORT;
    }
    updateUrl();
  }

  UrlPrivate::UrlPrivate(const std::string& url, const std::string& defaultProtocol)
    : url(url)
    , protocol()
    , host()
    , port(0)
    , components(0)
  {
    if (!(split_me(url) & SCHEME)) {
      protocol = defaultProtocol;
      components |= SCHEME;
    }
    updateUrl();
  }

  UrlPrivate::UrlPrivate(const std::string& url, const std::string& defaultProtocol, unsigned short defaultPort)
    : url(url)
    , protocol()
    , host()
    , port(0)
    , components(0)
  {
    int result = split_me(url);
    if (!(result & SCHEME)) {
      protocol = defaultProtocol;
      components |= SCHEME;
    }
    if (!(result & PORT)) {
      port = defaultPort;
      components |= PORT;
    }
    updateUrl();
  }

  void UrlPrivate::updateUrl()
  {
    url = std::string();
    if(components & SCHEME)
      url += protocol + "://";
    if(components & HOST)
      url += host;
    if(components & PORT)
      url += std::string(":") + boost::lexical_cast<std::string>(port);
  }

  bool UrlPrivate::isValid() const {
    return components == (SCHEME | HOST | PORT);
  }

  int UrlPrivate::split_me(const std::string& url) {
    /******
     * Not compliant with RFC 3986
     * This function can parse:
     * scheme://host:port and return SCHEME | HOST | PORT
     * scheme://host and return SCHEME | HOST
     * host:port and return HOST | PORT
     * host and return HOST
     * scheme://:port return SCHEME | PORT
     * scheme:// return SCHEME
     * :port return PORT
     *  return 0
     */
    std::string _url = url;
    std::string _scheme = "";
    std::string _host = "";
    unsigned short _port = 0;
    components = 0;

    size_t place = 0;
    place = _url.find("://");
    if (place != std::string::npos) {
      _scheme = url.substr(0, place);
      components |= SCHEME;
      place += 3;
    } else
      place = 0;

    _url = _url.substr(place);
    place = _url.find(":");
    _host = _url.substr(0, place);
    if (!_host.empty())
      components |= HOST;

    if (place != std::string::npos) {
      const auto strPort = _url.substr(place+1);
      // std::stol raises an exception if the conversion fails, instead prefer strtol that sets errno
      char* end = nullptr;
      errno = 0;
      const auto longPort = std::strtol(strPort.data(), &end, 10);
      if (errno == 0 && end == strPort.data() + strPort.size()
          && longPort >= std::numeric_limits<uint16_t>::min()
          && longPort <= std::numeric_limits<uint16_t>::max())
      {
        _port = static_cast<uint16_t>(longPort);
        components |= PORT;
      }
      else
      {
        const auto localErrno = errno;
        qiLogWarning() << "Could not parse port '" << strPort << "' from url '" << url
                       << "' (errno:" << localErrno << ", strerror:'" << std::strerror(localErrno)
                       << "')";
      }
    }

    port = _port;
    host = _host;
    protocol = _scheme;
    return components;
  }

  bool operator==(const Url& lhs, const Url& rhs)
  {
    return lhs.str() == rhs.str();
  }

  std::ostream& operator<<(std::ostream& out, const Url& url)
  {
    return out << url.str();
  }

  Url toUrl(const Uri& uri)
  {
    Url url;
    url.setProtocol(uri.scheme());
    const auto optAuth = uri.authority();
    if (!optAuth.empty())
    {
      const auto auth = *optAuth;
      url.setHost(auth.host());
      const auto optPort = auth.port();
      if (!optPort.empty())
      {
        const auto port = *optPort;
        url.setPort(port);
      }
    }
    return url;
  }
}
