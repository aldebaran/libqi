/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_METAMETHOD_HPP_
#define _QIMESSAGING_METAMETHOD_HPP_

#include <qimessaging/metafunction.hpp>
#include <qimessaging/method_type.hpp>


namespace qi {

  typedef boost::function<MetaFunctionResult(Value, const MetaFunctionParameters&)> MetaCallable;

  class QIMESSAGING_API MetaMethod {
  public:
    MetaMethod() {};
    MetaMethod(unsigned int uid, const std::string& sigret, const std::string& signature, MetaCallable value);
    MetaMethod(const std::string& name, unsigned int uid, MethodValue value);
    MetaMethod(unsigned int uid, const std::string& sigret, const std::string& signature, MetaFunction val);

    std::string signature() const;
    std::string sigreturn() const;
    const MetaCallable& functor() const;
    unsigned int       uid() const;

  protected:
    MetaCallable      _functor;
    unsigned int      _uid;
    std::string       _signature;
    std::string       _sigreturn;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaMethod &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaMethod &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaMethod &meta);

}; // namespace qi


#endif // __METAMETHOD_HPP__
