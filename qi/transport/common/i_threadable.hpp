/*
* i_threadable.h
*
*  Created on: Oct 8, 2009 at 10:47:09 AM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef AL_TRANSPORT_I_THREADABLE_HPP_
#define AL_TRANSPORT_I_THREADABLE_HPP_

namespace qi {
  namespace Transport {

    /// <summary>
    /// The threadable interface to ensure that the run () method is available.
    /// </summary>
    class IThreadable {
    public:
      virtual ~IThreadable () {}

      virtual void run () = 0;
    };

  }
}

#endif  // AL_TRANSPORT_I_THREADABLE_HPP_
