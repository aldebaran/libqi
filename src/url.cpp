/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/url.hpp>
#include <sstream>
#include <qitype/type.hpp>

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

  bool Url::operator==(const Url& url)
  {
    return this->str() == url.str();
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

  const std::string& Url::host() const {
    return _p->host;
  }

  unsigned short Url::port() const {
    return _p->port;
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
  }

  UrlPrivate::UrlPrivate(const std::string& url)
    : url(url)
    , protocol()
    , host()
    , port(0)
    , components(0)
  {
    split_me(url);
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
      std::stringstream ss;
        ss << port;
      this->url += ":" + ss.str();
    }
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
      this->url = protocol + "://" + url;
    }
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
      this->url = protocol + "://" + url;
    }
    if (!(result & PORT)) {
      port = defaultPort;
      components |= PORT;
      std::stringstream ss;
        ss << port;
      this->url += ":" + ss.str();
    }
  }

  bool UrlPrivate::isValid() const {
    return (components & (PORT | SCHEME)) == (PORT | SCHEME);
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
      std::stringstream ss(_url.substr(place+1));
      ss >> _port;
      components |= PORT;
    }

    port = _port;
    host = _host;
    protocol = _scheme;
    return components;
  }
}


QI_EQUIVALENT_STRING_REGISTER(qi::Url, &qi::Url::str);
