/*
 * result_defintion.hpp
 *
 *  Created on: Oct 5, 2009 at 5:34:02 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_RESULTDEFINITION_HPP_
#define LIBIPPC_RESULTDEFINITION_HPP_

#include <alcommon-ng/collections/variables_list.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#ifdef WIN32
# include <alcommon-ng/win_stdint.hpp>
#else
# include <stdint.h>
#endif

#include <boost/serialization/access.hpp>

namespace AL {
  namespace Messaging {

/**
 * @brief A exception class definition used to define a result returned by a
 * remote procedure call.
 * This class and its attributes must be serializable in order to be sent to
 * the server.
 */
class ResultDefinition {
public:
  ResultDefinition ();
  ResultDefinition (const CallDefinition &def);
  ResultDefinition (uint32_t request_id, const VariableValue & val);
  bool operator==(const ResultDefinition& rhs) const;
  virtual ~ResultDefinition ();

  uint32_t getRequestId () const;
  void setRequestId (uint32_t id);

  void exception (const std::string & message);

  template <typename T>
  void value (const T & val);

  bool exceptionCaught () const;
  std::string exceptionMessage () const;
  const VariableValue & value () const;

private:
  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version);

private:
  /**
  * request_id same ID than call def
  */
  uint32_t request_id;

  /**
  * is exception during call
  */
  bool is_exception;

  /**
  * pcall -> ID
  * callVoid -> int 42 ?
  * call -> exception.what() or result
  */
  VariableValue v;
};

}
}

#include <alcommon-ng/serialization/result_definition.hxx>

#endif /* !LIBIPPC_RESULTDEFINITION_HPP_ */
