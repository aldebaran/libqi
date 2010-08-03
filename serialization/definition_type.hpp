/*
 * definition_type.hpp
 *
 *  Created on: Nov 4, 2009 at 3:07:48 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#ifndef LIBIPPC_DEFINITIONTYPE_HPP_
#define LIBIPPC_DEFINITIONTYPE_HPP_

namespace AL {
  namespace Messaging {

enum DefinitionType {
  TypeCall = 1 << 0,
  TypeResult = 1 << 1,
  TypeException = 1 << 2
};

}
}

#endif /* !LIBIPPC_DEFINITIONTYPE_HPP_ */
