/*
** network_client.hpp
** Login : <ctaf42@donnetout>
** Started on  Thu Feb  2 11:59:48 2012 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef   	NETWORK_CLIENT_HPP_
# define   	NETWORK_CLIENT_HPP_

#include <qimessaging/transport/transport_socket.hpp>

#include <vector>
#include <string>

namespace qi {

  class NetworkThread;
  class Object;

  class Session : public qi::TransportSocketInterface {
  public:
    Session();
    virtual ~Session();

    void onConnected(TransportSocket *client);
    void onDisconnected(TransportSocket *client);
    void onWriteDone(TransportSocket *client);
    void onReadyRead(TransportSocket *client, qi::Message &msg);


    void connect(const std::string &masterAddress);
    bool disconnect();
    void join();
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

    std::vector<std::string> services();

    qi::TransportSocket* serviceSocket(const std::string &name,
                                       unsigned int      *idx,
                                       const std::string &type = "tcp");

    qi::Object* service(const std::string &service,
                        const std::string &type = "tcp");

    qi::TransportSocket *tc;
    qi::NetworkThread   *_nthd;
  };
}


#endif	    /* !NETWORK_CLIENT_PP_ */
