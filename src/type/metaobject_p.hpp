#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_METAOBJECT_P_HPP_
#define _SRC_METAOBJECT_P_HPP_

#include <qi/atomic.hpp>
#include <qi/type/metasignal.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/type/metamethod.hpp>
#include <qi/anyobject.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace qi {

  class MetaObjectPrivate {
  public:
    //by default we start at qiObjectSpecialMemberMaxUid,
    //the first qiObjectSpecialMemberMaxUid id are reserved for bound object
    MetaObjectPrivate()
      : _index(qiObjectSpecialMemberMaxUid - 1)
      , _dirtyCache(false)
    {
    }

    MetaObjectPrivate(const MetaObjectPrivate &rhs);
    MetaObjectPrivate&  operator=(const MetaObjectPrivate &rhs);

    using NameToIdx = std::map<std::string, unsigned int>;

    inline int idFromName(const NameToIdx& map, const std::string& name) {
      NameToIdx::const_iterator it = map.find(name);
      if (it == map.end())
        return -1;
      else
        return it->second;
    }

    inline int methodId(const std::string &name) {
      return idFromName(_methodsNameToIdx, name);
    }

    inline int signalId(const std::string &name) {
      int res = idFromName(_eventsNameToIdx, name);
      if (res < 0)
      { // maybe we were given a full signature, but signal is indexed by name
        std::vector<std::string> split = signatureSplit(name);
        if (name != split[1])
          return signalId(split[1]);
      }
      return res;
    }

    //if you want to use those methods think twice...
    //they are only useful to merge two metaobjects and I bet
    //you really dont want to do that.
    bool addMethods(const MetaObject::MethodMap &mms);
    bool addSignals(const MetaObject::SignalMap &mms);
    bool addProperties(const MetaObject::PropertyMap &mms);

    std::vector<MetaMethod> findMethod(const std::string &name) const;
    std::vector<MetaObject::CompatibleMethod> findCompatibleMethod(const std::string &nameOrSignature);

    MetaSignal* signal(const std::string &name);

    unsigned int addMethod(MetaMethodBuilder& builder, int uid = -1);

    unsigned int addSignal(const std::string &name, const Signature &signature, int id = -1);

    unsigned int addProperty(const std::string& name, const Signature &sig, int id = -1);

    // Recompute data cached in *ToIdx
    void refreshCache();

    void setDescription(const std::string& desc);

    int findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args, bool* canCache) const;

  private:
    friend class MetaObject;

  public:
    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    //name::sig() -> Index
    NameToIdx                           _methodsNameToIdx;
    MetaObject::MethodMap               _methods;

  private:
    mutable boost::recursive_mutex      _methodsMutex;

  public:
    using OverloadMap = std::map<std::string, MetaMethod*>;
    OverloadMap                         _methodNameToOverload;

    //name::sig() -> Index
    NameToIdx                           _eventsNameToIdx;
    MetaObject::SignalMap               _events;
    mutable boost::recursive_mutex      _eventsMutex;

    MetaObject::PropertyMap             _properties;
    mutable boost::recursive_mutex      _propertiesMutex;

    qi::Atomic<unsigned int>            _index;

    std::string                         _description;

    // true if cache must be refreshed
    mutable bool                        _dirtyCache;

    // SHA1 of the metaobject's content, used to compare two meta objects
    struct SHA1Digest
    {
      static const int sha1DigestSize = 5;
      unsigned int sha1Digest[sha1DigestSize] = {}; // force default-initialization <=> zero-initialization for int
    };
    SHA1Digest                          _contentSHA1;

    // Global uid for event subscribers.
    static qi::Atomic<int> uid;

    // Generate an error message for overload resolution failure
    std::string generateErrorString(const std::string &signature, const std::string &resolvedSignature,
                                    const std::vector<std::pair<MetaMethod, float> > &candidates,
                                    int error, bool logError = true) const;
    friend class TypeImpl<MetaObjectPrivate>;
    friend class TypeImpl<MetaObject>;
  };

  inline bool operator< (const MetaObjectPrivate::SHA1Digest& lhs, const MetaObjectPrivate::SHA1Digest& rhs)
  {
    return std::memcmp(lhs.sha1Digest, rhs.sha1Digest, sizeof(lhs.sha1Digest)) < 0;
  }

}

#endif  // _SRC_METAOBJECT_P_HPP_
