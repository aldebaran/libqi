#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGEDISPATCHER_HPP_
#define _SRC_MESSAGEDISPATCHER_HPP_

#include <qi/anyobject.hpp>

#include <boost/thread/synchronized_value.hpp>
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
  class MessageDispatcher
  {
  public:
    using MessageHandler = std::function<DispatchStatus (const Message&)>;

    MessageDispatcher(ExecutionContext& execContext);

    Future<bool> dispatch(Message msg);

    qi::SignalLink messagePendingConnect(unsigned int serviceId,
                                         unsigned int objectId,
                                         MessageHandler fun) noexcept;

    /// @invariant
    ///   `d.messagePendingDisconnect(sid, oid, d.messagePendingConnect(sid, oid, _)) == true`
    bool messagePendingDisconnect(unsigned int serviceId,
                                  unsigned int objectId,
                                  qi::SignalLink linkId) noexcept;

  public:
    struct RecipientId
    {
      unsigned int serviceId;
      unsigned int objectId;
      KA_GENERATE_FRIEND_REGULAR_OPS_2(RecipientId, serviceId, objectId)
    };

    ExecutionContext& _execContext;

    using MessageHandlerList = boost::container::flat_map<SignalLink, MessageHandler>;
    using RecipientMessageHandlerMap = boost::container::flat_map<RecipientId, MessageHandlerList>;

    // Mutable state of the object.
    struct State
    {
      RecipientMessageHandlerMap recipients;
      SignalLink nextSignalLink = 0;
    };
    using SyncState =  boost::synchronized_value<State>;
    SyncState _state;

  private:
    static bool tryDispatch(const MessageHandlerList& handlers, const Message& msg);
  };
}

#endif  // _SRC_MESSAGEDISPATCHER_HPP_
