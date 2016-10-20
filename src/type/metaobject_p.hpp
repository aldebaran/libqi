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
    MetaObjectPrivate& operator=(const MetaObjectPrivate &rhs);

    enum MetaObjectType
    {
      MetaObjectType_None = 0,
      MetaObjectType_Signal = 1,
      MetaObjectType_Method = 2,
      MetaObjectType_Property = 3
    };

    struct MetaObjectIdType
    {
      MetaObjectIdType() = default;
      ~MetaObjectIdType() = default;
      MetaObjectIdType(const MetaObjectIdType& rhs) = default;
      MetaObjectIdType(unsigned int uid, MetaObjectType t)
        : id(uid)
        , type(t)
      {
      }

      unsigned int id{0};
      MetaObjectType type{MetaObjectType_None};
    };

    using SignatureToIdx = std::map<std::string, MetaObjectIdType>;
    inline int idFromName(const SignatureToIdx& map, const std::string& sig, MetaObjectType type = MetaObjectType_None) const {
      SignatureToIdx::const_iterator it = map.find(sig);
      if (it != map.end())
        if (it->second.type == type)
          return it->second.id;

      return -1;
    }

    inline int methodId(const std::string &signature) const {
      return idFromName(_objectNameToIdx, signature, MetaObjectType_Method);
    }

    inline int propertyId(const std::string &signature) const {
      return idFromName(_objectNameToIdx, signature, MetaObjectType_Property);
    }

    inline int signalId(const std::string &signature) const {
      // Search id from signature
      int id = idFromName(_objectNameToIdx, signature, MetaObjectType_Signal);
      if (id == -1)
      {
        // if the name is a name and not a signature search directly inside the signal map
        for (const auto& ms : _events)
          if (ms.second.name() == signature)
            return ms.first;
      }
      return id;
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

    unsigned int addSignal(const std::string &name, const Signature &signature, int id = -1, bool isSignalProperty = false);

    unsigned int addProperty(const std::string& name, const Signature &signature, int id = -1);

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
    MetaObject::MethodMap               _methods;

  private:
    mutable boost::recursive_mutex      _methodsMutex;

  public:
    using OverloadMap = std::map<std::string, MetaMethod*>;
    OverloadMap                         _methodNameToOverload;

    //name::sig() -> Index
    SignatureToIdx                      _objectNameToIdx;
    MetaObject::SignalMap               _events;
    mutable boost::recursive_mutex      _eventsMutex;

    MetaObject::PropertyMap             _properties;
    mutable boost::recursive_mutex      _propertiesMutex;

    qi::Atomic<unsigned int>            _index;

    std::string                         _description;

    // true if cache must be refreshed
    mutable bool                        _dirtyCache;

    // Global uid for event subscribers.
    static qi::Atomic<int> uid;

    // Generate an error message for overload resolution failure
    std::string generateErrorString(const std::string &signature, const std::string &resolvedSignature,
                                    const std::vector<std::pair<MetaMethod, float> > &candidates,
                                    int error, bool logError = true) const;
    friend class TypeImpl<MetaObjectPrivate>;
    friend class TypeImpl<MetaObject>;
  };

}

#endif  // _SRC_METAOBJECT_P_HPP_
