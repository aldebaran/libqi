/*
 * variables_list.hpp
 *
 *  Created on: Oct 19, 2009 at 11:31:46 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_VARIABLESLIST_HPP_
#define LIBIPPC_VARIABLESLIST_HPP_

#include <alcommon-ng/collections/equality_visitor.hpp>

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
#include <boost/type_traits.hpp>

namespace AL {
  namespace Messaging {

    class VariableValue;

    typedef boost::variant<bool,
                           int,
                           float,
                           double,
                           std::string,
                           std::vector<unsigned char>,
                           std::vector<VariableValue>
                           > ValueType;


    template <typename T>
    struct convert {
      T as (const ValueType& value, bool _empty) const {
        (void) _empty;
        return boost::get<T, bool, int, float, double, std::string, std::vector<unsigned char>, std::vector<VariableValue> > (value);
      }
    };


    /** The generic serializable type to use to transmit parameters. */
    class VariableValue {
    public:
      VariableValue ()        : _empty(true)  {};
      VariableValue(bool b)   : _empty(false) { val = b; }
      VariableValue(int i)    : _empty(false) { val = i; }
      VariableValue(float f)  : _empty(false) { val = f; }
      VariableValue(double d) : _empty(false) { val = d; }
      VariableValue(const std::string & s)                  : _empty(false) { val = s; }
      VariableValue(const std::vector<unsigned char> & bin) : _empty(false) { val = bin; }
      VariableValue(const std::vector<VariableValue> & v)   : _empty(false) { val = v; }

      ~VariableValue() {}

      VariableValue & operator= (const VariableValue & v) {
        this->val    = v.val;
        this->_empty = v._empty;
        return *this;
      }

      bool operator== (const VariableValue& rhs) const {
        return boost::apply_visitor(EqualityVisitor(), this->val, rhs.val);
      }

      //todo: optimise: we want const ref
      template <typename T>
      T as () const {
        return convert<T>().as(val, _empty);
      }

      operator bool                       () const { return as<bool>(); }
      operator int                        () const { return as<int>(); }
      operator float                      () const { return as<float>(); }
      operator double                     () const { return as<double>(); }
      operator std::string                () const { return as<std::string>(); }
      operator std::vector<unsigned char> () const { return as<std::vector<unsigned char> >(); }
      operator std::vector<VariableValue> () const { return as<std::vector<VariableValue> >(); }

      /** Return the value as an VariableValue */
      const ValueType & value() const    {  return val; }
      void value(const ValueType & pval) { val = pval; }

      const bool &empty ()const          { return _empty; }
      void empty(const bool &isempty)    { _empty = isempty; }

    private:
      ValueType val;
      bool      _empty;

    private:
      friend class boost::serialization::access;

      template <class Archive>
          void serialize (Archive & ar, unsigned int version) {
        (void) version;
        ar & boost::serialization::make_nvp("val", val);
        ar & boost::serialization::make_nvp("empty", _empty);
      }
    };

    typedef std::vector<VariableValue> ArgumentList;
    typedef VariableValue              ReturnValue;
  }
}

//TODO:
//namespace boost {
//  namespace serialization {
//    template<class Archive>
//    void serialize(Archive &ar, AL::Messaging::VariableValue &varval, const unsigned int version) {
//        ar & boost::serialization::make_nvp("empty", varval.empty());
//        //ar & boost::serialization::make_nvp("list",  varval.value());
//      return;
//    }
//  }
//}

#include <alcommon-ng/collections/print_visitor.hpp>
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
