#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METAOBJECT_HPP_
#define _QITYPE_METAOBJECT_HPP_

#include <qitype/metamethod.hpp>
#include <qitype/metasignal.hpp>

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

    typedef std::map<unsigned int, MetaMethod> MethodMap;
    MethodMap methodMap() const;

    //not called signals because it conflict with Qt keywords :S
    typedef std::map<unsigned int, MetaSignal> SignalMap;
    SignalMap signalMap() const;

    MetaMethod*       method(unsigned int id);
    const MetaMethod* method(unsigned int id) const;

    MetaSignal*       signal(unsigned int id);
    const MetaSignal* signal(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name) const;
    std::vector<MetaSignal> findSignal(const std::string &name) const;

    ///Merge two MetaObject. Dest method and signal ids will be incremented by offset.
    static qi::MetaObject merge(const qi::MetaObject &source, const qi::MetaObject &dest);

    MetaObjectPrivate   *_p;
  };

  class MetaObjectBuilderPrivate;
  class QITYPE_API MetaObjectBuilder {
  public:
    MetaObjectBuilder();

    unsigned int addMethod(const std::string& sigret, const std::string& signature, int id = -1);
    unsigned int addSignal(const std::string &sig, int id = -1);

    qi::MetaObject metaObject();

  private:
    boost::shared_ptr<MetaObjectBuilderPrivate> _p;
  };

  namespace details {
    QITYPE_API void printMetaObject(std::ostream &stream, const qi::MetaObject &metaObject);
  }

};

#endif  // _QITYPE_METAOBJECT_HPP_
