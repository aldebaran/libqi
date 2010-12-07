/*
** qi.c
** Login : <ctaf42@cgestes-de2>
** Started on  Thu Nov 18 18:11:44 2010 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
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

#include <qi/qi.h>
#include <qi/messaging.hpp>
#include <qi/messaging/src/client_impl.hpp>

struct client_private_t {
public:
  //Constructor
  client_private_t(const char *name, const char *address)
    : client(std::string(name), std::string(address))
  {}

public:
  qi::detail::ClientImpl client;
};


struct message_private_t {
  qi::serialization::Message msg;
};


qi_client_t *qi_client_create(const char *name, const char *address) {
  client_private_t *pclient = new client_private_t(name, address);
  return (qi_client_t *)pclient;
}

void      qi_client_call(qi_client_t *client, const char *method, qi_message_t *msg, qi_message_t *ret)
{
  client_private_t  *pclient  = (client_private_t *)client;
  message_private_t *preturn  = (message_private_t *)ret;
  message_private_t *pmessage = (message_private_t *)msg;

  pclient->client.call(std::string(method), pmessage->msg, preturn->msg);
}

qi_message_t *qi_message_create()
{
  message_private_t *pmsg = new message_private_t();
  return (qi_message_t *)pmsg;
}

void qi_message_write_int(qi_message_t *msg, const int i)
{
  message_private_t *pmsg = (message_private_t *)msg;
  pmsg->msg.writeInt(i);
}

void qi_message_write_string(qi_message_t *msg, const char *s)
{
  message_private_t *pmsg = (message_private_t *)msg;
  pmsg->msg.writeString(std::string(s));
}

int qi_message_read_int(qi_message_t *msg)
{
  message_private_t *pmsg = (message_private_t *)msg;
  int i;
  pmsg->msg.readInt(i);
  return i;
}

char *qi_message_read_string(qi_message_t *msg)
{
 message_private_t *pmsg = (message_private_t *)msg;
 std::string s;
 pmsg->msg.readString(s);
 //TODO: buffer overflow
#ifdef _WIN32
 return _strdup(s.c_str());
#else
 return strdup(s.c_str());
#endif
}
