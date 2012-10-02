#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#ifndef _QIMESSAGING_GENERICVALUESPECIALIZED_HPP_
#define _QIMESSAGING_GENERICVALUESPECIALIZED_HPP_

#include <qimessaging/genericvalue.hpp>


namespace qi
{
  template<typename T>
  class GenericIterator: public GenericValue
  {
  public:
    void operator ++();
    void operator ++(int);
    T operator *();
    bool operator ==(const GenericIterator& b) const;
    inline bool operator !=(const GenericIterator& b) const;
  };

  class GenericListIterator: public GenericIterator<GenericValue>
  {};

  class GenericMapIterator: public GenericIterator<std::pair<GenericValue, GenericValue> >
  {};

  class GenericList: public GenericValue
  {
  public:
    GenericList();
    GenericList(GenericValue&);
    GenericList(TypeList* type, void* value);
    size_t size();
    GenericListIterator begin();
    GenericListIterator end();
    void pushBack(GenericValue val);
    Type* elementType();
  };

  class GenericMap: public GenericValue
  {
  public:
    GenericMap();
    GenericMap(GenericValue&);
    GenericMap(TypeMap* type, void* value);
    size_t size();
    GenericMapIterator begin();
    GenericMapIterator end();
    void insert(GenericValue key, GenericValue val);
    Type* keyType();
    Type* elementType();
  };

  QIMESSAGING_API GenericValue makeGenericTuple(std::vector<GenericValue> values);

}

#include <qimessaging/details/genericvaluespecialized.hxx>
#include <qimessaging/details/typelist.hxx>
#include <qimessaging/details/typemap.hxx>
#include <qimessaging/details/typepointer.hxx>
#endif
