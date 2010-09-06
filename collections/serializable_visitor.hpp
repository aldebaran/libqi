/*
 * serializable_visitor.hpp
 *
 *  Created on: Oct 20, 2009 at 9:47:36 AM
 *      Author: Jean-Charles DELAY
 *       Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_SERIALIZABLEVISITOR_HPP_
#define LIBIPPC_SERIALIZABLEVISITOR_HPP_

#include <alcommon-ng/collections/variables_list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/variant.hpp>

namespace AL {
  namespace Messaging {

template <class Archive>
class SerializableVisitor : public boost::static_visitor<> {
public:
  SerializableVisitor (Archive & ar, unsigned int version) : ar(ar), version(version) {}
  ~SerializableVisitor () {}

  void operator() (int * p) { ar & (int) p; }
  void operator() (int i) { ar & i; }
  void operator() (float f) { ar & f; }
  void operator() (double d) { ar & d; }
  void operator() (bool b) { ar & b; }
  void operator() (std::string & s) { ar & s; }
  void operator() (std::vector<unsigned char> & bin) { ar & bin; }
  void operator() (std::vector<VariableValue> & v) { ar & v; }
  void operator() (std::map<std::string, std::string> & v) { ar & v; }

private:
  Archive & ar;
  unsigned int version;
};

}
}

#endif /* !LIBIPPCSERIALIZABLEVISITOR_HPP_ */
