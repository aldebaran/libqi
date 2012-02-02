/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebararan Robotics
*/

#include <qimessaging/message.h>
#include <qimessaging/serialization.hpp>

/// Message
qi_message_t *qi_message_create()
{
  qi::DataStream *pmsg = new qi::DataStream();
  return (qi_message_t *)pmsg;
}

void qi_message_destroy(qi_message_t *msg)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  delete pmsg;
}


void qi_message_write_bool(qi_message_t *msg, const char b)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeBool(b);
}

void qi_message_write_char(qi_message_t *msg, const char c)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeChar(c);
}

void qi_message_write_int(qi_message_t *msg, const int i)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeInt(i);
}

void qi_message_write_float(qi_message_t *msg, const float f)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeFloat(f);
}

void qi_message_write_double(qi_message_t *msg, const double d)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeDouble(d);
}

void qi_message_write_string(qi_message_t *msg, const char *s)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeString(std::string(s));
}

void qi_message_write_raw(qi_message_t *msg, const char *s, unsigned int size)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  pmsg->writeString(std::string(s, size));
}




char  qi_message_read_bool(qi_message_t *msg) {
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  bool b;
  pmsg->readBool(b);
  return b;

}

char  qi_message_read_char(qi_message_t *msg) {
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  char c;
  pmsg->readChar(c);
  return c;
}

int qi_message_read_int(qi_message_t *msg)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  int i;
  pmsg->readInt(i);
  return i;
}

float qi_message_read_float(qi_message_t *msg) {
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  float f;
  pmsg->readFloat(f);
  return f;
}

double qi_message_read_double(qi_message_t *msg) {
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
  double d;
  pmsg->readDouble(d);
  return d;
}

char *qi_message_read_string(qi_message_t *msg)
{
  qi::DataStream *pmsg = static_cast<qi::DataStream *>(msg);
 std::string s;
 pmsg->readString(s);
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
