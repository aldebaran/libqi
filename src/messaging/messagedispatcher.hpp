#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGEDISPATCHER_HPP_
#define _SRC_MESSAGEDISPATCHER_HPP_

#include <qi/anyobject.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/container/flat_map.hpp>
#include "message.hpp"

namespace qi {

  /**
   * @brief The MessageDispatcher class dispatches messages from a TransportSocket
   * \internal
   *
   * Receive message from a TransportSocket and send them on the appropriate
   * signal, based on the serviceId of the message.
   *
   * This class generate an error message for all pending message that have timed out.
   * at the moment it only generate message if the socket have been disconnected.
   *
   * TODO: handle timeout on request taking too long to complete
   */
  class MessageDispatcher {
  public:
    using MessageHandler = std::function<DispatchStatus (const Message&)>;

    MessageDispatcher(ExecutionContext& execContext);

    //internal: called by Socket to tell the class that we sent a message
    void sent(const qi::Message& msg);
    //internal: called by Socket to tell the class a message have been receive
    void dispatch(const qi::Message& msg);
    void cleanPendingMessages();

    static const unsigned int ALL_OBJECTS;
    qi::SignalLink messagePendingConnect(unsigned int serviceId, unsigned int objectId, MessageHandler fun);
    void           messagePendingDisconnect(unsigned int serviceId, unsigned int objectId, qi::SignalLink linkId);

  public:
    using Target = std::pair<unsigned int, unsigned int>;
    using MessageHandlerList = boost::container::flat_map<SignalLink, MessageHandler>;

    using SignalMap = boost::container::flat_map<Target, MessageHandlerList>;
    using MessageSentMap = boost::container::flat_map<unsigned int, MessageAddress>;

    ExecutionContext&      _execContext;
    SignalMap              _signalMap;
    SignalLink             _nextSignalLink = 0;
    boost::recursive_mutex _signalMapMutex;

    MessageSentMap         _messageSent;
    boost::mutex           _messageSentMutex;
  };

}

#endif  // _SRC_MESSAGEDISPATCHER_HPP_
