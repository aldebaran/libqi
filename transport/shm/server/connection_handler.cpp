/*
 * connection_handler.cpp
 *
 *  Created on: Oct 13, 2009 at 2:26:49 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/transport/shm/server/connection_handler.hpp>
#include <alcommon-ng/serialization/serialization.hpp>

#include <alcommon-ng/transport/common/handlers_pool.hpp>

#include <alcommon-ng/transport/shm/memory/mapped_device.hpp>
#include <alcommon-ng/transport/shm/memory/mapped_segment_selector.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <boost/bind.hpp>

#include <boost/interprocess/shared_memory_object.hpp>

#include <alcommon-ng/collections/variables_list.hpp>

namespace AL {
  namespace Messaging {

  ConnectionHandler::ConnectionHandler (const std::string & rdv_name, ServerCommandDelegate * callback, internal::ServerResponseDelegate *responsedelegate)
    : rdv_name(rdv_name),
      callback(callback),
      response_delegate(responsedelegate)
  {
    this->setTaskName("ConnectionHandler");
  }

  ConnectionHandler::~ConnectionHandler () {
    MappedSegmentSelector::instance().free(rdv_name.c_str());
  }

  void ConnectionHandler::run() {
#ifdef IPPC_DEBUG
    std::cout << "DEBUG: receiving = `" << rdv_name << "`" << std::endl;
#endif
    CallDefinition def;

    try {
      io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(rdv_name.c_str(),
                                                                                            MappedSegmentSelector::MS_OPEN | MappedSegmentSelector::MS_REMOVE));
      std::iostream stream(&buf);
      IArchive archive(stream);
      archive >> def;

    } catch (const std::exception & e) {
      std::cerr << "[IppcThread] Exception caught : " << e.what() << std::endl;
    }

    boost::interprocess::shared_memory_object::remove(rdv_name.c_str());

    if (callback) {
      try
      {
        AL::ALPtr<ResultDefinition> result = callback->ippcCallback(def);
        if (response_delegate)
          response_delegate->sendResponse(def, result);
      }
      catch (...)
      {
        std::cout << "Exception catch at ConnectionHandler::run level" << std::endl;
      }
    }
  }

}
}