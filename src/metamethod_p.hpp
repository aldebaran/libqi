#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METAMETHOD_P_HPP_
#define _QITYPE_METAMETHOD_P_HPP_

# include <map>
# include <string>

# include <qitype/metamethod.hpp>

namespace qi {
  class MetaMethodParameterPrivate {
  public:
    MetaMethodParameterPrivate();
    MetaMethodParameterPrivate(const std::string& name, const std::string& doc);

    std::string name;
    std::string description;
  };

  class MetaMethodPrivate {
  public:
    MetaMethodPrivate();

    void setDescription(const std::string& desc);
    void addParameter(const MetaMethodParameter& mm);

    unsigned int  uid;
    qi::Signature sigreturn;
    std::string   name;
    qi::Signature parametersSignature;
    std::string   description;
    MetaMethodParameterVector parameters;
    std::string   returnDescription;
  };

  class MetaMethodBuilderPrivate {
  public:
    qi::MetaMethod metaMethod;
  };
}

#endif // _QITYPE_METAMETHOD_P_HPP_
