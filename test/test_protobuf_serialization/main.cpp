/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <gtest/gtest.h>
#include <protobuf/message.h>
#include <protobuf/dynamic_message.h>
#include <protobuf/descriptor.h>
#include "myproto.pb.h"

using namespace google::protobuf;

TEST(TestProtoSerialization, Basic) {
  testproto::Person p;
  p.set_name("toto");

  testproto::Person_PhoneNumber *phone = p.add_phone();
  phone->set_number("0642");

  std::cout << p.phone(0).number() << std::endl;

  std::cout << p.name() << std::endl;

  std::cout << "ProtoTest" << std::endl;
}
