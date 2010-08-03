/*
 * result_connection_handler.cpp
 *
 *  Created on: Nov 9, 2009 at 5:33:35 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#include <alcommon-ng/transport/shm/server/result_connection_handler.hpp>

#include <alcommon-ng/serialization/iarchive.hpp>
#include <alcommon-ng/transport/shm/memory/mapped_device.hpp>
#include <alcommon-ng/transport/shm/memory/mapped_segment_selector.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <alcommon-ng/transport/shm/client/result_handler.hpp>

#include <boost/bind.hpp>

namespace AL {
  namespace Messaging {

ResultConnectionHandler::ResultConnectionHandler (const std::string & rdv_name, ResultHandler & resultHandler) :
  rdv_name(rdv_name), m_resultHandler(resultHandler) {
}

ResultConnectionHandler::~ResultConnectionHandler () {
	MappedSegmentSelector::instance().free(rdv_name.c_str());
}

void ResultConnectionHandler::run() {
#ifdef IPPC_DEBUG
  std::cout << "DEBUG: res recv  = `" << rdv_name << "`" << std::endl;
#endif
  try {
	  io::stream_buffer<MappedDevice> buf(MappedSegmentSelector::instance().get(rdv_name.c_str(),
	      MappedSegmentSelector::MS_OPEN | MappedSegmentSelector::MS_REMOVE));
		std::iostream stream(&buf);
		IArchive archive(stream);
		ResultDefinition res;
		archive >> res;
		//std::cout << "[debug] Got the result definition ! id = `" << res.getRequestId() << "'" << std::endl;
    res.getRequestId();
    //res.getRequestId()->access;
    if (m_resultHandler.get(res.getRequestId()) == NULL)
    {
     // return;
    }
    m_resultHandler.get(res.getRequestId())->access;
//		std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! REQUEST ID [" << res.getRequestId() << "]" << std::endl;
		boost::mutex::scoped_lock l(m_resultHandler.get(res.getRequestId())->access);
		m_resultHandler.get(res.getRequestId())->setResult(res);


  } catch (const std::exception & e) {
    std::cerr << "[Ippc] Thread Exception caught : " << e.what() << std::endl;
  }
}

}
}