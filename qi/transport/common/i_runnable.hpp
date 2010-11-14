/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_I_RUNNABLE_HPP_
# define QI_TRANSPORT_I_RUNNABLE_HPP_


namespace qi {
  namespace transport {

    class IRunnable
    {
    public:
      virtual ~IRunnable () {}

      virtual void run () = 0;
    };

  }
}

#endif  // QI_TRANSPORT_I_RUNNABLE_HPP_
