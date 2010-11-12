/*
 * print_visitor.hpp
 *
 *  Created on: Oct 20, 2009 at 9:48:23 AM
 *      Author: Jean-Charles DELAY
 *       Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef QI_MESSAGING_PRINTVISITOR_HPP_
#define QI_MESSAGING_PRINTVISITOR_HPP_

#include <list>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <boost/variant.hpp>

namespace qi {
  namespace messaging {

    class VariableValue;
    struct EmptyValue;

    class PrintVisitor : public boost::static_visitor<std::ostream &> {
    public:
      PrintVisitor (std::ostream & ostr) : ostr(ostr) {}

      //  std::ostream & operator () (int * p);
      std::ostream & operator () (int i);
      std::ostream & operator () (float f);
      std::ostream & operator () (double d);
      std::ostream & operator () (bool b);
      std::ostream & operator () (const std::string &s);
      std::ostream & operator () (const EmptyValue &s);
      std::ostream & operator () (const std::vector<unsigned char> &bin);
      std::ostream & operator () (const std::vector<VariableValue> &v);
      std::ostream & operator () (const std::map<std::string, std::string> &v);
    private:
      std::ostream & ostr;
    };

  }
}



#endif /* !QI_MESSAGING_PRINTVISITOR_HPP_ */
