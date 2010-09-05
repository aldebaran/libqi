/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_TRANSPORT_SERVER_HPP_
# define AL_TRANSPORT_SERVER_HPP_

#include <althread/altask.h>
#include <string>
#include <alcommon-ng/transport/common/i_threadable.hpp>
#include <alcommon-ng/transport/common/server_response_delegate.hpp>
#include <alcommon-ng/transport/common/i_datahandler.hpp>

namespace AL {
  namespace Transport {

    class Server : public IThreadable {
    public:
      explicit Server(const std::string &_serverAddress)
        : _serverAddress(_serverAddress),
          _dataHandler(0) {}
      virtual ~Server() {}

      virtual void setDataHandler(IDataHandler* callback) {
        _dataHandler = callback;
      }
      virtual IDataHandler *getDataHandler() {
        return _dataHandler;
      }

    protected:
      std::string        _serverAddress;
      IDataHandler        *_dataHandler;
    };
  }
}

#endif  // AL_TRANSPORT_SERVER_HPP_
