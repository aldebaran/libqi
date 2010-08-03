/*
 * alvalue_convertor.hpp
 *
 *  Created on: Oct 20, 2009 at 9:48:23 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_ALVALUECONVERTOR_HPP_
#define LIBIPPC_ALVALUECONVERTOR_HPP_

#include <list>
#include <vector>
#include <string>
#include <iostream>

#include <boost/variant.hpp>

#include <alvalue/alvalue.h>

namespace AL {
  namespace Messaging {

class VariableValue;

class ALValueConvertor : public boost::static_visitor<AL::ALValue> {
public:
  ALValueConvertor () {}

//  AL::ALValue operator () (int * p);
  AL::ALValue operator () (int i);
  AL::ALValue operator () (float f);
  AL::ALValue operator () (double d);
  AL::ALValue operator () (bool b);
  AL::ALValue operator () (const std::string & s);
  AL::ALValue operator () (const std::vector<unsigned char> & bin);
  //AL::ALValue operator () (const std::vector<float> & bin);
  AL::ALValue operator () (const std::vector<VariableValue> & v);

};

}
}

#endif /* !LIBIPPC_ALVALUECONVERTOR_HPP_ */
