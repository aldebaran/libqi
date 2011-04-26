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
#include <iostream>
#include <cmath>
#include <qimessaging/call_definition.hpp>
#include <qimessaging/serialization/thrift/serialize.hpp>

#include <transport/TTransportUtils.h>
#include <protocol/TJSONProtocol.h>
#include <protocol/TBinaryProtocol.h>

#include <qi/perf/dataperftimer.hpp>

using qi::perf::DataPerfTimer;

static const int gLoopCount   = 100000;


template <typename T>
std::string serializeJsonThrift(T &t)
{
  using namespace apache::thrift::transport;
  using namespace apache::thrift::protocol;
  TMemoryBuffer* buffer = new TMemoryBuffer;
  boost::shared_ptr<TTransport> trans(buffer);
  TJSONProtocol protocol(trans);

  qi::serialization::thriftSerialize(&protocol, t);

  uint8_t* buf;
  uint32_t size;
  buffer->getBuffer(&buf, &size);
  return std::string((char*)buf, (unsigned int)size);
}

template <typename T>
std::string serializeBinaryThrift(T &t)
{
  using namespace apache::thrift::transport;
  using namespace apache::thrift::protocol;
  TMemoryBuffer* buffer = new TMemoryBuffer;
  boost::shared_ptr<TTransport> trans(buffer);
  TBinaryProtocol protocol(trans);

  qi::serialization::thriftSerialize(&protocol, t);

  uint8_t* buf;
  uint32_t size;
  buffer->getBuffer(&buf, &size);
  return std::string((char*)buf, (unsigned int)size);
}

int main(int argc, char *argv[])
{
  qi::messaging::CallDefinition      def;

  DataPerfTimer dt;

  std::cout << "binary" << std::endl;
  for (int i = 0; i < 12; ++i)
  {
    unsigned int                  numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string                   request = std::string(numBytes, 'B');
    qi::messaging::CallDefinition def;

    def.args()[0] = request;

    dt.start(gLoopCount, numBytes);
    for (int j = 0; j< gLoopCount; ++j)
    {
      std::string json = serializeBinaryThrift(def);
    }
    dt.stop();
  }

  std::cout << "json" << std::endl;
  for (int i = 0; i < 12; ++i)
  {
    unsigned int                  numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string                   request = std::string(numBytes, 'B');
    qi::messaging::CallDefinition def;

    def.args()[0] = request;
    dt.start(gLoopCount, numBytes);
    for (int j = 0; j< gLoopCount; ++j)
    {
      std::string json = serializeJsonThrift(def);
    }
    dt.stop();
  }

  //AL::ALPtr<AL::Messaging::ResultDefinition> res;
  //std::cout << "Start Serialization" << std::endl;
  //std::string json = serializeJsonThrift(def);
  //std::cout << json << std::endl;
  return 0;
}

