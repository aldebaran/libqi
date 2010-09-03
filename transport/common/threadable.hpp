/*
* threadable.h
*
*  Created on: Oct 8, 2009 at 10:47:09 AM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef AL_TRANSPORT_THREADABLE_HPP_
#define AL_TRANSPORT_THREADABLE_HPP_

namespace AL {
  namespace Transport {

    /// <summary>
    /// The threadable interface to ensure that the run () method is available.
    /// </summary>
    class Threadable {
    public:
      virtual ~Threadable () {}

      virtual void run () = 0;
    };

  }
}

#endif  // AL_TRANSPORT_THREADABLE_HPP_
