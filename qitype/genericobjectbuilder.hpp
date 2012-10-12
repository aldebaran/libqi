#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_GENERICOBJECTBUILDER_HPP_
#define _QIMESSAGING_GENERICOBJECTBUILDER_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/genericobject.hpp>

namespace qi {

  class DynamicObject;
  class GenericObjectBuilderPrivate;
  class QIMESSAGING_API GenericObjectBuilder
  {
  public:
    GenericObjectBuilder();
    GenericObjectBuilder(DynamicObject *dynobject);

    ~GenericObjectBuilder();

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method);
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function);
    template<typename FUNCTION_TYPE>
    inline unsigned int advertiseEvent(const std::string& eventName);


    int xAdvertiseMethod(const std::string &retsig, const std::string& signature, GenericFunction func);
    int xAdvertiseEvent(const std::string& signature);

    qi::ObjectPtr object();
  public:
    GenericObjectBuilderPrivate *_p;
    QI_DISALLOW_COPY_AND_ASSIGN(GenericObjectBuilder);
  };

}

#include <qimessaging/details/genericobjectbuilder.hxx>

#endif  // _QIMESSAGING_GENERICOBJECTBUILDER_HPP_
