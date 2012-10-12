#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_GENERICVALUESPECIALIZED_HPP_
#define _QITYPE_GENERICVALUESPECIALIZED_HPP_

#include <qitype/genericvalue.hpp>


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

  QITYPE_API GenericValue makeGenericTuple(std::vector<GenericValue> values);

}

#include <qitype/details/genericvaluespecialized.hxx>
#include <qitype/details/typelist.hxx>
#include <qitype/details/typemap.hxx>
#include <qitype/details/typepointer.hxx>
#endif  // _QITYPE_GENERICVALUESPECIALIZED_HPP_
