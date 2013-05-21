#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGEDISPATCHER_HPP_
#define _SRC_MESSAGEDISPATCHER_HPP_

#include <qitype/genericobject.hpp>
#include <qitype/signal.hpp>
#include <boost/thread/mutex.hpp>
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
    MessageDispatcher();

    //internal: called by Socket to tell the class that we sent a message
    void sent(const qi::Message& msg);
    //internal: called by Socket to tell the class a message have been receive
    void dispatch(const qi::Message& msg);
    void cleanPendingMessages();

    static const unsigned int ALL_OBJECTS;
    qi::SignalBase::Link messagePendingConnect(unsigned int serviceId, unsigned int objectId, boost::function<void (const qi::Message&)> fun);
    bool                 messagePendingDisconnect(unsigned int serviceId, unsigned int objectId, qi::SignalBase::Link linkId);

  public:
    typedef std::pair<unsigned int, unsigned int> Target;
    typedef Signal<const qi::Message&> OnMessageSignal;
    // use shared-ptr on signal so that we may hold it without holding the map lock
    typedef std::map<Target, boost::shared_ptr<OnMessageSignal> > SignalMap;
    typedef std::map<unsigned int, MessageAddress>                   MessageSentMap;

    SignalMap              _signalMap;
    boost::recursive_mutex _signalMapMutex;

    MessageSentMap         _messageSent;
    boost::mutex           _messageSentMutex;
  };

}

#endif  // _SRC_MESSAGEDISPATCHER_HPP_
