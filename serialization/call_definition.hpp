/*
 * call_defintion.hpp
 *
 *  Created on: Oct 5, 2009 at 4:35:09 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_CALLDEFINITION_HPP_
#define LIBIPPC_CALLDEFINITION_HPP_

#include <alcommon-ng/collections/variables_list.hpp>

#include <string>
#include <boost/serialization/access.hpp>

#ifdef WIN32
# include <alcommon-ng/win_stdint.hpp>
#endif

namespace AL {
  namespace Messaging {

/**
 * @brief A basic call definition used to define a remote procedure call.
 * This class and its attributes must be serializable in order to be sent to
 * the server.
 * "client" class
 */
class CallDefinition {
public:
  CallDefinition ();
  CallDefinition (uint32_t request_id);
  virtual ~CallDefinition ();

  void setMethodName (const std::string & methodName);
  void setModuleName (const std::string & moduleName);
  void asResult (bool value);
  bool asResult () const;

  void isPCall (bool value);
  bool isPCall () const;

  void setSender (const std::string & sender);
  const std::string & getSender () const;

  template <typename T>
  void push (const T & value);

  int32_t getRequestId () const;
  void setRequestId (int32_t id);

  const std::string & getMethodName () const;
  const std::string & getModuleName () const;
  const VariablesList & getParameters () const;

private:
  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version);

private:

  /**
  * request_id method ID
  */
  int32_t request_id;

  /**
  * methodName
  */
  std::string methodName;

/**
  * moduleName empty = broker method
  */
  std::string moduleName;

    /**
  * list parameter
  */
  VariablesList list;

    /**
  * res or not (call void or not)
  */
  bool as_res;


  /**
  * pcall or not
  */
  bool is_pc;

  /**
  * broker to contact for result
  */
  std::string sender;
};

}
}

#include <alcommon-ng/serialization/call_definition.hxx>

#endif /* !LIBIPPC_CALLDEFINITION_HPP_ */
