/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/typeinterface.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/signature.hpp>
#include "metaobject_p.hpp"
#include "metamethod_p.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <qi/iocolor.hpp>
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
    // cache data uses pointers to map entries and must be refreshed
    refreshCache();
    return (*this);
  }

  std::vector<qi::MetaMethod> MetaObjectPrivate::findMethod(const std::string &name) const
  {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    std::vector<qi::MetaMethod>         ret;
    MetaObject::MethodMap::const_iterator     it;

    for (it = _methods.begin(); it != _methods.end(); ++it) {
      const qi::MetaMethod &mm = it->second;
      if (mm.name() == name)
        ret.push_back(mm);
    }
    return ret;
  }

  int MetaObject::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args, bool* canCache) const
  {
    return _p->findMethod(nameWithOptionalSignature, args, canCache);
  }

  std::string MetaObjectPrivate::generateErrorString(
    const std::string& signature,
    const std::vector<std::pair<MetaMethod, float> >& candidates,
    bool logError)
  {
    std::stringstream                           ss;
    std::vector<std::pair<MetaMethod, float> >::const_iterator it;

    if (candidates.size() == 0) {
      ss << "Can't find method: " << signature << std::endl;
      return ss.str();
    }

    if (candidates.size() == 1) {
      ss << "Arguments types did not match for " << signature << ":" << std::endl
         << "  Candidate:" << std::endl;
    } else {
      ss << "Ambiguous overload for " << signature << ":" << std::endl
         << "  Candidate(s):" << std::endl;
    }
    for (it = candidates.begin(); it != candidates.end(); ++it) {
      const qi::MetaMethod       &mm = it->first;
      ss << "  " << mm.toString() << " (" << it->second << ')' << std::endl;
    }
    if (logError)
      qiLogError() << ss.str();
    return ss.str();
  }

  struct less_pair_second
  {
    template<typename T> bool operator()(const T& a, const T& b) const
    {
      return a.second < b.second;
    }
  };

  int MetaObjectPrivate::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args, bool* canCache) const
  {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    if (_dirtyCache)
      const_cast<MetaObjectPrivate*>(this)->refreshCache();
    MetaObject::MethodMap::const_iterator it;
    if (nameWithOptionalSignature.find(':') != nameWithOptionalSignature.npos)
    { // full name and signature was given, there can be only one match
      if (canCache)
        *canCache = true;
      NameToIdx::const_iterator itRev =  _methodsNameToIdx.find(nameWithOptionalSignature);
      if (itRev == _methodsNameToIdx.end())
        return -1;
      else
        return itRev->second;
    }
    // Only name given, try to find an unique match with given argument count
    OverloadMap::const_iterator overloadIt = _methodNameToOverload.find(nameWithOptionalSignature);
    if (overloadIt == _methodNameToOverload.end())
    { // no match for the name, no chance
      if (canCache)
        *canCache = true;
      return -1;
    }
    MetaMethod* firstMatch = 0;
    bool ambiguous = false;
    size_t nargs = args.size();
    for (MetaMethod* mm = overloadIt->second; mm; mm=mm->_p->next)
    {
      assert(mm->name() == nameWithOptionalSignature);
      const Signature& sig = mm->parametersSignature();
      if (sig == "m" || sig.children().size() == nargs)
      {
        if (firstMatch)
        { // this is the second match, ambiguity that needs args to resolve
          ambiguous = true;
          break;
        }
        else
        {
          firstMatch = mm;
          // go on to check for more matches
        }
      }
    }
    if (canCache)
      *canCache = !ambiguous || !firstMatch;
    if (!firstMatch)
      return -1; // no match
    if (!ambiguous)
      return firstMatch->uid();
    // resolve ambiguity by using arguments
    for (unsigned dyn = 0; dyn<2; ++dyn)
    {
      Signature sResolved = args.signature(dyn==1);
      std::string resolvedSig = sResolved.toString();
      std::string fullSig = nameWithOptionalSignature + "::" + resolvedSig;
      qiLogDebug() << "Finding method for resolved signature " << fullSig;
      // First try an exact match, which is much faster if we're lucky.
      NameToIdx::const_iterator itRev =  _methodsNameToIdx.find(nameWithOptionalSignature);
      if (itRev != _methodsNameToIdx.end())
        return itRev->second;
      typedef std::vector<std::pair<MetaMethod, float> > Methods;

      typedef std::vector<std::pair<const MetaMethod*, float> > MethodsPtr;
      MethodsPtr mml;

      // embed findCompatibleMethod
      for (MetaMethod* mm = overloadIt->second; mm; mm=mm->_p->next)
      { // still suboptimal, we are rescanning all overloads regardless of arg count
        float score = sResolved.isConvertibleTo(mm->parametersSignature());
        if (score)
          mml.push_back(std::make_pair(mm, score));
      }

      if (mml.empty())
        continue;
      if (mml.size() == 1)
        return mml.front().first->uid();

      // get best match
      MethodsPtr::iterator it = std::max_element(mml.begin(), mml.end(), less_pair_second());
      int count = 0;
      for (unsigned i=0; i<mml.size(); ++i)
      {
        if (mml[i].second == it->second)
          ++count;
      }
      assert(count);
      if (count > 1)
        qiLogVerbose() << generateErrorString(fullSig, const_cast<MetaObjectPrivate*>(this)->findCompatibleMethod(nameWithOptionalSignature), false);
      else
        return it->first->uid();
    }
    return -1;
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

  MetaSignal* MetaObjectPrivate::signal(const std::string &name)
  {
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);
    int id = signalId(name);
    if (id < 0)
      return 0;
    else
      return &_events[id];
  }

  unsigned int MetaObjectPrivate::addMethod(MetaMethodBuilder& builder, int uid) {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    qi::MetaMethod method = builder.metaMethod();
    NameToIdx::iterator it = _methodsNameToIdx.find(method.toString());
    if (it != _methodsNameToIdx.end()) {
      qiLogWarning() << "Method("<< it->second << ") already defined (and overriden): "
                     << method.toString();
      return it->second;
    }

    if (uid == -1)
      uid = ++_index;
    builder.setUid(uid);
    _methods[uid] = builder.metaMethod();
    _methodsNameToIdx[method.toString()] = uid;
    _dirtyCache = true;
    return uid;
  }

  unsigned int MetaObjectPrivate::addSignal(const std::string &name, const Signature &signature, int uid) {
#ifndef NDEBUG
    std::vector<std::string> split = signatureSplit(name);
    if (name != split[1])
      throw std::runtime_error("Unexpected full signature " + name);
#endif
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);
    NameToIdx::iterator it = _eventsNameToIdx.find(name);
    if (it != _eventsNameToIdx.end()) {
      MetaSignal &ms = _events[it->second];
      qiLogWarning() << "Signal("<< it->second << ") already defined (and overriden): " << ms.toString() << "instead of requested: " << name;
      return it->second;
    }

    if (uid == -1)
      uid = ++_index;
    MetaSignal ms(uid, name, signature);
    _events[uid] = ms;
    _eventsNameToIdx[name] = uid;
    _dirtyCache = true;
    return uid;
  }

  unsigned int MetaObjectPrivate::addProperty(const std::string& name, const qi::Signature& sig, int id)
  {
    boost::recursive_mutex::scoped_lock sl(_propertiesMutex);
    for (MetaObject::PropertyMap::iterator it = _properties.begin(); it != _properties.end(); ++it)
    {
      if (it->second.name() == name)
      {
        qiLogWarning() << "Property already exists: " << name;
        return it->second.uid() ;
      }
    }
    if (id == -1)
      id = ++_index;
    _properties[id] = MetaProperty(id, name, sig);
    _dirtyCache = true;
    return id;
  }

  bool MetaObjectPrivate::addMethods(const MetaObject::MethodMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    MetaObject::MethodMap::const_iterator it;
    unsigned int newUid;

    for (it = mms.begin(); it != mms.end(); ++it) {
      newUid = it->second.uid();
      MetaObject::MethodMap::iterator jt = _methods.find(newUid);
      //same id and same signature: we dont mind.
      if (jt != _methods.end()) {
        if ((jt->second.toString() != it->second.toString()) ||
            (jt->second.returnSignature() != it->second.returnSignature()))
          return false;
      }
      _methods[newUid] = qi::MetaMethod(newUid, it->second);
      _methodsNameToIdx[it->second.toString()] = newUid;
    }
    _dirtyCache = true;
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
      if (jt != _events.end()) {
        if ((jt->second.toString() != it->second.toString()))
          return false;
      }
      _events[newUid] = qi::MetaSignal(newUid, it->second.name(), it->second.parametersSignature());
      _eventsNameToIdx[it->second.name()] = newUid;
    }
    _dirtyCache = true;
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
      if (jt != _properties.end()) {
        if ((jt->second.toString() != it->second.toString()))
          return false;
      }
      _properties[newUid] = qi::MetaProperty(newUid, it->second.name(), it->second.signature());
    }
    _dirtyCache = true;
    //todo: update uid
    return true;
  }


  void MetaObjectPrivate::refreshCache()
  {
    // Both change on property(=event) and method will invalidate the cache.
    boost::recursive_mutex::scoped_lock ml(_methodsMutex);
    boost::recursive_mutex::scoped_lock el(_eventsMutex);
    unsigned int idx = 0;
    {
      _methodsNameToIdx.clear();
      _methodNameToOverload.clear();
      for (MetaObject::MethodMap::iterator i = _methods.begin();
        i != _methods.end(); ++i)
      {
        _methodsNameToIdx[i->second.toString()] = i->second.uid();
        idx = std::max(idx, i->second.uid());
        OverloadMap::iterator overloadIt = _methodNameToOverload.find(i->second.name());
        if (overloadIt == _methodNameToOverload.end())
        {
          _methodNameToOverload[i->second.name()] = &i->second;
          i->second._p->next = 0;
        }
        else
        { // push_front
          i->second._p->next =  overloadIt->second;
          overloadIt->second = &i->second;
        }
      }
    }
    {
      _eventsNameToIdx.clear();
      for (MetaObject::SignalMap::iterator i = _events.begin();
        i != _events.end(); ++i)
      {
        _eventsNameToIdx[i->second.name()] = i->second.uid();
        idx = std::max(idx, i->second.uid());
      }
    }
    // never lower index
    _index = std::max(idx, *_index);
    _dirtyCache = false;
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

  bool MetaObject::isPrivateMember(const std::string &name, unsigned int uid)
  {
    return uid < qiObjectSpecialMemberMaxUid
        || (name.size() > 0 && name[0] == '_');
  }

  const MetaSignal* MetaObject::signal(const std::string &name) const
  {
    return _p->signal(name);
  }

  qi::MetaObject MetaObject::merge(const qi::MetaObject &source, const qi::MetaObject &dest) {
    qi::MetaObject result = source;
    if (!result._p->addMethods(dest.methodMap()))
      qiLogError() << "can't merge metaobject (methods)";
    if (!result._p->addSignals(dest.signalMap()))
      qiLogError() << "can't merge metaobject (signals)";
    if (!result._p->addProperties(dest.propertyMap()))
      qiLogError() << "can't merge metaobject (properties)";
    result._p->setDescription(dest.description());
    result._p->refreshCache();
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

  unsigned int MetaObjectBuilder::addMethod(const qi::Signature& sigret,
                                            const std::string& name,
                                            const qi::Signature& signature,
                                            int id)
  {
    MetaMethodBuilder mmb;
    mmb.setReturnSignature(sigret);
    mmb.setName(name);
    mmb.setParametersSignature(signature);
    return _p->metaObject._p->addMethod(mmb, id);
  }

  unsigned int MetaObjectBuilder::addMethod(MetaMethodBuilder& builder, int id) {
    return _p->metaObject._p->addMethod(builder, id);
  }

  unsigned int MetaObjectBuilder::addSignal(const std::string &name, const qi::Signature& sig, int id) {
    return _p->metaObject._p->addSignal(name, sig, id);
  }

  unsigned int MetaObjectBuilder::addProperty(const std::string& name, const qi::Signature& sig, int id)
  {
     return _p->metaObject._p->addProperty(name, sig, id);
  }

  qi::MetaObject MetaObjectBuilder::metaObject() {
    _p->metaObject._p->refreshCache();
    return _p->metaObject;
  }

  void MetaObjectBuilder::setDescription(const std::string &desc) {
    return _p->metaObject._p->setDescription(desc);
  }

}

namespace qi {
  namespace details {

    static bool bypass(const std::string &name, unsigned int uid, bool showHidden) {
      if (showHidden)
        return false;
      if (MetaObject::isPrivateMember(name, uid))
        return true;
      return false;
    }

    template <typename T>
    static int calcOffset(const T &mmaps, bool showHidden) {
      typename T::const_iterator it;
      int max = 0;
      for (it = mmaps.begin(); it != mmaps.end(); ++it) {
        if (bypass(it->second.name(), it->second.uid(), showHidden))
          continue;
        int cur = it->second.name().size();
        if (cur > max)
          max = cur;
      }
      return max;
    }

    static StreamColor FC(StreamColor col, bool enable) {
      if (enable)
        return col;
      return StreamColor_None;
    }

    static void printCat(std::ostream &stream, bool color, const std::string &cat) {
      stream << FC(StreamColor_Green, color) << "  * " << FC(StreamColor_Fuchsia, color) << cat << FC(StreamColor_Reset, color) << ":" << std::endl;
    }

    static void printIdName(std::ostream &stream, bool color, int offset, unsigned int id, const std::string &name) {
      stream << "   "
             << FC(StreamColor_Blue, color)
             << std::right << std::setfill('0') << std::setw(3) << id
             << FC(StreamColor_Reset, color)
             << std::left << std::setw(0) << std::setfill(' ')
             << " " << std::setw(offset) << name << std::setw(0);
    }

    void printMetaObject(std::ostream &stream, const qi::MetaObject &mobj, bool color, bool showHidden, bool showDoc, bool raw, bool parseable) {
      qi::MetaObject::MethodMap   methods = mobj.methodMap();
      qi::MetaObject::SignalMap   events = mobj.signalMap();
      qi::MetaObject::PropertyMap props = mobj.propertyMap();

      int offsetProps = std::min(calcOffset(props, showHidden), 30);
      int offsetSigs  = std::min(calcOffset(events, showHidden), 30);
      int offsetMeth  = std::min(calcOffset(methods, showHidden), 30);

      if (parseable)
      {
        stream << ":";
      }
      else if (methods.size())
      {
        printCat(stream, color, "Methods");
      }
      qi::MetaObject::MethodMap::const_iterator itMM;
      qi::MetaMethodParameterVector::const_iterator itMMPV;
      std::string comma = "";
      for (itMM = methods.begin(); itMM != methods.end(); ++itMM) {
        if (bypass(itMM->second.name(), itMM->second.uid(), showHidden))
          continue;
        if (parseable)
        {
          stream << comma << itMM->second.name();
          comma = ",";
          continue;
        }
        printIdName(stream, color, offsetMeth, itMM->second.uid(), itMM->second.name());
        if (raw)
          stream << " " << FC(StreamColor_Blue, color) << itMM->second.returnSignature().toString() << FC(StreamColor_Reset, color)
                 << " " << FC(StreamColor_Yellow, color) << itMM->second.parametersSignature().toString() << FC(StreamColor_Reset, color)
                 << std::endl;
        else
          stream << " " << FC(StreamColor_Blue, color) << itMM->second.returnSignature().toPrettySignature() << FC(StreamColor_Reset, color)
                 << " " << FC(StreamColor_Yellow, color) << itMM->second.parametersSignature().toPrettySignature() << FC(StreamColor_Reset, color)
                 << std::endl;
        if (!showDoc)
          continue;
        if (itMM->second.description() != "")
          stream << "       " << FC(StreamColor_DarkGreen, color) << itMM->second.description() << FC(StreamColor_Reset, color) << std::endl;

        qi::MetaMethodParameterVector mmpVect = itMM->second.parameters();
        for (itMMPV = mmpVect.begin(); itMMPV != mmpVect.end(); ++itMMPV) {
          stream << "       " << FC(StreamColor_Brown, color) << itMMPV->name() << ": "
                 << FC(StreamColor_DarkGreen, color) << itMMPV->description() << FC(StreamColor_Reset, color)
                 << std::endl;
        }

        if (itMM->second.returnDescription() != "")
          stream << FC(StreamColor_Brown, color) << "       return: " << FC(StreamColor_DarkGreen, color) << itMM->second.returnDescription() << FC(StreamColor_Reset, color)
                 << std::endl;
      }

      if (parseable)
      {
        stream << ":";
        comma = "";
      }
      else if (events.size())
      {
        printCat(stream, color, "Signals");
      }
      qi::MetaObject::SignalMap::const_iterator it3;
      for (it3 = events.begin(); it3 != events.end(); ++it3)
      {
        if (bypass(it3->second.name(), it3->second.uid(), showHidden))
          continue;
        if (parseable)
        {
          stream << comma << it3->second.name();
          comma = ",";
          continue;
        }
        printIdName(stream, color, offsetSigs, it3->second.uid(), it3->second.name());
        stream << " " << FC(StreamColor_Yellow, color) << it3->second.parametersSignature().toPrettySignature() << FC(StreamColor_Reset, color)
               << std::endl;
      }

      if (parseable)
      {
        stream << ":";
        comma = "";
      }
      else if (props.size())
      {
        printCat(stream, color, "Properties");
      }
      for (qi::MetaObject::PropertyMap::const_iterator it = props.begin();
        it != props.end(); ++it)
      {
        if (bypass(it->second.name(), it->second.uid(), showHidden))
          continue;
        if (parseable)
        {
          stream << comma << it->second.name();
          comma = ",";
          continue;
        }
        printIdName(stream, color, offsetProps, it->second.uid(), it->second.name());
        stream << " " << FC(StreamColor_Yellow, color) << it->second.signature().toPrettySignature() << FC(StreamColor_Reset, color)
               << std::endl;
      }
      if (parseable)
        stream << std::endl;
    }
  }


  MetaObject::MetaObject(const MethodMap& methodMap, const SignalMap& signalMap,
    const PropertyMap& propertyMap, const std::string& description)
  {
    _p = new MetaObjectPrivate();
    _p->_methods = methodMap;
    _p->_events = signalMap;
    _p->_properties = propertyMap;
    _p->_description = description;
    _p->refreshCache();
  }

}

namespace {
  static const qi::MetaObject::MethodMap& methodMap(qi::MetaObject* ptr)
  {
    return ptr->_p->_methods;
  }
  static const qi::MetaObject::SignalMap& signalMap(qi::MetaObject* ptr)
  {
    return ptr->_p->_events;
  }
  static const qi::MetaObject::PropertyMap& propertyMap(qi::MetaObject* ptr)
  {
    return ptr->_p->_properties;
  }
  static const std::string& description(qi::MetaObject* ptr)
  {
    return ptr->_p->_description;
  }
}

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(::qi::MetaObject,
  QI_STRUCT_HELPER("methods", methodMap),
  QI_STRUCT_HELPER("signals", signalMap),
  QI_STRUCT_HELPER("properties", propertyMap),
  QI_STRUCT_HELPER("description", description));
