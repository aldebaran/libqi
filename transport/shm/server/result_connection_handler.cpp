/*
 * result_connection_handler.cpp
 *
 *  Created on: Nov 9, 2009 at 5:33:35 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#include <alcommon-ng/transport/shm/server/result_connection_handler.hpp>

#include <alcommon-ng/transport/shm/memory/mapped_device.hpp>
#include <alcommon-ng/transport/shm/memory/mapped_segment_selector.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <alcommon-ng/transport/shm/client/result_handler.hpp>

#include <boost/bind.hpp>

namespace AL {
  namespace Transport {

ResultConnectionHandler::ResultConnectionHandler(const std::string & rdv_name, ResultHandler & resultHandler)
  : rdv_name(rdv_name),
    m_resultHandler(resultHandler) {
}

ResultConnectionHandler::~ResultConnectionHandler () {
  MappedSegmentSelector::instance().free(rdv_name.c_str());
}

void ResultConnectionHandler::run() {
  try {
    io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(rdv_name.c_str(),
      MappedSegmentSelector::MS_OPEN | MappedSegmentSelector::MS_REMOVE));
    std::iostream stream(&buf);
    std::string   result;
    unsigned int  id;

    stream >> id;
    stream >> result;

    m_resultHandler.set(id, result);

  } catch (const std::exception & e) {
    std::cerr << "[Ippc] Thread Exception caught : " << e.what() << std::endl;
  }
}

}
}
