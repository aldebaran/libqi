/*
 * alvalue_convertor.cpp
 *
 *  Created on: Oct 20, 2009 at 9:48:23 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alippc/collections/variables_list.hpp>
#include <alippc/collections/alvalue_convertor.hpp>

#ifndef foreach
# include <boost/foreach.hpp>
# define foreach  BOOST_FOREACH
#endif

namespace AL {
  namespace Messaging {

//AL::ALValue ALValueConvertor::operator () (int * p) {
//  if (!p)
//    return AL::ALValue();
//
//  return AL::ALValue(p);
//}

AL::ALValue ALValueConvertor::operator () (int i) {
  return AL::ALValue(i);
}

AL::ALValue ALValueConvertor::operator () (float f) {
  return AL::ALValue(f);
}

AL::ALValue ALValueConvertor::operator () (double d) {
  return AL::ALValue(d);
}

AL::ALValue ALValueConvertor::operator () (bool b) {
  return AL::ALValue(b);
}

AL::ALValue ALValueConvertor::operator () (const std::string & s) {
   return AL::ALValue(s);
}

AL::ALValue ALValueConvertor::operator () (const std::vector<unsigned char> & bin) {
  return AL::ALValue(bin);
}

/*AL::ALValue ALValueConvertor::operator () (const std::vector<float> & bin) {
  return AL::ALValue(bin);
}*/

AL::ALValue ALValueConvertor::operator () (const std::vector<VariableValue> & v) {
  AL::ALValue value;
  value.arraySetSize(v.size());
  unsigned int i = 0;
  foreach (VariableValue val, v)
    value[i++] = val.convertToALValue();

  return value;
}

}
}
