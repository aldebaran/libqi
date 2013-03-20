#ifndef _QITYPE_METAPROPERTY_HPP_
#define _QITYPE_METAPROPERTY_HPP_

#include <qitype/type.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi
{
  class QITYPE_API MetaProperty
  {
  public:
    MetaProperty(unsigned int uid, const std::string& name, const std::string& sig)
    : _uid(uid), _name(name), _signature(sig) {}
    MetaProperty() {}
    const std::string& name() const { return _name;}
    const std::string& signature() const { return _signature;}
    unsigned int uid() const { return _uid;}

  private:
    unsigned int _uid;
    std::string  _name;
    std::string  _signature;
    QI_TYPE_STRUCT_PRIVATE_ACCESS(MetaProperty);
  };
}

QI_TYPE_STRUCT(qi::MetaProperty, _uid, _name, _signature);

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif
