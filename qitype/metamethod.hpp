#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METAMETHOD_HPP_
# define _QITYPE_METAMETHOD_HPP_

# include <string>
# include <map>

# include <qitype/api.hpp>
# include <qitype/type.hpp>

# ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
# endif

namespace qi {
  class MetaMethodParameterPrivate;
  class GenericFunction;
  class QITYPE_API MetaMethodParameter {
  public:
    MetaMethodParameter();
    MetaMethodParameter(const MetaMethodParameter& other);
    MetaMethodParameter(const std::string& name, const std::string& doc);
    ~MetaMethodParameter();

    MetaMethodParameter& operator= (const MetaMethodParameter& other);

    std::string name() const;
    std::string description() const;

    MetaMethodParameterPrivate* _p;
  };
  typedef std::vector<MetaMethodParameter> MetaMethodParameterVector;

  class MetaMethodPrivate;
  /// Representation of a method in an GenericObject.
  class QITYPE_API MetaMethod {
  public:
    MetaMethod();
    MetaMethod(const MetaMethod& other);
    MetaMethod(unsigned int newUid, const MetaMethod& other);
    ~MetaMethod();

    MetaMethod& operator= (const MetaMethod& other);

    unsigned int uid() const;
    std::string signature() const;
    std::string sigreturn() const;
    std::string description() const;
    MetaMethodParameterVector parameters() const;
    std::string returnDescription() const;

    MetaMethodPrivate* _p;

  private:
    friend class TypeImpl<MetaMethod>;
  };

  class MetaMethodBuilderPrivate;
  class QITYPE_API MetaMethodBuilder {
  public:
    MetaMethodBuilder();
    MetaMethodBuilder(const std::string& name, const std::string& doc = "");
    MetaMethodBuilder(const MetaMethodBuilder& other);
    ~MetaMethodBuilder();

    MetaMethodBuilder& operator= (const MetaMethodBuilder& other);

    std::string name() const;

    void setUid(unsigned int uid);
    void setSignatures(const std::string& name, const GenericFunction& f);
    void setSignatures(const GenericFunction& f);
    void setSignature(const std::string& sig);
    void setSigreturn(const std::string& sig);
    void setReturnDescription(const std::string& doc);
    void addParameter(const std::string& name, const std::string& documentation);
    void setDescription(const std::string& documentation);

    qi::MetaMethod metaMethod();

    MetaMethodBuilderPrivate* _p;
  };
} // namespace qi

# ifdef _MSC_VER
#  pragma warning( pop )
# endif

#endif  // _QITYPE_METAMETHOD_HPP_
