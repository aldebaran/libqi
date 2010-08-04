/*
 * variables_list.hpp
 *
 *  Created on: Oct 19, 2009 at 11:31:46 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_VARIABLESLIST_HPP_
#define LIBIPPC_VARIABLESLIST_HPP_

#include <alcommon-ng/collections/print_visitor.hpp>
#include <alcommon-ng/collections/alvalue_convertor.hpp>
#include <alcommon-ng/exceptions/exceptions.hpp>

#include <list>
#include <vector>
#include <string>
#include <typeinfo>

#include <boost/variant.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>

#ifndef foreach
# include <boost/foreach.hpp>
# define foreach  BOOST_FOREACH
#endif

#include <alvalue/alvalue.h>
#include <alcore/alptr.h>

namespace AL {
  namespace Messaging {

class VariableValue;

/** Boost.Variant which represent aldebaran's ALValue enum. */
typedef boost::variant<int,
                       float,
                       double,
                       bool,
                       std::string,
                       std::vector<unsigned char>,
                       //std::vector<float>,
                       std::vector<VariableValue>
                      > ValueType;


template <typename T>
class convert {
public:
	T as (const ValueType & value, bool empty) const {
	  (void) empty;
	    return boost::get<T,
	                      int,
										    float,
										    double,
										    bool,
										    std::string,
										    std::vector<unsigned char>,
                        //std::vector<float>,  // 6
										    std::vector<VariableValue>
										   > (value);
	}
};

template <>
class convert<AL::ALValue> {
public:
	AL::ALValue as (const ValueType & value, bool empty) const {
	    AL::ALValue v;
	    if (empty) {
	      return v;
	    }
	    ALValueConvertor convertor;
			return boost::apply_visitor(convertor, value);
	}
};

/** The generic serializable type to use to transmit parameters. */
class VariableValue {
public:
  VariableValue () : empty(true) {
  };

public:
  /** VariableValue ctor from an int */
  VariableValue(int i) : empty(false) { val = i; }
  /** VariableValue ctor from a float */
  VariableValue(float f) : empty(false) { val = f; }
  /** VariableValue ctor from a double */
  VariableValue(double d) : empty(false) { val = d; }
  /** VariableValue ctor from a bool */
  VariableValue(bool b) : empty(false) {
		val = b;
	}
  /** VariableValue ctor from a string */
  VariableValue(const std::string & s) : empty(false) { val = s; }
  /** VariableValue ctor from binary data */
  VariableValue(const std::vector<unsigned char> & bin) : empty(false) { val = bin; }

    // dhoussin
  //VariableValue(const std::vector<float> & bin) : empty(false) { val = bin; }


  /** VariableValue ctor from a vector of VariableValue */
  VariableValue(const std::vector<VariableValue> & v) : empty(false) { val = v; }
  /** VariableValue dtor */
  ~VariableValue() {}

  /** VariableValue ctor from a vector of R */
  template <typename R>
  VariableValue(const std::vector<R> & v) : empty(false) {
    std::vector<VariableValue> vec;
    foreach (VariableValue var, v)
      vec.push_back(var);
    val = vec;
	}

  VariableValue & operator = (const VariableValue & v) {
    this->val = v.val;
    this->empty = v.empty;
    return *this;
  }

  VariableValue & operator = (bool b) {
    this->empty = false;
    this->val = b;
    return *this;
  }

  /** VariableValue ctor from an alvalue */
  template <typename T>
  VariableValue(const AL::ALPtr<T> & ptr) : empty(false) {
    val = (void *) ptr.get();
  }

  /** VariableValue ctor from an alvalue */
  VariableValue(const AL::ALValue & value) : empty(false) {
    switch (value.getType()) {
    case (AL::ALValue::TypeArray):
		  {
	      std::vector<VariableValue> vec;
	      for (unsigned int i = 0; i < value.getSize(); ++i)
	        vec.push_back(value[i]);
	      val = vec;
	      break;
		  }
    case (AL::ALValue::TypeBool):
      val = value.Convert<bool>();
	    break;
    case (AL::ALValue::TypeInt):
      val = value.Convert<int>();
	    break;
    case (AL::ALValue::TypeFloat):
      val = value.Convert<float>();
	    break;
    case (AL::ALValue::TypeString):
      val = value.Convert<std::string>();
	    break;
    case (AL::ALValue::TypeBinary):
      val = value.Convert<std::vector<unsigned char> >();
	    break;
    case (AL::ALValue::TypeInvalid):
      empty = true;
	    break;
    default:
      break;
      throw Exception(std::string("Unsuported ALValue format "));
    }
	}

  /** Return the value under its original type. i.e. explicit conversion */
  template <typename T>
  T as () const {
    std::string name = typeid(T).name();
    convert<T> c;
    return c.as(val, empty);
  }

  /** Implicit conversion operator for void ptr */
//  operator void * () const { return as<int *>(); }
  /** Implicit conversion operator for int */
  operator int () const { return as<int>(); }
  /** Implicit conversion operator for float */
  operator float () const { return as<float>(); }
  /** Implicit conversion operator for double */
  operator double () const { return as<double>(); }
  /** Implicit conversion operator for bool */
  operator bool () const {
		return as<bool>();
	}
  /** Implicit conversion operator for binary data */
  operator std::vector<unsigned char> () const { return as<std::vector<unsigned char> >(); }

  //operator std::vector<float> () const { return as<std::vector<float> >(); }
  /** Implicit conversion operator for array */
  operator std::vector<VariableValue> () const { return as<std::vector<VariableValue> >(); }

  /** Return the value as an VariableValue */
  const ValueType & value () const { return val; }

  AL::ALValue convertToALValue () const {
    AL::ALValue v;
    if (empty) {
      return v;
    }
    ALValueConvertor convertor;
		return boost::apply_visitor(convertor, val);
  }

  bool isEmpty () {
    return empty;
  }

private:
  ValueType val;
  bool empty;

private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize (Archive & ar, unsigned int version) {
    (void) version;
    ar & boost::serialization::make_nvp("val", val);
    ar & boost::serialization::make_nvp("empty", empty);
  }
};

/**
 * A list which can contain any type of VariableValue enum.
 * Used to transmit parameters for calls.
 */
class VariablesList : public std::list<VariableValue> {
public:
  VariablesList () {}

  AL::ALValue convertToALValue () const {
    AL::ALValue alvalue;
    alvalue.arraySetSize(this->size());
    unsigned int i = 0;
    foreach (VariableValue v, *this) {
      alvalue[i++] = v.convertToALValue();
    }

    return alvalue;
  }

private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize (Archive & ar, unsigned int version) {
    (void) version;
    std::list<VariableValue> * l = this;
    ar & boost::serialization::make_nvp("list", *l);
  }
};

}
}

/**
 * Print the value in ostr stream.
 * @param ostr The stream to send the printed value in.
 * @param value The value to print.
 * @return ostr.
 */
inline std::ostream & operator << (std::ostream & ostr, const AL::Messaging::VariableValue & value) {
	AL::Messaging::PrintVisitor visitor(ostr);
	return boost::apply_visitor(visitor, value.value());
}

#endif /* !LIBIPPC_VARIABLESLIST_HPP_ */
