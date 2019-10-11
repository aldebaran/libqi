#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_METAOBJECT_HPP_
#define _QI_TYPE_METAOBJECT_HPP_

#include <qi/type/metamethod.hpp>
#include <qi/type/metasignal.hpp>
#include <qi/type/metaproperty.hpp>
#include <ka/macro.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

namespace qi {

  class MetaObjectPrivate;
  class GenericFunctionParameters;

  /// Description of the signals and methods accessible on an ObjectTypeInterface
  class QI_API MetaObject  {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    /**
    *   @param nameWithSignature The complete full signature (name::signature) for example myMethod::(s).
    *   @return The method's id or -1 if the method wasn't found.
    */
    int methodId(const std::string &nameWithSignature) const;
    /**
    *   @param name The name of the signal or its full signature.
    *   @return The signal's id or -1 if the signal wasn't found.
    */
    int signalId(const std::string &name) const;
    /**
    *   @param name The property's name.
    *   @return The property's id or -1 if the property wasn't found.
    */
    int propertyId(const std::string& name) const;

    using MethodMap = std::map<unsigned int, MetaMethod>;
    /**
    *   @return The map of all the methods.
    */
    MethodMap methodMap() const;

    //not called signals because it conflict with Qt keywords :S
    using SignalMap = std::map<unsigned int, MetaSignal>;
    /**
    *   @return The map of all the signals.
    */
    SignalMap signalMap() const;

    using PropertyMap = std::map<unsigned int, MetaProperty>;
    /**
    *   @return The map of all the properties.
    */
    PropertyMap propertyMap() const;

    /**
    *   @param id The method's id.
    *   @return The desired method or null if the id is invalid.
    */
    MetaMethod*       method(unsigned int id);
    const MetaMethod* method(unsigned int id) const;

    /**
    *   @param id The signal's id.
    *   @return The desired signal or null if the id is invalid.
    */
    MetaSignal*       signal(unsigned int id);
    const MetaSignal* signal(unsigned int id) const;
    /**
    *   @param name The name of the signal or its full signature.
    *   @return The desired signal or null if the signal wasn't found.
    */
    const MetaSignal* signal(const std::string &name) const;

    /**
    *   @param id The property's id.
    *   @return The desired property of null if the id is invalid.
    */
    MetaProperty*       property(unsigned int id);
    const MetaProperty* property(unsigned int id) const;

    /** Find a method matching nameWithOptionalSignature that can be
    *   called with arguments args.
    *   @return The mathing method id, or -1 if none or an ambiguous set was found.
    *   @warning This method optimises for speed at the expense of possible false positive:
    *            It returns a match as soon as there is only one possible candidate
    *            remaining, even though this candidate can prove later on to be
    *            incompatible with the arguments.
    *   @param nameWithOptionalSignature The method's name or its full signature.
    *   @param args The parameters' type of the method.
    *   @param canCache If set, will be filled with true if the returned method
    *          can be cached regardless of the arguments types (but not argument count),
    *          and false otherwise.
    */
    int findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args, bool* canCache=0) const;
    /**
    *   @param name The exact method's name.
    *   @return A vector containing all the overloaded version of the method.
    */
    std::vector<MetaMethod> findMethod(const std::string &name) const;
    using CompatibleMethod = std::pair<MetaMethod, float>;
    /** Find all the methods compatible with nameOrSignature. If no
    *   signature is specified, the method relies on findMethod.
    *   @param nameOrSignature Either the name or the signature of the method.
    *   @return A vector containing all the compatible method and their
    *   associated compatibility's score.
    */
    std::vector<CompatibleMethod> findCompatibleMethod(const std::string &nameOrSignature) const;

    /**
    *   @param name The member's name.
    *   @param uid The uid's name.
    *   @return True if the member is considered internal, and should not be
    *   listed.
    */
    static bool isPrivateMember(const std::string &name, unsigned int uid);

    /** Merge two MetaObject. Dest method and signal ids will be incremented by offset.
    *   @param source The source object.
    *   @param dest The destination object.
    *   @return The merge's result of the two objects.
    */
    static qi::MetaObject merge(const qi::MetaObject &source, const qi::MetaObject &dest);

    /**
    *   @return The object's description.
    */
    std::string description() const;

    MetaObjectPrivate   *_p;
    MetaObject(const MethodMap& methodMap, const SignalMap& signalMap,
      const PropertyMap& propertyMap, const std::string& description);
  };

  bool QI_API operator < (const MetaObject& a, const MetaObject& b);


  /** Information about an operation that attempted to add a member to the type's interface. */
  struct MemberAddInfo
  {
    MemberAddInfo(unsigned int newId, bool newMember)
      : id(newId), isNewMember(newMember)
    {}

    unsigned int id;    ///< Id of the member that has been created or that already existed.
    bool isNewMember;   ///< True iff the member has been created through the operation.
  };

  class MetaObjectBuilderPrivate;
  class QI_API MetaObjectBuilder {
  public:
    MetaObjectBuilder();

    void setDescription(const std::string& desc);

    MemberAddInfo addMethod(const qi::Signature &sigret,
                           const std::string &name,
                           const qi::Signature &signature,
                           int id = -1);

    /// @see MetaObjectPrivate::addMethod()
    MemberAddInfo addMethod(MetaMethodBuilder& builder, int id = -1);

    /// @see MetaObjectPrivate::addSignal()
    MemberAddInfo addSignal(const std::string &name, const qi::Signature& sig, int id = -1);

    /// @see MetaObjectPrivate::addProperty()
    MemberAddInfo addProperty(const std::string& name, const qi::Signature& sig, int id = -1);
    qi::MetaObject metaObject();

  private:
    // C4251
    boost::shared_ptr<MetaObjectBuilderPrivate> _p;
  };

  namespace detail {
    QI_API void printMetaObject(std::ostream &stream, const qi::MetaObject &metaObject, bool color=true, bool showHidden=false, bool showDoc=false, bool raw=false, bool parseable=false);
  }

}

KA_WARNING_POP()

#endif  // _QITYPE_METAOBJECT_HPP_
