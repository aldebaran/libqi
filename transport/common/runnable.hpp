/*
 * runnable.h
 *
 *  Created on: Nov 9, 2009 at 5:27:58 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#ifndef LIBIPPC_RUNNABLE_HPP_
#define LIBIPPC_RUNNABLE_HPP_
#include <althread/altask.h>

namespace AL {
  namespace Messaging {

class Runnable: public AL::ALTask 
{
public:
  virtual ~Runnable () {}

  virtual void run () = 0;
};

}
}

#endif /* !LIBIPPC_RUNNABLE_HPP_ */
