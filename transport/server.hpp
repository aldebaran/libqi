/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_TRANSPORT_SERVER_HPP_
# define AL_TRANSPORT_SERVER_HPP_

#include <string>
#include <alcommon-ng/transport/common/i_threadable.hpp>
#include <alcommon-ng/transport/common/i_server_response_handler.hpp>
#include <alcommon-ng/transport/common/i_data_handler.hpp>

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
