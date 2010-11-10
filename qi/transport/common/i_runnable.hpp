/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  AL_TRANSPORT_I_RUNNABLE_HPP_
# define AL_TRANSPORT_I_RUNNABLE_HPP_
#include <althread/altask.h>

namespace qi {
  namespace Transport {

    class IRunnable: public AL::ALTask
    {
    public:
      virtual ~IRunnable () {}

      virtual void run () = 0;
    };

  }
}

#endif  // AL_TRANSPORT_I_RUNNABLE_HPP_
