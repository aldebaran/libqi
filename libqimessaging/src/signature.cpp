/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/signature.hpp>

namespace qi {

qi::SignatureStream &operator&(qi::SignatureStream &os, const bool&) {
  os.write(qi::Signature::Type_Bool);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const char&) {
  os.write(qi::Signature::Type_Int8);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const signed char&) {
  os.write(qi::Signature::Type_Int8);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned char&) {
  os.write(qi::Signature::Type_UInt8);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const short&) {
  os.write(qi::Signature::Type_Int16);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned short&) {
  os.write(qi::Signature::Type_UInt16);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const int&) {
  os.write(qi::Signature::Type_Int32);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned int&) {
  os.write(qi::Signature::Type_UInt32);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const long&) {
  os.write(qi::Signature::Type_Int64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned long&) {
  os.write(qi::Signature::Type_UInt64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const long long&) {
  os.write(qi::Signature::Type_Int64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned long long&) {
  os.write(qi::Signature::Type_UInt64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const float&) {
  os.write(qi::Signature::Type_Float);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const double&) {
  os.write(qi::Signature::Type_Double);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const std::string&) {
  os.write(qi::Signature::Type_String);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const char*) {
  os.write(qi::Signature::Type_String);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, char*) {
  os.write(qi::Signature::Type_String);
  return os;
}

}
