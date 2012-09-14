/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "messagedispatcher.hpp"

namespace qi {

#if 0
  //Needed for handling message Timeout
  static int _gst()
  {
    static const std::string st = qi::os::getenv("QI_MESSAGE_TIMEOUT");
    if (st != "")
    {
      return atoi(st.c_str());
    }
    else
    {
      // Default timeout in NAOqi 1
      return 5 * 60;
    }
  }

  static inline unsigned int getSocketTimeout()
  {
    static int _socket_timeout = _gst();
    return _socket_timeout;
  }
#endif

  MessageDispatcher::MessageDispatcher()
  {
  }

  void MessageDispatcher::dispatch(qi::Message msg) {
    //remove the address from the messageSent map
    if (msg.type() == qi::Message::Type_Reply)
    {
      boost::mutex::scoped_lock sl(_messageSentMutex);
      MessageSentMap::iterator it;
      it = _messageSent.find(msg.id());
      if (it != _messageSent.end())
        _messageSent.erase(it);
      else
        qiLogDebug("messagedispatcher") << "Message " << msg.id() <<  " is not in the messageSent map";
    }

    {
      boost::mutex::scoped_lock sl(_signalMapMutex);
      SignalMap::iterator it;
      it = _signalMap.find(msg.service());
      if (it != _signalMap.end())
        it->second(msg);
    }
  }

  qi::SignalBase::Link
  MessageDispatcher::messagePendingConnect(unsigned int serviceId, boost::function<void (qi::Message)> fun) {
    boost::mutex::scoped_lock sl(_signalMapMutex);
    qi::Signal<void (qi::Message)> &sig = _signalMap[serviceId];
    return sig.connect(fun);
  }

  bool MessageDispatcher::messagePendingDisconnect(unsigned int serviceId, qi::SignalBase::Link linkId) {
    boost::mutex::scoped_lock sl(_signalMapMutex);
    SignalMap::iterator it;
    it = _signalMap.find(serviceId);
    if (it != _signalMap.end())
      //TODO: cleanup empty signal in the map
      return it->second.disconnect(linkId);
    return false;
  }

  void MessageDispatcher::cleanPendingMessages()
  {
    //we are deleting the Socket and want to timeout all pending request
    //or the cleanup timer ask us to remove pending request that timed out
    while (true)
    {
      MessageAddress ma;
      {
        boost::mutex::scoped_lock l(_messageSentMutex);
        MessageSentMap::iterator it = _messageSent.begin();
        if (it == _messageSent.end())
          break;
        ma = it->second;
        _messageSent.erase(it);
      }
      //generate an error message for the caller.
      qi::Message msg;
      msg.setId(ma.messageId);
      msg.setType(qi::Message::Type_Error);
      msg.setService(ma.serviceId);
      msg.setObject(ma.objectId);
      msg.setFunction(ma.functionId);
      qi::Buffer buf;
      qi::ODataStream ds(buf);
      ds << "Endpoint disconnected, message dropped.";
      msg.setBuffer(buf);
      dispatch(msg);
    }
  }

  void MessageDispatcher::sent(qi::Message msg) {
    //store Call id, we can use them later to notify the client
    //if the call did not succeed. (network disconnection, message lost)
    if (msg.type() == qi::Message::Type_Call)
    {
      boost::mutex::scoped_lock l(_messageSentMutex);
      MessageSentMap::iterator it = _messageSent.find(msg.id());
      if (it != _messageSent.end()) {
        qiLogInfo("messagedispatcher") << "Message ID conflict. A message with the same Id is already in flight" << msg.id();
        return;
      }
      _messageSent[msg.id()] = msg.address();
    }
    return;
  }



}
