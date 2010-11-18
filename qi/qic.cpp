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

#include "qi.h"
#include <qi/messaging.hpp>


struct client_private_t {
  client_private_t(const char *name, const char *address);
  qi::Client client;
};

client_private_t::client_private_t(const char *name, const char *address)
  : client(std::string(name), std::string(address))
{}


 qi_client_t *qi_client_create(const char *name, const char *address) {
  client_private_t *pclient = new client_private_t(name, address);
  return (qi_client_t *)pclient;
}

//void      qi_client_call(qi_client_t *client, message_t *msg)
//{
//  client_private_t * pclient = (client_private_t *)client;
//}
