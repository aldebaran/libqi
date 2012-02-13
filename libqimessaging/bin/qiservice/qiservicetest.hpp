/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef QISERVICETEST_HPP_
# define QISERVICETEST_HPP_

# include <iostream>
# include <vector>
# include <map>

# include <qimessaging/transport.hpp>
# include <qimessaging/datastream.hpp>
# include <qimessaging/session.hpp>


namespace qi
{
  class ServiceTest : public qi::TransportServer
  {
  public:
    ServiceTest();
    virtual ~ServiceTest();

    void start(const std::string &address);

    virtual void onConnected(const qi::Message &msg);
    virtual void onWrite(const qi::Message &msg);
    virtual void onRead(const qi::Message &msg);

  private:
    void reply(const qi::Message &msg);

  private:
    qi::NetworkThread   *nthd;
    qi::TransportServer *ts;
  };
}; // !qi

#endif	    /* !QISERVICETEST_HPP_ */
