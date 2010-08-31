/*
 * print_visitor.cpp
 *
 *  Created on: Oct 20, 2009 at 9:48:23 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/collections/print_visitor.hpp>
#include <alcommon-ng/collections/variables_list.hpp>

#ifndef foreach
# include <boost/foreach.hpp>
# define foreach  BOOST_FOREACH
#endif

namespace AL {
  namespace Messaging {

//std::ostream & PrintVisitor::operator () (int * p) {
//  return ostr << p;
//}

std::ostream & PrintVisitor::operator () (int i) {
  return ostr << i;
}

std::ostream & PrintVisitor::operator () (float f) {
  return ostr << f;
}

std::ostream & PrintVisitor::operator () (double d) {
  return ostr << d;
}

std::ostream & PrintVisitor::operator () (bool b) {
  return ostr << b;
}

std::ostream & PrintVisitor::operator () (const std::string & s) {
   return ostr << s;
}

std::ostream & PrintVisitor::operator () (const EmptyValue &s) {
   return ostr << "empty";
}

std::ostream & PrintVisitor::operator () (const std::vector<unsigned char> & bin) {
  ostr << "{ ";
  foreach (unsigned char c, bin)
    ostr << c << " ";
  return ostr << "}";
}

std::ostream & PrintVisitor::operator () (const std::vector<VariableValue> & v) {
  return ostr << "FIXME vector VariableValue";
  //ostr << "{ ";
  //foreach (VariableValue value, v)
  //  ostr << value << " ";
  //return ostr << "}";
}

std::ostream & PrintVisitor::operator () (const std::map<std::string, std::string> & v) {
  return ostr << "FIXME map std::string";
}

}
}
