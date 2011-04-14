/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebararan Robotics
*/

#include <qimessaging/qi.h>
#include <qi/serialization.hpp>

/// Message
qi_message_t *qi_message_create()
{
  qi::serialization::Message *pmsg = new qi::serialization::Message();
  return (qi_message_t *)pmsg;
}

void qi_message_destroy(qi_message_t *msg)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  delete pmsg;
}


void qi_message_write_bool(qi_message_t *msg, const char b)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeBool(b);
}

void qi_message_write_char(qi_message_t *msg, const char c)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeChar(c);
}

void qi_message_write_int(qi_message_t *msg, const int i)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeInt(i);
}

void qi_message_write_float(qi_message_t *msg, const float f)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeFloat(f);
}

void qi_message_write_double(qi_message_t *msg, const double d)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeDouble(d);
}

void qi_message_write_string(qi_message_t *msg, const char *s)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeString(std::string(s));
}

void qi_message_write_raw(qi_message_t *msg, const char *s, unsigned int size)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  pmsg->writeString(std::string(s, size));
}




char  qi_message_read_bool(qi_message_t *msg) {
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  bool b;
  pmsg->readBool(b);
  return b;

}

char  qi_message_read_char(qi_message_t *msg) {
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  char c;
  pmsg->readChar(c);
  return c;
}

int qi_message_read_int(qi_message_t *msg)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  int i;
  pmsg->readInt(i);
  return i;
}

float qi_message_read_float(qi_message_t *msg) {
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  float f;
  pmsg->readFloat(f);
  return f;
}

double qi_message_read_double(qi_message_t *msg) {
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
  double d;
  pmsg->readDouble(d);
  return d;
}

char *qi_message_read_string(qi_message_t *msg)
{
  qi::serialization::Message *pmsg = static_cast<qi::serialization::Message *>(msg);
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
