/*
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

#include <alcommon-ng/ippc.hpp>
#include <alcommon-ng/serialization/thrift/serialize.hpp>

#include <transport/TTransportUtils.h>
#include <protocol/TJSONProtocol.h>

template <typename T>
std::string serializeJsonThrift(T &t)
{
  using namespace apache::thrift::transport;
  using namespace apache::thrift::protocol;
  TMemoryBuffer* buffer = new TMemoryBuffer;
  boost::shared_ptr<TTransport> trans(buffer);
  TJSONProtocol protocol(trans);

  AL::Serialization::thriftSerialize(&protocol, t);

  uint8_t* buf;
  uint32_t size;
  buffer->getBuffer(&buf, &size);
  return std::string((char*)buf, (unsigned int)size);
}

int main(int argc, char *argv[])
{
  AL::Messaging::CallDefinition      def;

  def.setMethodName("test2");
  def.setSender("titi caca");
  def.push(41);
  //AL::ALPtr<AL::Messaging::ResultDefinition> res;
  std::cout << "Start Serialization" << std::endl;
  std::string json = serializeJsonThrift(def);
  std::cout << json << std::endl;
  return 0;
}
