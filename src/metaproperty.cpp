#include <qitype/metaproperty.hpp>
#include <qitype/anyobject.hpp>

namespace qi
{
  MetaProperty::MetaProperty(unsigned int uid, const std::string& name, const qi::Signature& sig)
    : _uid(uid), _name(name), _signature(sig)
  {}

  MetaProperty::MetaProperty()
  {}

  const std::string& MetaProperty::name() const
  {
    return _name;
  }
  const qi::Signature& MetaProperty::signature() const
  {
    return _signature;
  }

  std::string MetaProperty::toString() const
  {
    return _name + "::" + _signature.toString();
  }

  unsigned int MetaProperty::uid() const
  {
    return _uid;
  }

  bool MetaProperty::isPrivate() const
  {
    return MetaObject::isPrivateMember(name(), uid());
  }
}
