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

//     // Same as the last block, but do it dynamically via the Message
//     // reflection interface.
//     Message* foo = new Foo;
//     Descriptor* descriptor = foo->GetDescriptor();
//
//     // Get the descriptors for the fields we're interested in and verify
//     // their types.
//     FieldDescriptor* text_field = descriptor->FindFieldByName("text");
//     assert(text_field != NULL);
//     assert(text_field->type() == FieldDescriptor::TYPE_STRING);
//     assert(text_field->label() == FieldDescriptor::TYPE_OPTIONAL);
//     FieldDescriptor* numbers_field = descriptor->FindFieldByName("numbers");
//     assert(numbers_field != NULL);
//     assert(numbers_field->type() == FieldDescriptor::TYPE_INT32);
//     assert(numbers_field->label() == FieldDescriptor::TYPE_REPEATED);
//
//     // Parse the message.
//     foo->ParseFromString(data);
//
//     // Use the reflection interface to examine the contents.
//     const Reflection* reflection = foo->GetReflection();
//     assert(reflection->GetString(foo, text_field) == "Hello World!");
//     assert(reflection->FieldSize(foo, numbers_field) == 3);
//     assert(reflection->GetRepeatedInt32(foo, numbers_field, 0) == 1);
//     assert(reflection->GetRepeatedInt32(foo, numbers_field, 1) == 5);
//     assert(reflection->GetRepeatedInt32(foo, numbers_field, 2) == 42);
//
//     delete foo;
using namespace google::protobuf;

TEST(TestProtoSerialization, Basic) {
  //Descriptor *desc = new Descriptor();

  //DynamicMessageFactory().GetPrototype( Message *foo;

  //Message *prototype = factory_.GetPrototype(descriptor_);

  //protobuf::Descriptor *foo.desc;


  std::cout << "ProtoTest" << std::endl;
}
