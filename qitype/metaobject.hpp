#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METAOBJECT_HPP_
#define _QITYPE_METAOBJECT_HPP_

#include <qitype/metamethod.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/metaproperty.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  class MetaObjectPrivate;
  /// Description of the signals and methods accessible on an ObjectType
  class QITYPE_API MetaObject  {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    int methodId(const std::string &name) const;
    int signalId(const std::string &name) const;
    int propertyId(const std::string& name) const;

    typedef std::map<unsigned int, MetaMethod> MethodMap;
    MethodMap methodMap() const;

    //not called signals because it conflict with Qt keywords :S
    typedef std::map<unsigned int, MetaSignal> SignalMap;
    SignalMap signalMap() const;

    typedef std::map<unsigned int, MetaProperty> PropertyMap;
    PropertyMap propertyMap() const;

    MetaMethod*       method(unsigned int id);
    const MetaMethod* method(unsigned int id) const;

    MetaSignal*       signal(unsigned int id);
    const MetaSignal* signal(unsigned int id) const;
    const MetaSignal* signal(const std::string &name) const;

    MetaProperty*       property(unsigned int id);
    const MetaProperty* property(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name) const;
    typedef std::pair<MetaMethod, float> CompatibleMethod;
    std::vector<CompatibleMethod> findCompatibleMethod(const std::string &nameOrSignature) const;

    ///Merge two MetaObject. Dest method and signal ids will be incremented by offset.
    static qi::MetaObject merge(const qi::MetaObject &source, const qi::MetaObject &dest);

    std::string description() const;

    MetaObjectPrivate   *_p;
    MetaObject(const MethodMap& methodMap, const SignalMap& signalMap,
      const PropertyMap& propertyMap, const std::string& description);
  };

  class MetaObjectBuilderPrivate;
  class QITYPE_API MetaObjectBuilder {
  public:
    MetaObjectBuilder();

    void setDescription(const std::string& desc);
    unsigned int addMethod(const qi::Signature &sigret,
                           const std::string &name,
                           const qi::Signature &signature,
                           int id = -1);
    unsigned int addMethod(MetaMethodBuilder& builder, int id = -1);
    unsigned int addSignal(const std::string &name, const qi::Signature& sig, int id = -1);
    unsigned int addProperty(const std::string& name, const qi::Signature& sig, int id = -1);
    qi::MetaObject metaObject();

  private:
    // C4251
    boost::shared_ptr<MetaObjectBuilderPrivate> _p;
  };

  namespace details {
    QITYPE_API void printMetaObject(std::ostream &stream, const qi::MetaObject &metaObject);
  }

}


#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_METAOBJECT_HPP_
