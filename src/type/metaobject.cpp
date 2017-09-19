/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/type/typeinterface.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/signature.hpp>
#include "metaobject_p.hpp"
#include "metamethod_p.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <qi/iocolor.hpp>
#include <qi/detail/print.hpp>
#include <iomanip>

qiLogCategory("qitype.metaobject");

namespace qi {

qi::Atomic<int> MetaObjectPrivate::uid{1};

  MetaObjectPrivate::MetaObjectPrivate(const MetaObjectPrivate &rhs)
  {
    (*this) = rhs;
  }

  MetaObjectPrivate&  MetaObjectPrivate::operator=(const MetaObjectPrivate &rhs)
  {
    if (this == &rhs)
      return *this;

    {
      boost::recursive_mutex::scoped_lock sl(rhs._methodsMutex);
      _objectNameToIdx = rhs._objectNameToIdx;
      _methods          = rhs._methods;
    }
    {
      boost::recursive_mutex::scoped_lock sl(rhs._eventsMutex);
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

    for (const auto& method: _methods) {
      const qi::MetaMethod &mm = method.second;
      if (mm.name() == name)
        ret.push_back(mm);
    }
    return ret;
  }

  int MetaObject::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args, bool* canCache) const
  {
    return _p->findMethod(nameWithOptionalSignature, args, canCache);
  }

  static void displayCandidates(std::stringstream& ss, const std::vector<std::pair<MetaMethod, float> >& candidates) {
    if (candidates.empty())
      return;

    if (candidates.size() == 1) {
      ss << "  Candidate:" << std::endl;
    } else {
      ss << "  Candidates:" << std::endl;
    }
    for(const auto& candidate: candidates) {
      ss << "  " << candidate.first.toString() << " (" << candidate.second << ')' << std::endl;
    }
  }

  static void displayMeths(std::stringstream& ss, const std::vector<MetaMethod>& candidates) {
    if (candidates.empty())
      return;

    if (candidates.size() == 1) {
      ss << "  Candidate:" << std::endl;
    } else {
      ss << "  Candidates:" << std::endl;
    }
    for (const auto& candidate: candidates) {
      ss << "  " << candidate.toString() << std::endl;
    }
  }



  std::string MetaObjectPrivate::generateErrorString(const std::string& signature,
                                                     const std::string& resolvedSignature,
                                                     const std::vector<std::pair<MetaMethod, float> >& candidates,
                                                     int error,
                                                     bool logError) const
  {
    std::stringstream ss;

    if (error == -1 && candidates.size() != 0) {
      qiLogError() << "Broken error handling in generateErrorString";
      logError = 1;
    }
    switch (error) {
      case -1: {
        ss << "Can't find method: " << signature << " (resolved to '" << resolvedSignature << "')" << std::endl;
        std::vector<MetaMethod> mmv = findMethod(qi::signatureSplit(signature)[1]);
        displayMeths(ss, mmv);
        break;
      }
      case -2:
        ss << "Arguments types did not match for " << signature  << " (resolved to '" << resolvedSignature << "')" << ":" << std::endl;
        displayCandidates(ss, candidates);
        break;
      case -3:
        ss << "Ambiguous overload for " << signature  << " (resolved to '" << resolvedSignature << "')" << ":" << std::endl;
        displayCandidates(ss, candidates);
        break;
      default:
        qiLogError() << "Invalid error id for generateErrorString";
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

  /*
   * return a negative value on error
   *  -1 : no method found
   *  -2 : arguments do not matches
   *  -3 : ambiguous matches
   */
  int MetaObjectPrivate::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args, bool* canCache) const
  {
    // We can keep this outside the lock because we assume MetaMethods can't be
    // removed
    MetaMethod* firstOverload = nullptr;
    {
      boost::recursive_mutex::scoped_lock sl(_methodsMutex);
      if (_dirtyCache)
        const_cast<MetaObjectPrivate*>(this)->refreshCache();
      if (nameWithOptionalSignature.find(':') != nameWithOptionalSignature.npos)
      { // full name and signature was given, there can be only one match
        if (canCache)
          *canCache = true;
        int idRev = methodId(nameWithOptionalSignature);
        if (idRev == -1) {
          std::string funname = qi::signatureSplit(nameWithOptionalSignature)[1];
          // check if it's no method found, or if it's arguments mismatch
          if (methodId(funname) != -1) {
            return -2;
          }
          return -1;
        }
        else
          return idRev;
      }
      // Only name given, try to find an unique match with given argument count
      OverloadMap::const_iterator overloadIt = _methodNameToOverload.find(nameWithOptionalSignature);
      if (overloadIt == _methodNameToOverload.end())
      { // no match for the name, no chance
        if (canCache)
          *canCache = true;
        return -1;
      }
      MetaMethod* firstMatch = nullptr;
      bool ambiguous = false;
      size_t nargs = args.size();
      for (MetaMethod* mm = overloadIt->second; mm; mm=mm->_p->next)
      {
        QI_ASSERT(mm->name() == nameWithOptionalSignature);
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
      if (!firstMatch) {
        //TODO....
        return -2; // no match for a correct overload (bad number of args)
      }
      if (!ambiguous) {

        return firstMatch->uid();
      }
      firstOverload = overloadIt->second;
    }

    int retval = -2;
    // resolve ambiguity by using arguments
    for (unsigned dyn = 0; dyn < 2; ++dyn)
    {
      // DO *NOT* hold the lock while resolving signatures dynamically. This
      // may block (and in case of python need the GIL)
      Signature sResolved = args.signature(dyn==1);
      {
        boost::recursive_mutex::scoped_lock sl(_methodsMutex);
        std::string resolvedSig = sResolved.toString();
        std::string fullSig = nameWithOptionalSignature + "::" + resolvedSig;
        qiLogDebug() << "Finding method for resolved signature " << fullSig;
        // First try an exact match, which is much faster if we're lucky.
        int idRev = methodId(nameWithOptionalSignature);
        if (idRev != -1)
          return idRev;

        using MethodsPtr = std::vector<std::pair<const MetaMethod*, float>>;
        MethodsPtr mml;

        // embed findCompatibleMethod
        for (MetaMethod* mm = firstOverload; mm; mm=mm->_p->next)
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
        QI_ASSERT(count);
        if (count > 1) {
          qiLogVerbose() << generateErrorString(nameWithOptionalSignature, fullSig, const_cast<MetaObjectPrivate*>(this)->findCompatibleMethod(nameWithOptionalSignature), -3, false);
          retval = -3;
        } else
          return it->first->uid();
      }
    }
    return retval;
  }

  std::vector<MetaObject::CompatibleMethod> MetaObjectPrivate::findCompatibleMethod(const std::string &nameOrSignature)
  {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    std::vector<MetaObject::CompatibleMethod>         ret;
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

    for (auto& method: _methods) {
      const qi::MetaMethod& mm = method.second;

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
      return  nullptr;
    else
      return &_events[id];
  }

  MemberAddInfo MetaObjectPrivate::addMethod(MetaMethodBuilder& builder, int uid) {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    const qi::MetaMethod method = builder.metaMethod();

    int sigId = signalId(method.toString());
    int propId = propertyId(method.toString());
    if (sigId != -1 || propId != -1)
    {
      std::ostringstream err;
      err << "Method(" << sigId << ") already defined: "
          << method.toString();
      throw std::runtime_error(err.str());
    }

    int methId = methodId(method.toString());
    if (methId != -1) {
      qiLogWarning() << "Method("<< methId << ") already defined (and overriden): "
                     << method.toString();
      return MemberAddInfo(methId, false);
    }

    if (uid == -1)
      uid = ++_index;
    builder.setUid(uid);
    _methods[uid] = builder.metaMethod();
    _objectNameToIdx[method.toString()] = MetaObjectIdType(uid, MetaObjectType_Method);
    _dirtyCache = true;
    return MemberAddInfo(uid, true);
  }

  MemberAddInfo MetaObjectPrivate::addSignal(const std::string &name, const Signature &signature, int uid, bool isSignalProperty) {
#ifndef NDEBUG
    std::vector<std::string> split = signatureSplit(name);
    if (name != split[1])
      throw std::runtime_error("Unexpected full signature " + name);
#endif
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);

    // We need a temporary MetaSignal without any UID to get full signature
    const MetaSignal msWithoutUid(-1, name, signature);
    int methId = methodId(msWithoutUid.toString());
    int propId = propertyId(msWithoutUid.toString());
    if (methId != -1 || propId != -1)
    {
      std::ostringstream err;
      err << "Signal(" << methId << ") already defined: "
          << msWithoutUid.toString();
      throw std::runtime_error(err.str());
    }

    int sigId = signalId(msWithoutUid.toString());
    if (sigId != -1)
    {
      const MetaSignal &ms = _events[sigId];
      qiLogWarning() << "Signal("<< sigId << ") already defined (and overriden): " << ms.toString() <<
                        "instead of requested: " << name;
      return MemberAddInfo(sigId, false);
    }

    if (uid == -1)
      uid = ++_index;
    const MetaSignal ms(uid, name, signature);
    _events[uid] = ms;
    if (isSignalProperty)
    {
      _objectNameToIdx[ms.toString()] = MetaObjectIdType(uid, MetaObjectType_Property);
    }
    else
    {
      _objectNameToIdx[ms.toString()] = MetaObjectIdType(uid, MetaObjectType_Signal);
    }
    _dirtyCache = true;
    return MemberAddInfo(uid, true);
  }

  MemberAddInfo MetaObjectPrivate::addProperty(const std::string& name, const qi::Signature& signature, int id)
  {
    boost::recursive_mutex::scoped_lock sl(_propertiesMutex);
    // We need a temporary MetaProperty without any UID to get full signature
    const MetaProperty mpWithoutUid(-1, name, signature);
    const MetaSignal msWithoutUid(-1, name, std::string("(" + signature.toString() + ")"));

    {
      const int methId = methodId(msWithoutUid.toString());
      const int sigId = signalId(msWithoutUid.toString());

      // If an id is provided, we should find a signal member which have the same signature.
      if (methId != -1 || (sigId != -1 && sigId != id))
      {
        std::ostringstream err;
        err << "Property \"" << mpWithoutUid.toString() << "\" already defined, with method ID #"
          << methId << " and signal ID #" << sigId;
        throw std::runtime_error(err.str());
      }
    }

    for (MetaObject::PropertyMap::iterator it = _properties.begin(); it != _properties.end(); ++it)
    {
      if (it->second.name() == name)
      {
        qiLogWarning() << "Property already exists: " << name;
        return MemberAddInfo(it->second.uid(), false);
      }
    }
    if (id == -1)
      id = ++_index;
    const MetaProperty mp(id, name, signature);
    _properties[id] = mp;
    _objectNameToIdx[mp.toString()] = MetaObjectIdType(id, MetaObjectType_Property);
    _dirtyCache = true;
    return MemberAddInfo(id, true);
  }

  bool MetaObjectPrivate::addMethods(const MetaObject::MethodMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_methodsMutex);
    unsigned int newUid;

    for (const auto& method: mms) {
      newUid = method.second.uid();
      MetaObject::MethodMap::iterator jt = _methods.find(newUid);
      //same id and same signature: we dont mind.
      if (jt != _methods.end()) {
        if ((jt->second.toString() != method.second.toString()) ||
            (jt->second.returnSignature() != method.second.returnSignature()))
          return false;
      }
      _methods[newUid] = qi::MetaMethod(newUid, method.second);
      _objectNameToIdx[method.second.toString()] = MetaObjectIdType(newUid, MetaObjectType_Method);
    }
    _dirtyCache = true;
    //todo: update uid
    return true;
  }

  bool MetaObjectPrivate::addSignals(const MetaObject::SignalMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_eventsMutex);
    unsigned int newUid;

    for (const auto& signal: mms) {
      newUid = signal.second.uid();
      MetaObject::SignalMap::iterator jt = _events.find(newUid);
      if (jt != _events.end()) {
        if ((jt->second.toString() != signal.second.toString()))
          return false;
      }
      const qi::MetaSignal ms(newUid, signal.second.name(), signal.second.parametersSignature());
      _events[newUid] = ms;
      _objectNameToIdx[ms.toString()] = MetaObjectIdType(newUid, MetaObjectType_Signal);
    }
    _dirtyCache = true;
    //todo: update uid
    return true;
  }

  bool MetaObjectPrivate::addProperties(const MetaObject::PropertyMap &mms) {
    boost::recursive_mutex::scoped_lock sl(_propertiesMutex);
    unsigned int newUid;

    for (const auto& property: mms) {
      newUid = property.second.uid();
      MetaObject::PropertyMap::iterator jt = _properties.find(newUid);
      if (jt != _properties.end()) {
        if ((jt->second.toString() != property.second.toString()))
          return false;
      }
      const qi::MetaProperty mp(newUid, property.second.name(), property.second.signature());
      _properties[newUid] = mp;
      _objectNameToIdx[mp.toString()] = MetaObjectIdType(newUid, MetaObjectType_Property);
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
    std::ostringstream buff;
    {
      _objectNameToIdx.clear();
      _methodNameToOverload.clear();
      for (auto& metaMethodsSlot : _methods)
      {
        auto& metaMethod = metaMethodsSlot.second;
        const std::string methodNameSignature = metaMethod.toString();
        _objectNameToIdx[methodNameSignature] = MetaObjectIdType(metaMethod.uid(), MetaObjectType_Method);
        idx = std::max(idx, metaMethod.uid());
        buff << methodNameSignature << metaMethod.uid();

        OverloadMap::iterator overloadIt = _methodNameToOverload.find(metaMethod.name());
        if (overloadIt == _methodNameToOverload.end())
        {
          _methodNameToOverload[metaMethod.name()] = &metaMethod;
          metaMethod._p->next = 0;
        }
        else
        { // push_front
          metaMethod._p->next =  overloadIt->second;
          overloadIt->second = &metaMethod;
        }
      }
    }
    {
      for (auto& metaSignalSlot : _events)
      {
        auto& metaSignal = metaSignalSlot.second;
        const auto metaSignalNameSignature = metaSignal.toString();
        _objectNameToIdx[metaSignalNameSignature] = MetaObjectIdType(metaSignal.uid(), MetaObjectType_Signal);
        idx = std::max(idx, metaSignal.uid());
        buff << metaSignalNameSignature << metaSignal.uid();
      }
    }
    buff << _description;

    // never lower index
    _index = std::max(idx, _index.load());

    // update content hash
    _contentSHA1 = ka::sha1(buff.str());
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
      return  nullptr;
    return &i->second;
  }

  const MetaMethod *MetaObject::method(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_methodsMutex);
    MethodMap::const_iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return  nullptr;
    return &i->second;
  }

  MetaSignal *MetaObject::signal(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_eventsMutex);
    SignalMap::iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return  nullptr;
    return &i->second;
  }

  const MetaSignal *MetaObject::signal(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_eventsMutex);
    SignalMap::const_iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return  nullptr;
    return &i->second;
  }

  MetaProperty *MetaObject::property(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_propertiesMutex);
    PropertyMap::iterator i = _p->_properties.find(id);
    if (i == _p->_properties.end())
      return  nullptr;
    return &i->second;
  }

  const MetaProperty *MetaObject::property(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_propertiesMutex);
    PropertyMap::const_iterator i = _p->_properties.find(id);
    if (i == _p->_properties.end())
      return  nullptr;
    return &i->second;
  }

  int MetaObject::methodId(const std::string &nameWithSignature) const
  {
    return _p->methodId(nameWithSignature);
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

  MemberAddInfo MetaObjectBuilder::addMethod(const qi::Signature& sigret,
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

  MemberAddInfo MetaObjectBuilder::addMethod(MetaMethodBuilder& builder, int id) {
    return _p->metaObject._p->addMethod(builder, id);
  }

  MemberAddInfo MetaObjectBuilder::addSignal(const std::string &name, const qi::Signature& sig, int id) {
    return _p->metaObject._p->addSignal(name, sig, id);
  }

  MemberAddInfo MetaObjectBuilder::addProperty(const std::string& name, const qi::Signature& sig, int id)
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
  namespace detail {

    void printMetaObject(std::ostream& stream,
                         const qi::MetaObject& mobj,
                         bool color,
                         bool showHidden,
                         bool showDoc,
                         bool raw,
                         bool parseable)
    {
      const auto displayHidden = (showHidden ? DisplayHiddenMembers::Display : DisplayHiddenMembers::Hide);

      if (parseable)
      {
        ParseablePrintStream printer(stream, displayHidden);
        printer.print(mobj);
      }
      else
      {
        using Option = PrettyPrintStream::Option;
        using Options = PrettyPrintStream::Options;

        Options opts;
        if (color)   opts.set(Option::Colorized);
        if (showDoc) opts.set(Option::Documentation);
        if (raw)     opts.set(Option::RawSignatures);
        PrettyPrintStream prettyPrinter(stream, displayHidden, opts);
        prettyPrinter.print(mobj);
      }
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

  bool operator < (const MetaObject& a, const MetaObject& b)
  {
    return a._p->_contentSHA1 < b._p->_contentSHA1;
  }
}
