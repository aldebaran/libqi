/*
 * connection.cpp
 *
 *  Created on: Oct 8, 2009 at 12:26:41 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/transport/shm/client/shmconnection.hpp>
#include <alcommon-ng/transport/shm/definition_type.hpp>

#include <alcommon-ng/transport/shm/memory/mapped_device.hpp>
#include <alcommon-ng/transport/shm/memory/mapped_segment_selector.hpp>

#include <alcommon-ng/transport/shm/util/invite_generator.hpp>

#include <alcommon-ng/transport/shm/client/result_handler.hpp>

#include <iostream>
#include <exception>
#include <boost/thread/mutex.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <alcommon-ng/exceptions/exceptions.hpp>

#include <alcore/alerror.h>

namespace io = boost::iostreams;

namespace AL {
  namespace Transport {

ShmConnection::ShmConnection(const std::string & server_name, ResultHandler & resultHandler)
  : server_name(server_name),
    connector(server_name),
    resultHandler(resultHandler) {
}

ShmConnection::~ShmConnection () {
  MappedSegmentSelector::instance().free(invite);
  resultHandler.getShmPool().task_completed(invite);
}

void ShmConnection::init (DefinitionType type) {
  std::memset(invite, 0, SEGMENT_NAME_MAX);
  this->handShake(type);
}

void ShmConnection::handShake (DefinitionType type) {
  std::string name = resultHandler.getShmPool().get_shm();
  memcpy(invite, name.c_str(), SEGMENT_NAME_MAX - 1);
  invite[SEGMENT_NAME_MAX - 1] = 0;
  connector.handShake(invite, type);
}

//send tosend, and return result
void ShmConnection::send(const std::string &tosend, std::string &result)
{
  this->init(TypeCall);
  unsigned int id;

  id = resultHandler.generateID();
  //resultHandler.push(id);
  {
    //TODO: oups
    //boost::mutex::scoped_lock l(resultHandler.get(id)->access);
    {
      io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(invite,
        MappedSegmentSelector::MS_CREATE | MappedSegmentSelector::MS_REMOVE), &connector);
      std::iostream stream(&buf);
      stream << id;
      stream << tosend;
    }
    std::cout << "wait result for " << id << " mod: " << std::endl;
    //resultHandler.get(id)->wait_result(l);
    std::cout << "End wait result for " << id << std::endl;

//    if (resultHandler.get(id)->asResult())
//      res = resultHandler.get(id)->getResult();
//    else {
//      std::string message = resultHandler.get(id)->getException();
//      throw ALERROR("ShmConnection", "send", message);
//      std::cout << "[!!] error thrown" << std::endl;
//    }
  }
  //resultHandler.remove(id);
}

//send back a result to the server
void ShmConnection::send(const std::string& result) {
  this->init(TypeResult);
  io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(invite,
                MappedSegmentSelector::MS_CREATE | MappedSegmentSelector::MS_REMOVE), &connector);
  std::iostream stream(&buf);
  stream << result;
  //OArchive archive(stream);
  //archive << def;
}

}
}
