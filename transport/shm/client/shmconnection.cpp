/*
 * connection.cpp
 *
 *  Created on: Oct 8, 2009 at 12:26:41 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/transport/shm/client/shmconnection.hpp>
#include <alcommon-ng/serialization/definition_type.hpp>
#include <alcommon-ng/serialization/oarchive.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

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
  namespace Messaging {

ShmConnection::ShmConnection (const std::string & server_name, ResultHandler & resultHandler) :
  server_name(server_name), connector(server_name), resultHandler(resultHandler) {
}

ShmConnection::~ShmConnection () {
  MappedSegmentSelector::instance().free(invite);
  resultHandler.getShmPool().task_completed(invite);
}

void ShmConnection::init (DefinitionType type) {
//  InviteGenerator::init();
  std::memset(invite, 0, SEGMENT_NAME_MAX); // valgrind
  this->handShake(type);
}

void ShmConnection::handShake (DefinitionType type) {
//  InviteGenerator::generate(invite, SEGMENT_NAME_MAX);
  std::string name = resultHandler.getShmPool().get_shm();
  for (unsigned int i = 0; i < SEGMENT_NAME_MAX - 1; ++i)
    invite[i] = name[i];
  invite[SEGMENT_NAME_MAX - 1] = 0;

  connector.handShake(invite, type);
}

void ShmConnection::send (CallDefinition & def) {
	this->init(TypeCall);
#ifdef IPPC_DEBUG
  std::cout << "DEBUG: sending   = `" << std::string(invite, SEGMENT_NAME_MAX) << "`" << std::endl;
#endif
  unsigned int id ; //= def.getRequestId() == -1 ? resultHandler.generateID() : def.getRequestId();

  if (def.getRequestId() == -1)
  {
    id = resultHandler.generateID();
  }
  else
  {
    id = def.getRequestId();
    if (id == 0)
    {
      id = resultHandler.generateID();
      def.setRequestId(id);
    }
  }

  //id = resultHandler.generateID();
  resultHandler.push(id);
  {
	  boost::mutex::scoped_lock l(resultHandler.get(id)->access);

	  {
		  CallDefinition d = def;
		  d.setRequestId(id);
		  d.asResult(false);

		  io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(invite,
				MappedSegmentSelector::MS_CREATE | MappedSegmentSelector::MS_REMOVE), &connector);
		  std::iostream stream(&buf);
		  OArchive archive(stream);

		  archive << d;
	  }

	  ResultDefinition res;
	  resultHandler.get(id)->wait_result(l);
//	  std::cout << "[!!] Caught acknowledge for void call" << std::endl;
	  if (resultHandler.get(id)->asResult())
		  res = resultHandler.get(id)->getResult();
	  else {
	    std::string message = resultHandler.get(id)->getException();
	    throw ALERROR("ShmConnection", "send", message);
		  std::cout << "[!!] error thrown" << std::endl;
	  }
  }
  resultHandler.remove(id);
}

void ShmConnection::send (CallDefinition & def, ResultDefinition & res) {
  this->init(TypeCall);
#ifdef IPPC_DEBUG
  std::cout << "DEBUG: sending   = `" << std::string(invite, SEGMENT_NAME_MAX) << "`" << std::endl;
#endif
  unsigned int id ; //= def.getRequestId() == -1 ? resultHandler.generateID() : def.getRequestId();

  //std::cout << "send: " << def.getMethodName() << std::endl;
  if (def.getRequestId() == -1)
  {
    //std::cout << "generateID" << std::endl;
    id = resultHandler.generateID();
  }
  else
  {
    //std::cout << "getRequestId" << std::endl;
    id = def.getRequestId();
    if (id == 0)
    {
      id = resultHandler.generateID();
      def.setRequestId(id);
    }
  }

  resultHandler.push(id);
  {
	  boost::mutex::scoped_lock l(resultHandler.get(id)->access);

	  {
		  CallDefinition d = def;
		  d.setRequestId(id);
		  d.asResult(true);

		  io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(invite,
				MappedSegmentSelector::MS_CREATE | MappedSegmentSelector::MS_REMOVE), &connector);
		  std::iostream stream(&buf);
		  OArchive archive(stream);

		  archive << d;
	  }
//    std::cout << "wait result for " << id << " mod: " <<  def.getModuleName() << " meth: " << def.getModuleName() << std::endl;
	  resultHandler.get(id)->wait_result(l);
    //std::cout << "End wait result for " << id << std::endl;
//	  std::cout << "[!!] Caught result" << std::endl;
	  if (resultHandler.get(id)->asResult())
		  res = resultHandler.get(id)->getResult();
	  else {
	    std::string message = resultHandler.get(id)->getException();
	    throw ALERROR("ShmConnection", "send", message);
		  std::cout << "[!!] error thrown" << std::endl;
	  }
  }
  resultHandler.remove(id);
}

void ShmConnection::send (const CallDefinition & def, ResultDefinition & res, const std::string & sender) {
  this->init(TypeCall);

  throw std::exception(); // deprecated

  CallDefinition d = def;

  if (d.getRequestId() == -1)
    d.setRequestId(resultHandler.generateID());

  d.asResult(true);
  d.setSender(sender);
  resultHandler.push(d.getRequestId());
  boost::mutex::scoped_lock l(resultHandler.get(d.getRequestId())->access);

  {
	  io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(invite,
			MappedSegmentSelector::MS_CREATE | MappedSegmentSelector::MS_REMOVE), &connector);
	  std::iostream stream(&buf);
	  OArchive archive(stream);

	  archive << d;
  }

  resultHandler.get(d.getRequestId())->wait_result(l);
  if (resultHandler.get(d.getRequestId())->asResult())
	  res = resultHandler.get(d.getRequestId())->getResult();
  else {
    std::string message = resultHandler.get(d.getRequestId())->getException();
    throw ALERROR("ShmConnection", "send", message);
  }
  resultHandler.remove(d.getRequestId());
}

void ShmConnection::send (const ResultDefinition & def) {
  this->init(def.exceptionCaught() ? TypeException : TypeResult);
#ifdef IPPC_DEBUG
  std::cout << "DEBUG: returning = `" << std::string(invite, SEGMENT_NAME_MAX) << "`" << std::endl;
#endif
//std::cout << "DEBUG: returning = `" << std::string(invite, SEGMENT_NAME_MAX) << "`" << std::endl;
  io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(invite,
		MappedSegmentSelector::MS_CREATE | MappedSegmentSelector::MS_REMOVE), &connector);
  std::iostream stream(&buf);
  OArchive archive(stream);
  archive << def;
}

}
}
