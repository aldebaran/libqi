/*
** Author(s):
**  - Chris Kilner       <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_SUBSCRIBE_HANDLER_HPP__
#define   __QI_TRANSPORT_SUBSCRIBE_HANDLER_HPP__

namespace qi {
  namespace transport {
    class SubscribeHandler {
    public:
      /// <summary>
      /// Subscribe handler. Used by a subscriber to receive data
      /// </summary>
      /// <param name="publishData"> The published data. </param>
      virtual void subscribeHandler(const std::string &publishData) = 0;
    };
  }
}

#endif // __QI_TRANSPORT_SUBSCRIBE_HANDLER_HPP__
