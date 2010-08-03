/*
 * result_info.hxx
 *
 *  Created on: Nov 4, 2009 at 12:02:04 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

namespace AL {
  namespace Messaging {

template <typename L>
void ResultInfo::wait_result (L & lock) {
  cond.wait(lock);
}

}
}
