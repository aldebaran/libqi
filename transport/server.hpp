/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _AL_MESSAGING_TRANSPORT_SERVER_HPP_
# define _AL_MESSAGING_TRANSPORT_SERVER_HPP_

#include <althread/altask.h>
#include <string>
#include <alcommon-ng/transport/common/threadable.hpp>
#include <alcommon-ng/transport/common/server_response_delegate.hpp>
#include <alcommon-ng/transport/common/datahandler.hpp>

namespace AL {
  namespace Transport {

    class Server : public Threadable {
    public:
      explicit Server(const std::string &_serverAddress)
        : _serverAddress(_serverAddress),
          _dataHandler(0) {}
      virtual ~Server() {}

      virtual void setDataHandler(DataHandler* callback) {
        _dataHandler = callback;
      }
      virtual DataHandler *getDataHandler() {
        return _dataHandler;
      }

    protected:
      std::string        _serverAddress;
      DataHandler        *_dataHandler;
    };
  }
}

#endif  // _AL_MESSAGING_TRANSPORT_SERVER_HPP_
