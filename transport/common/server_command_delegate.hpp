/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef LIBIPP_CALLBACKCOMMAND_HPP_
#define LIBIPP_CALLBACKCOMMAND_HPP_

#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

namespace AL {
  namespace Messaging {

/** Delegate a program should implement to receive call from ippc client
  */

class ServerCommandDelegate {
public:
  //return 0 if no result is expected
  virtual AL::ALPtr<ResultDefinition> ippcCallback (const CallDefinition &) = 0;
};

}
}

#endif /* !LIBIPPC_CALLBACKCOMMAND_HPP_ */
