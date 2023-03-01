/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <qi/buffer.hpp>
#include <qi/types.hpp>

int main(int, char **)
{
  // Create a buffer and add a string in it.
  qi::Buffer buffer;
  unsigned char tmpChar[] = "ahahahahahahLOLKikou";
  buffer.write(tmpChar, sizeof(tmpChar));

  // Read the string in the buffer.
  const unsigned char *content = (const unsigned char*)buffer.read();
  std::cout << content << std::endl;

  // Create a buffer and add it as a sub-buffer.
  size_t offset;
  {
    unsigned char tmpChar2[] = "Oh! Oh! Oh! C'est le père Noël!";
    qi::Buffer subBuffer;
    subBuffer.write(tmpChar2, sizeof(tmpChar2));
    offset = buffer.addSubBuffer(subBuffer);
    // Here we destroy "subBuffer".
  }

  // But he's still in buffer.
  std::cout << (const unsigned char*)buffer.subBuffer(offset).read() << std::endl;

  // Add int and double in the buffer.
  int tmpInt = 42;
  buffer.write(&tmpInt, sizeof(int));
  double tmpDouble = 42;
  buffer.write(&tmpDouble, sizeof(double));

  // Read the int.
  if (buffer.read(&tmpInt, offset + sizeof(qi::uint32_t), sizeof(int)) != sizeof(int)) {
    std::cerr << "Can't read an int in the buffer!" << std::endl;
    return 1;
  }

  std::cout << "We read " << tmpInt << std::endl;

  // Read the double.
  if (buffer.read(&tmpDouble, offset + sizeof(qi::uint32_t) + sizeof(int), sizeof(double)) != sizeof(double)) {
    std::cerr << "Can't read a double in the buffer!" << std::endl;
    return 1;
  }

  std::cout << "We read " << tmpDouble << std::endl;

  return 0;
}
