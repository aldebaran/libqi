/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/type.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/signature.hpp>
#include "metaobject_p.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <iomanip>

qiLogCategory("qitype.metaobject");

namespace qi {

  qi::Atomic<int> MetaObjectPrivate::uid = 1;

  MetaObjectPrivate::MetaObjectPrivate(const MetaObjectPrivate &rhs)
  {
    (*this) = rhs;
  }

  MetaObjectPrivate&  MetaObjectPrivate::operator=(const MetaObjectPrivate &rhs)
  {
    {
      boost::recursive_mutex::scoped_lock sl(rhs._methodsMutex);
      _methodsNameToIdx = rhs._methodsNameToIdx;
      _methods          = rhs._methods;
    }
    {
      boost::recursive_mutex::scoped_lock sl(rhs._eventsMutex);
      _eventsNameToIdx = rhs._eventsNameToIdx;
      _events = rhs._events;
    }
    {
      boost::recursive_mutex::scoped_lock sl(rhs._propertiesMutex);
      _properties = rhs._properties;
    }
    _index = rhs._index;
    _description = rhs._description;
    return (*this);
  }

  std::vector<qi::MetaMethod> MetaObjectPrivate::findMethod(const std::string &name)
  {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    std::vector<qi::MetaMethod>         ret;
    MetaObject::MethodMap::iterator     it;
    std::string                         cname(name);

    if (cname.find("::", 0, 2) == std::string::npos)
      cname += "::";

    for (it = _methods.begin(); it != _methods.end(); ++it) {
      qi::MetaMethod &mm = it->second;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }

  std::vector<MetaObject::CompatibleMethod> MetaObjectPrivate::findCompatibleMethod(const std::string &nameOrSignature)
  {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    std::vector<MetaObject::CompatibleMethod>         ret;
    MetaObject::MethodMap::iterator     it;
    std::string cname(nameOrSignature);

    //no signature specified fallback on findMethod
    if (cname.find(':') == std::string::npos)
    {
      std::vector<MetaMethod> r = findMethod(cname);
      ret.reserve(r.size());
      for (unsigned i=0; i<r.size(); ++i)
        ret.push_back(std::make_pair(r[i], 1.0f));
      return ret;
    }

    std::vector<std::string> sigsorig = qi::signatureSplit(nameOrSignature);
    if (sigsorig[1].empty())
      return ret;

    Signature sresolved(sigsorig[2]);

    for (it = _methods.begin(); it != _methods.end(); ++it) {
      const qi::MetaMethod& mm = it->second;

      if (sigsorig[1] != mm.name())
        continue;
      float score = sresolved.isConvertibleTo(Signature(mm.parametersSignature()));
      if (score)
        ret.push_back(std::make_pair(mm, score));
    }
    return ret;
  }

  std::vector<MetaSignal> MetaObjectPrivate::findSignal(const std::string &name)
  {
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);
    std::vector<MetaSignal>           ret;
    MetaObject::SignalMap::iterator it;
    std::string cname(name);
    cname += "::";

    for (it = _events.begin(); it != _events.end(); ++it) {
      MetaSignal &mm = it->second;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }

  int MetaObjectPrivate::addMethod(MetaMethodBuilder& builder, int uid) {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    unsigned int id;
    qi::MetaMethod method = builder.metaMethod();
    NameToIdx::iterator it = _methodsNameToIdx.find(method.signature());
    if (it != _methodsNameToIdx.end()) {
      qiLogWarning()
          << "Method("<< it->second << ") already defined (and reused): "
          << method.sigreturn() << " "
          << method.signature();
      return 0;
    }
    if (-1 < uid)
      id = uid;
    else
      id = ++_index;

    builder.setUid(id);
    _methods[id] = builder.metaMethod();
    _methodsNameToIdx[method.signature()] = id;
    // qiLogDebug() << "Adding method("<< id << "): " << sigret << " " << signature;
    return id;
  }

  int MetaObjectPrivate::addSignal(const std::string &sig, int uid) {
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);
    unsigned int id;
    NameToIdx::iterator it = _eventsNameToIdx.find(sig);
    if (it != _eventsNameToIdx.end()) {
      qiLogWarning() << "Signal("<< it->second << ") already defined (and reused): " << sig;
      return 0;
    }
    if (uid >= 0)
      id = uid;
    else
      id = ++_index;
    MetaSignal ms(id, sig);
    _events[id] = ms;
    _eventsNameToIdx[sig] = id;
    // qiLogDebug() << "Adding signal("<< id << "): " << sig;
    return id;
  }

  int MetaObjectPrivate::addProperty(const std::string& name, const std::string& sig, int id)
  {
    boost::recursive_mutex::scoped_lock sl(_propertiesMutex);
    for (MetaObject::PropertyMap::iterator it = _properties.begin(); it != _properties.end(); ++it)
    {
      if (it->second.name() == name)
      {
        qiLogWarning() << "Property already exists: " << name;
        return 0;
      }
    }
    if (id == -1)
      id = ++_index;
    _properties[id] = MetaProperty(id, name, sig);
    return id;
  }

  bool MetaObjectPrivate::addMethods(const MetaObject::MethodMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    MetaObject::MethodMap::const_iterator it;
    unsigned int newUid;

    for (it = mms.begin(); it != mms.end(); ++it) {
      newUid = it->second.uid();
      MetaObject::MethodMap::iterator jt = _methods.find(newUid);
      if (jt != _methods.end())
        return false;
      _methods[newUid] = qi::MetaMethod(newUid, it->second);
      _methodsNameToIdx[it->second.signature()] = newUid;
    }
    //todo: update uid
    return true;
  }

  bool MetaObjectPrivate::addSignals(const MetaObject::SignalMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);
    MetaObject::SignalMap::const_iterator it;
    unsigned int newUid;

    for (it = mms.begin(); it != mms.end(); ++it) {
      newUid = it->second.uid();
      MetaObject::SignalMap::iterator jt = _events.find(newUid);
      if (jt != _events.end())
        return false;
      _events[newUid] = qi::MetaSignal(newUid, it->second.signature());
      _eventsNameToIdx[it->second.signature()] = newUid;
    }
    //todo: update uid
    return true;
  }

  bool MetaObjectPrivate::addProperties(const MetaObject::PropertyMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_propertiesMutex);
    MetaObject::PropertyMap::const_iterator it;
    unsigned int newUid;

    for (it = mms.begin(); it != mms.end(); ++it) {
      newUid = it->second.uid();
      MetaObject::PropertyMap::iterator jt = _properties.find(newUid);
      if (jt != _properties.end())
        return false;
      _properties[newUid] = qi::MetaProperty(newUid, it->second.name(), it->second.signature());
    }
    //todo: update uid
    return true;
  }


  void MetaObjectPrivate::refreshCache()
  {
    unsigned int idx = 0;
    {
      boost::recursive_mutex::scoped_lock sl(_methodsMutex);
      _methodsNameToIdx.clear();
      for (MetaObject::MethodMap::iterator i = _methods.begin();
        i != _methods.end(); ++i)
      {
        _methodsNameToIdx[i->second.signature()] = i->second.uid();
        idx = std::max(idx, i->second.uid());
      }
    }
    {
      boost::recursive_mutex::scoped_lock sl(_eventsMutex);
      _eventsNameToIdx.clear();
      for (MetaObject::SignalMap::iterator i = _events.begin();
        i != _events.end(); ++i)
      {
        _eventsNameToIdx[i->second.signature()] = i->second.uid();
        idx = std::max(idx, i->second.uid());
      }
    }
    _index = idx;
  }

  void MetaObjectPrivate::setDescription(const std::string &desc) {
    _description = desc;
  }

  MetaObject::MetaObject()
  {
    _p = new MetaObjectPrivate();
  }

  MetaObject::MetaObject(const MetaObject &other)
  {
    _p = new MetaObjectPrivate();
    *_p = *(other._p);
  }

  MetaObject& MetaObject::operator=(const MetaObject &other)
  {
    *_p = *(other._p);
    return (*this);
  }

  MetaObject::~MetaObject()
  {
    delete _p;
  }

  MetaMethod *MetaObject::method(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_methodsMutex);
    MethodMap::iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return 0;
    return &i->second;
  }

  const MetaMethod *MetaObject::method(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_methodsMutex);
    MethodMap::const_iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return 0;
    return &i->second;
  }

  MetaSignal *MetaObject::signal(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_eventsMutex);
    SignalMap::iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return 0;
    return &i->second;
  }

  const MetaSignal *MetaObject::signal(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_eventsMutex);
    SignalMap::const_iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return 0;
    return &i->second;
  }

  MetaProperty *MetaObject::property(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_propertiesMutex);
    PropertyMap::iterator i = _p->_properties.find(id);
    if (i == _p->_properties.end())
      return 0;
    return &i->second;
  }

  const MetaProperty *MetaObject::property(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_propertiesMutex);
    PropertyMap::const_iterator i = _p->_properties.find(id);
    if (i == _p->_properties.end())
      return 0;
    return &i->second;
  }

  int MetaObject::methodId(const std::string &name) const
  {
    return _p->methodId(name);
  }

  int MetaObject::signalId(const std::string &name) const
  {
    return _p->signalId(name);
  }

  MetaObject::MethodMap MetaObject::methodMap() const {
    boost::recursive_mutex::scoped_lock sl(_p->_methodsMutex);
    return _p->_methods;
  }

  MetaObject::SignalMap MetaObject::signalMap() const {
    boost::recursive_mutex::scoped_lock sl(_p->_eventsMutex);
    return _p->_events;
  }

  MetaObject::PropertyMap MetaObject::propertyMap() const {
    boost::recursive_mutex::scoped_lock sl(_p->_propertiesMutex);
    return _p->_properties;
  }

  int MetaObject::propertyId(const std::string& name) const
  {
    boost::recursive_mutex::scoped_lock sl(_p->_propertiesMutex);
    for (PropertyMap::iterator it = _p->_properties.begin();
      it != _p->_properties.end(); ++it)
    {
      if (it->second.name() == name)
        return it->first;
    }
    return -1;
  }

  std::vector<qi::MetaMethod> MetaObject::findMethod(const std::string &name) const
  {
    return _p->findMethod(name);
  }

  std::vector<MetaObject::CompatibleMethod> MetaObject::findCompatibleMethod(const std::string &name) const
  {
    return _p->findCompatibleMethod(name);
  }

  std::vector<MetaSignal> MetaObject::findSignal(const std::string &name) const
  {
    return _p->findSignal(name);
  }

  qi::MetaObject MetaObject::merge(const qi::MetaObject &source, const qi::MetaObject &dest) {
    qi::MetaObject result = source;
    if (!result._p->addMethods(dest.methodMap()))
      qiLogError() << "cant merge metaobject (methods)";
    if (!result._p->addSignals(dest.signalMap()))
      qiLogError() << "cant merge metaobject (signals)";
    if (!result._p->addProperties(dest.propertyMap()))
      qiLogError() << "cant merge metaobject (properties)";
    result._p->setDescription(dest.description());
    return result;
  }

  std::string MetaObject::description() const {
    return _p->_description;
  }

  //MetaObjectBuilder
  class MetaObjectBuilderPrivate {
  public:
    qi::MetaObject metaObject;
  };

  MetaObjectBuilder::MetaObjectBuilder()
    : _p(new MetaObjectBuilderPrivate)
  {
  }

  unsigned int MetaObjectBuilder::addMethod(const std::string& sigret,
                                            const std::string& signature,
                                            int id)
  {
    MetaMethodBuilder mmb;
    mmb.setSigreturn(sigret);
    mmb.setSignature(signature);
    return _p->metaObject._p->addMethod(mmb, id);
  }

  unsigned int MetaObjectBuilder::addMethod(MetaMethodBuilder& builder, int id) {
    return _p->metaObject._p->addMethod(builder, id);
  }

  unsigned int MetaObjectBuilder::addSignal(const std::string &sig, int id) {
    return _p->metaObject._p->addSignal(sig, id);
  }

  unsigned int MetaObjectBuilder::addProperty(const std::string& name, const std::string& sig, int id)
  {
     return _p->metaObject._p->addProperty(name, sig, id);
  }

  qi::MetaObject MetaObjectBuilder::metaObject() {
    return _p->metaObject;
  }

  void MetaObjectBuilder::setDescription(const std::string &desc) {
    return _p->metaObject._p->setDescription(desc);
  }

}

namespace qi {
  namespace details {
    static int calcOffsetMethod(const qi::MetaObject::MethodMap &mmaps) {
      qi::MetaObject::MethodMap::const_iterator it;
      int max = 0;
      for (it = mmaps.begin(); it != mmaps.end(); ++it) {
        int cur = it->second.sigreturn().size();
        if (cur > max)
          max = cur;
      }
      return max;
    }

    void printMetaObject(std::ostream &stream, const qi::MetaObject &mobj) {
      qi::MetaObject::MethodMap methods = mobj.methodMap();
      qi::MetaObject::MethodMap::const_iterator itMM;
      qi::MetaMethodParameterVector::const_iterator itMMPV;

      int offset = calcOffsetMethod(methods) + 1;
      //int offset2 = calcOffsetMethodSig(methods) + 2;

      stream << "methods:" << std::endl;
      for (itMM = methods.begin(); itMM != methods.end(); ++itMM) {
        stream << "  " << std::right << std::setfill('0') << std::setw(3)
               << itMM->second.uid() << std::setw(0) << " " << std::left
               << itMM->second.sigreturn() << " " << itMM->second.signature() << std::endl;

        if (itMM->second.description() != "")
          stream << "    documentation: " << itMM->second.description() << std::endl;

        qi::MetaMethodParameterVector mmpVect = itMM->second.parameters();
        if (0 < mmpVect.size())
          stream << "    params:" << std::endl;
        for (itMMPV = mmpVect.begin(); itMMPV != mmpVect.end(); ++itMMPV) {
          stream << "        " << itMMPV->name() << ": " << itMMPV->description() << std::endl;
        }

        if (itMM->second.returnDescription() != "")
          stream << "    return: " << itMM->second.returnDescription() << std::endl;
      }
      stream << "events:" << std::endl;
      qi::MetaObject::SignalMap events = mobj.signalMap();
      qi::MetaObject::SignalMap::const_iterator it3;
      for (it3 = events.begin(); it3 != events.end(); ++it3)
      {
        stream << "  " << std::right << std::setfill('0') << std::setw(3) << it3->second.uid() << std::setw(0) << " "
               << std::left << std::setfill(' ') << std::setw(offset) << "" << std::setw(0)
               << " " << it3->second.signature() << std::endl;
      }
      stream <<"properties:" << std::endl;
      qi::MetaObject::PropertyMap props = mobj.propertyMap();
      for (qi::MetaObject::PropertyMap::const_iterator it = props.begin();
        it != props.end(); ++it)
      {
        stream << "  " << std::right << std::setfill('0') << std::setw(3) << it->second.uid() << std::setw(0) << " "
        << std::left << std::setfill(' ') << std::setw(offset) << "" << std::setw(0)
               << " " << it->second.name() << ' ' << it->second.signature() << std::endl;
      }
    }
  }
}

static qi::MetaObjectPrivate* metaObjectPrivate(qi::MetaObject* p) {
  return p->_p;
}


QI_TYPE_STRUCT_EX(qi::MetaObjectPrivate, ptr->refreshCache();, _methods, _events, _properties, _description);
QI_TYPE_REGISTER(::qi::MetaObjectPrivate);


QI_TYPE_STRUCT_BOUNCE_REGISTER(::qi::MetaObject, ::qi::MetaObjectPrivate, metaObjectPrivate);
