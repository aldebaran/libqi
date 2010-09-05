/*
 * result_connection_handfler.hpp
 *
 *  Created on: Nov 9, 2009 at 5:33:35 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#ifndef LIBIPPC_RESULTCONNECTIONHANDLER_HPP_
#define LIBIPPC_RESULTCONNECTIONHANDLER_HPP_

#include <alcommon-ng/messaging/result_definition.hpp>
#include <alcommon-ng/transport/common/i_runnable.hpp>

#include <alcommon-ng/transport/shm/client/result_handler.hpp>

namespace AL {
  namespace Transport {

class ResultConnectionHandler : public IRunnable {
public:
  ResultConnectionHandler(const std::string & rdv_name, ResultHandler &resultHandler);
  virtual ~ResultConnectionHandler ();

  virtual void run();

private:
  std::string rdv_name;
  ResultHandler & m_resultHandler;
};

}
}

#endif /* !LIBIPPC_RESULTCONNECTIONHANDLER_HPP_ */
