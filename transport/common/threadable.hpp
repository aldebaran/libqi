/*
 * threadable.h
 *
 *  Created on: Oct 8, 2009 at 10:47:09 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_THREADABLE_HPP_
#define LIBIPPC_THREADABLE_HPP_

namespace AL {
  namespace Transport {

/**
 * @brief The threadable interface to ensure that the run () method is available.
 */
class Threadable {
public:
  virtual ~Threadable () {}

  virtual void run () = 0;
};

}
}

#endif /* !LIBIPPC_THREADABLE_HPP_ */
