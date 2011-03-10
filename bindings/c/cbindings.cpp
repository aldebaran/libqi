/*
** qi.c
** Login : <ctaf42@cgestes-de2>
** Started on  Thu Nov 18 18:11:44 2010 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Cedric GESTES
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
#include <qi/messaging/src/server_impl.hpp>


struct message_private_t {
  qi::serialization::Message msg;
};


/// Context

qi_context_t *qi_context_create() {
  qi::Context *pctx = new qi::Context();
  return (qi_context_t *)pctx;
}

void qi_context_destroy(qi_context_t *ctx) {
  if (!ctx)
    return;
  qi::Context *pctx = (qi::Context *)ctx;
  delete pctx;
}


/// Client

qi_client_t *qi_client_create(const char *name) {
  qi::detail::ClientImpl *pclient = new qi::detail::ClientImpl(name);
  return (qi_client_t *)pclient;
}

qi_client_t *qi_client_create_with_context(const char *name, qi_context_t *ctx) {
  qi::Context *pctx = (qi::Context *)ctx;
  qi::detail::ClientImpl *pclient = new qi::detail::ClientImpl(name, pctx);
  return (qi_client_t *)pclient;
}

void qi_client_connect(qi_client_t *client, const char *address)
{
  qi::detail::ClientImpl  *pclient  = (qi::detail::ClientImpl *)client;
  pclient->connect(address);
}

void      qi_client_destroy(qi_client_t *client)
{
  qi::detail::ClientImpl  *pclient  = (qi::detail::ClientImpl *)client;
  delete pclient;
}

void      qi_client_call(qi_client_t *client, const char *method, qi_message_t *msg, qi_message_t *ret)
{
  qi::detail::ClientImpl  *pclient  = (qi::detail::ClientImpl *)client;
  qi::serialization::Message *preturn  = (qi::serialization::Message *)ret;
  qi::serialization::Message *pmessage = (qi::serialization::Message *)msg;

  pclient->call(std::string(method), *pmessage, *preturn);
}


/// Server
qi_server_t *qi_server_create(const char *name) {
  qi::detail::ServerImpl *pserver = new qi::detail::ServerImpl(name);
  return (qi_server_t *)pserver;
}

void         qi_server_destroy(qi_server_t *server) {
  qi::detail::ServerImpl  *pserver  = (qi::detail::ServerImpl *)server;
  delete pserver;
}

void         qi_server_connect(qi_server_t *server, const char *address) {
  qi::detail::ServerImpl  *pserver  = (qi::detail::ServerImpl *)server;
  pserver->connect(address);
}

class CFunctor : public qi::Functor {
public:
  CFunctor(BindedMethod func, void *data = 0)
    : _func(func),
      _data(data)
  {
    ;
  }

  virtual void call(qi::serialization::Message &params, qi::serialization::Message& result)const {
    if (_func)
      _func((qi_message_t *)&params, (qi_message_t *)&result, _data);
  }

  virtual ~CFunctor() {
  }

private:
  BindedMethod  _func;
  void         *_data;

};

void         qi_server_advertise_service(qi_server_t *server, const char *methodSignature, BindedMethod func, void *data) {
  qi::detail::ServerImpl  *pserver  = (qi::detail::ServerImpl *)server;
  CFunctor *fun = new CFunctor(func, data);
  pserver->advertiseService(methodSignature, fun);
}

void         qi_server_unadvertise_service(qi_server_t *server, const char *methodSignature) {
  qi::detail::ServerImpl  *pserver  = (qi::detail::ServerImpl *)server;
  pserver->unadvertiseService(methodSignature);
}


/// Message

qi_message_t *qi_message_create()
{
  qi::serialization::Message *pmsg = new qi::serialization::Message();
  return (qi_message_t *)pmsg;
}

void qi_message_destroy(qi_message_t *msg)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  delete pmsg;
}


void qi_message_write_bool(qi_message_t *msg, const char b)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeBool(b);
}

void qi_message_write_char(qi_message_t *msg, const char c)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeChar(c);
}

void qi_message_write_int(qi_message_t *msg, const int i)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeInt(i);
}

void qi_message_write_float(qi_message_t *msg, const float f)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeFloat(f);
}

void qi_message_write_double(qi_message_t *msg, const double d)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeDouble(d);
}

void qi_message_write_string(qi_message_t *msg, const char *s)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeString(std::string(s));
}

void qi_message_write_raw(qi_message_t *msg, const char *s, unsigned int size)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  pmsg->writeString(std::string(s, size));
}




char  qi_message_read_bool(qi_message_t *msg) {
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  bool b;
  pmsg->readBool(b);
  return b;

}

char  qi_message_read_char(qi_message_t *msg) {
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  char c;
  pmsg->readChar(c);
  return c;
}

int qi_message_read_int(qi_message_t *msg)
{
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  int i;
  pmsg->readInt(i);
  return i;
}

float qi_message_read_float(qi_message_t *msg) {
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  float f;
  pmsg->readFloat(f);
  return f;
}

double qi_message_read_double(qi_message_t *msg) {
  qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
  double d;
  pmsg->readDouble(d);
  return d;
}

char *qi_message_read_string(qi_message_t *msg)
{
 qi::serialization::Message *pmsg = (qi::serialization::Message *)msg;
 std::string s;
 pmsg->readString(s);
 //TODO: buffer overflow
#ifdef _WIN32
 return _strdup(s.c_str());
#else
 return strdup(s.c_str());
#endif
}

char *qi_message_read_raw(qi_message_t *msg)
{
  return qi_message_read_string(msg);
}


// MASTER API
char *qi_master_locate_service(qi_client_t *client, const char *signature)
{
  qi::detail::ClientImpl     *pclient = (qi::detail::ClientImpl *)client;
  qi::serialization::Message  message;
  qi::serialization::Message  ret;

  message.writeString("master.locateService::s:ss");
  message.writeString(signature);
  message.writeString(pclient->endpointId());

  pclient->call(std::string("master.locateService::s:ss"), message, ret);
  std::string addr;
  ret.readString(addr);
  return strdup(addr.c_str());
}
