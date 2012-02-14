/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebararan Robotics
*/

#include <qimessaging/message.h>
#include <qimessaging/datastream.hpp>
#include <qimessaging/message.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>

typedef struct
{
  qi::DataStream *ds;
  qi::Message    *msg;
} qi_message_data_t;


/// Message
qi_message_t *qi_message_create()
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(malloc(sizeof(qi_message_data_t)));
  m->ds = new qi::DataStream();
  m->msg = new qi::Message();

  return reinterpret_cast<qi_message_t*>(m);
}

void qi_message_destroy(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  delete m->ds;
  delete m->msg;

  free(m);
}

void qi_message_set_type(qi_message_t *msg, qi_message_type_t type)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  switch (type)
  {
    case call:
      m->msg->setType(qi::Message::Call);
    case answer:
      m->msg->setType(qi::Message::Answer);
    case event:
      m->msg->setType(qi::Message::Event);
    case error:
      m->msg->setType(qi::Message::Error);
    case none:
      m->msg->setType(qi::Message::None);
    default:
      assert(false);
  }
}

qi_message_type_t qi_message_get_type(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  switch (m->msg->type())
  {
    case qi::Message::Call:
      return call;
    case qi::Message::Answer:
      return answer;
    case qi::Message::Event:
      return event;
    case qi::Message::Error:
      return error;
    case qi::Message::None:
      return none;
    default:
      assert(false);
  }
  return none;
}

void qi_message_set_id(qi_message_t *msg, unsigned int id)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  m->msg->setId(id);
}

unsigned int qi_message_get_id(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  return m->msg->id();
}

void qi_message_set_src(qi_message_t *msg, char *src)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  m->msg->setSource(src);
}

char *qi_message_get_src(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string r = m->msg->source();

#ifdef _WIN32
  return _strdup(r.c_str());
#else
  return strdup(r.c_str());
#endif

}

void qi_message_set_dst(qi_message_t *msg, char *dst)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  m->msg->setDestination(dst);
}

char *qi_message_get_dst(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string r = m->msg->destination();

#ifdef _WIN32
  return _strdup(r.c_str());
#else
  return strdup(r.c_str());
#endif
}

void qi_message_set_path(qi_message_t *msg, char *path)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  m->msg->setPath(path);
}

char *qi_message_get_path(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string r = m->msg->path();

#ifdef _WIN32
  return _strdup(r.c_str());
#else
  return strdup(r.c_str());
#endif
}

void qi_message_set_data(qi_message_t *msg, char *data)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  m->msg->setData(data);
}

char *qi_message_get_data(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string r = m->msg->data();

#ifdef _WIN32
  return _strdup(r.c_str());
#else
  return strdup(r.c_str());
#endif
}

void qi_message_write_bool(qi_message_t *msg, char b)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << b;
}

void qi_message_write_char(qi_message_t *msg, char c)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << c;
}

void qi_message_write_int(qi_message_t *msg, int i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << i;
}

void qi_message_write_float(qi_message_t *msg, float f)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << f;
}

void qi_message_write_double(qi_message_t *msg, double d)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << d;
}

void qi_message_write_string(qi_message_t *msg, const char *s)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << std::string(s);
}

void qi_message_write_raw(qi_message_t *msg, const char *s, unsigned int size)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  *m->ds << std::string(s, size);
}

char qi_message_read_bool(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  bool b;
  *m->ds >> b;
  return b;
}

char qi_message_read_char(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  char c;
  *m->ds >> c;
  return c;
}

int qi_message_read_int(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  int i;
  *m->ds >> i;
  return i;
}

float qi_message_read_float(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  float f;
  *m->ds >> f;
  return f;
}

double qi_message_read_double(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  double d;
  *m->ds >> d;
  return d;
}

char *qi_message_read_string(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string s;
  *m->ds >> s;
  //TODO: buffer overflow
#ifdef _WIN32
  return _strdup(s.c_str());
#else
  return strdup(s.c_str());
#endif
}

char *qi_message_read_raw(qi_message_t *msg, unsigned int *size)
{
  //TODO set size
  return qi_message_read_string(msg);
}
