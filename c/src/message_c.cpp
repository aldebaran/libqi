/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebararan Robotics
*/

#include <qimessaging/c/message_c.h>
#include <qimessaging/message.hpp>
#include <qimessaging/binaryencoder.hpp>
#include <qimessaging/binarydecoder.hpp>
#include "message_c_p.h"
#include <cstring>
#include <cstdlib>
#include <cassert>

qi::BinaryDecoder &get_is(qi_message_data_t *m)
{
  if (!m->is)
    m->is = new qi::BinaryDecoder (*m->buff);
  return *(m->is);
}

qi::BinaryEncoder &get_os(qi_message_data_t *m)
{
  if (!m->os)
    m->os = new qi::BinaryEncoder (*m->buff);
  return *(m->os);
}

/// Message
qi_message_t *qi_message_create()
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(malloc(sizeof(qi_message_data_t)));
  m->buff = new qi::Buffer;
  m->msg = new qi::Message;
  m->msg->setBuffer(*m->buff);
  m->is = 0;
  m->os = 0;
  return reinterpret_cast<qi_message_t*>(m);
}

void qi_message_destroy(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  delete m->is;
  delete m->os;
  delete m->msg;
  delete m->buff;

  free(m);
}

void qi_message_write_bool(qi_message_t *msg, char b)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << b;
}

void qi_message_write_int8(qi_message_t *msg, char c)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << c;
}

void qi_message_write_int16(qi_message_t *msg, short i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << i;
}

void qi_message_write_int32(qi_message_t *msg, int i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << i;
}

void qi_message_write_int64(qi_message_t *msg, long long i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << i;
}

void qi_message_write_uint8(qi_message_t *msg, unsigned char c)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << c;
}

void qi_message_write_uint16(qi_message_t *msg, unsigned short i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << i;
}

void qi_message_write_uint32(qi_message_t *msg, unsigned int i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << i;
}

void qi_message_write_uint64(qi_message_t *msg, unsigned long long i)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << i;
}



void qi_message_write_float(qi_message_t *msg, float f)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << f;
}

void qi_message_write_double(qi_message_t *msg, double d)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << d;
}

void qi_message_write_string(qi_message_t *msg, const char *s)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);

  get_os(m) << std::string(s);
}

void qi_message_write_raw(qi_message_t *msg, const char *s, unsigned int size)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  get_os(m) << std::string(s, size);
}

void          qi_message_write_list_begin(qi_message_t *msg, unsigned int size)
{
  qi_message_write_uint32(msg, size);
}

void          qi_message_write_map_begin(qi_message_t *msg, unsigned int size)
{
  qi_message_write_uint32(msg, size);
}

void          qi_message_write_tuple_begin(qi_message_t *msg, unsigned int size)
{
  qi_message_write_uint32(msg, size);
}

char qi_message_read_bool(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  bool b;
  get_is(m) >> b;
  return b;
}

char qi_message_read_int8(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  char c;
  get_is(m) >> c;
  return c;
}

short qi_message_read_int16(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  short c;
  get_is(m) >> c;
  return c;
}

int qi_message_read_int32(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  int i;
  get_is(m) >> i;
  return i;
}

long long qi_message_read_int64(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  long long i;
  get_is(m) >> i;
  return i;
}

unsigned char qi_message_read_uint8(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  unsigned char c;
  get_is(m) >> c;
  return c;
}

unsigned short qi_message_read_uint16(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  unsigned short c;
  get_is(m) >> c;
  return c;
}

unsigned int qi_message_read_uint32(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  unsigned int i;
  get_is(m) >> i;
  return i;
}

unsigned long long qi_message_read_uint64(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  unsigned long long i;
  get_is(m) >> i;
  return i;
}


float qi_message_read_float(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  float f;
  get_is(m) >> f;
  return f;
}

double qi_message_read_double(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  double d;
  get_is(m) >> d;
  return d;
}

char *qi_message_read_string(qi_message_t *msg)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string s;
  get_is(m) >> s;
  return qi::os::strdup(s.c_str());
}

void qi_message_free_string(char *str) {
  if (str)
    free(str);
}

char *qi_message_read_raw(qi_message_t *msg, unsigned int *size)
{
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(msg);
  std::string s;
  get_is(m) >> s;

  *size = s.size();
  return qi::os::strdup(s.c_str());
}

void qi_message_free_raw(char *raw) {
  if (raw)
    free(raw);
}

unsigned int  qi_message_read_list_size(qi_message_t *msg)
{
  return qi_message_read_uint32(msg);
}

unsigned int  qi_message_read_map_size(qi_message_t *msg)
{
  return qi_message_read_uint32(msg);
}

unsigned int  qi_message_read_tuple_size(qi_message_t *msg)
{
  return qi_message_read_uint32(msg);
}
