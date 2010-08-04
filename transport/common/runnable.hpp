/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_RUNNABLE_HPP_
#define AL_MESSAGING_RUNNABLE_HPP_
#include <althread/altask.h>

namespace AL {
  namespace Transport {

    class Runnable: public AL::ALTask
    {
    public:
      virtual ~Runnable () {}

      virtual void run () = 0;
    };

  }
}

#endif /* !AL_MESSAGING_RUNNABLE_HPP_ */
