/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "messagedispatcher.hpp"
#include <ka/errorhandling.hpp>

static const auto logCategory = "qimessaging.messagedispatcher";
qiLogCategory(logCategory);

// Helper for debug logs of the class. Can only be called from non static member functions of
// `qi::MessageDispatcher`.
#define QI_LOG_DEBUG_MSGDISPATCHER() \
  qiLogDebug() << this << " - "

namespace qi
{

namespace
{

  /// std::map || boost::flat_map M
  template<typename M>
  boost::optional<typename M::mapped_type> find(const M& map, const typename M::key_type& key)
  {
    const auto it = map.find(key);
    if (it == map.end())
      return {};
    return it->second;
  }

}

MessageDispatcher::MessageDispatcher(ExecutionContext& execContext)
  : _execContext{ execContext }
{
}

Future<bool> MessageDispatcher::dispatch(Message& msg)
{
  QI_LOG_DEBUG_MSGDISPATCHER() << "Posting a message " << msg.address() << " for dispatch.";
  return _execContext.async([=] {
    const auto handlers = find(_state->recipients, RecipientId{ msg.service(), msg.object() })
                            .value_or(MessageHandlerList{});
    QI_LOG_DEBUG_MSGDISPATCHER() << "Dispatching a message " << msg.address() << " to "
                                 << handlers.size() << " handlers.";
    return tryDispatch(handlers, msg);
  });
}

SignalLink MessageDispatcher::messagePendingConnect(unsigned int serviceId,
                                                    unsigned int objectId,
                                                    MessageHandler fun) noexcept
{
  auto state = _state.synchronize();
  auto& handlers = state->recipients[RecipientId{ serviceId, objectId }];
  const auto newSignalLinkId = state->nextSignalLink++;
  QI_LOG_DEBUG_MSGDISPATCHER() << "Connecting a handler (linkId=" << newSignalLinkId
                               << ") for message dispatch for service=" << serviceId
                               << ", object=" << objectId;
  handlers.emplace(newSignalLinkId, std::move(fun));
  return newSignalLinkId;
}

bool MessageDispatcher::messagePendingDisconnect(unsigned int serviceId,
                                                 unsigned int objectId,
                                                 SignalLink linkId) noexcept
{
  auto state = _state.synchronize();
  const auto it = state->recipients.find(RecipientId{ serviceId, objectId });
  if (it == state->recipients.end())
    return false;

  QI_LOG_DEBUG_MSGDISPATCHER()
    << "Disconnecting a handler (linkId=" << linkId
    << ") for message dispatch for service=" << serviceId << ", object=" << objectId;
  auto& handlers = it->second;
  const auto disconnectedCount = handlers.erase(linkId);
  if (handlers.empty())
    state->recipients.erase(it);
  return disconnectedCount == 1;
}

bool MessageDispatcher::tryDispatch(const MessageHandlerList& handlers, const Message& msg)
{
  // Continue dispatching if an exception was thrown.
  // onHandlerError: (std::exception || boost::exception || void) -> bool
  static const auto onHandlerError =
    ka::compose([] { return DispatchStatus::MessageHandlingFailure; },
                exceptionLogError(logCategory, "Message handler failed"));

  using Slot = MessageHandlerList::value_type;
  return std::any_of(handlers.begin(), handlers.end(), [&](const Slot& slot) {
    const auto& handler = slot.second;
    return isMessageHandled(ka::invoke_catch(onHandlerError, handler, msg));
  });
}
}
