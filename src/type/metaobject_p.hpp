/*
**  Copyright (C) 2012-2017 Softbank Robotics Europe
**  See COPYING for the license
*/

#ifndef _SRC_METAOBJECT_P_HPP_
#define _SRC_METAOBJECT_P_HPP_
#pragma once

#include <array>
#include <boost/optional.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <ka/macroregular.hpp>
#include <ka/range.hpp>
#include <qi/atomic.hpp>
#include <ka/sha1.hpp>
#include <qi/type/metasignal.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/type/metamethod.hpp>
#include <qi/anyobject.hpp>

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

    /** Adds a method member to the interface.

        @throws `std::runtime_error` if a signal or property has the same name and signature.

        @param builder  Meta method builder ready to create a meta method object.
                        It will be used to decude name and signature of the method to create.
        @param id       Id to associate with the method to create or -1 (default value) to request a new id.

        @return The id associated to the member and a clarification if it has been created, or not, through this operation.
                The member will not be created if another member of the same category, signature and name has been found,
                in which case the id returned is the already existing one.
    */
    MemberAddInfo addMethod(MetaMethodBuilder& builder, int uid = -1);

    /** Adds a Signal member to the interface.

        @throws `std::runtime_error` if a method or property has the same name and signature.

        @param name             Name to associate with the signal to create.
        @param signature        Signature of the signal to create.
        @param id               Id to associate with the signal to create or -1 (default value) to request a new id.
        @param isSignalProperty If false (default) the signal will be exposed
                                as an independent signal member (dissociate from a property)
                                in the interface.
                                If true, the signal to create will be part of a property interface
                                that you should add after this operation.
                                The signal will then be exposed as part of the property.


        @return The id associated to the member and a clarification if it has been created, or not, through this operation.
                The member will not be created if another member of the same category, signature and name has been found,
                in which case the id returned is the already existing one.
    */
    MemberAddInfo addSignal(const std::string &name, const Signature &signature, int id = -1, bool isSignalProperty = false);

    /** Adds a Property member to the interface.

        @throws `std::runtime_error` if a method have the same name and signature
                or if a signal already exists with the same name and signature but does not has the same id than
                the one specified, if specified.

        @param name       Name to associate with the property to create.
        @param signature  Signature of the property to create.
        @param id         Id to associate with the property to create or -1 (default value) to request a new id.

        @return The id associated to the member and a clarification if it has been created, or not, through this operation.
                The member will not be created if another member of the same category, signature and name has been found,
                in which case the id returned is the one of the member that was already existing.
    */
    MemberAddInfo addProperty(const std::string& name, const Signature &signature, int id = -1);

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


    boost::optional<ka::sha1_digest_t>  _contentSHA1;

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
