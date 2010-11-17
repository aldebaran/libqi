/*
** Author(s):
**  - Chris Kilner       <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_I_SUBSCRIBEHANDLER_HPP_
#define QI_TRANSPORT_I_SUBSCRIBEHANDLER_HPP_

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

#endif  // QI_TRANSPORT_I_SUBSCRIBEHANDLER_HPP_
