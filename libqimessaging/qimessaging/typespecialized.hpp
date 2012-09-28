#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPESPECIALIZED_HPP_
#define _QIMESSAGING_TYPESPECIALIZED_HPP_

#include <qimessaging/type.hpp>

namespace qi
{
// Interfaces for specialized types
class GenericListIterator;
class GenericMapIterator;
class QIMESSAGING_API TypeInt: public Type
{
public:
  virtual int64_t get(void* value) const = 0;
  virtual void set(void** storage, int64_t value) = 0;
  virtual Kind kind() const { return Int;}
};

class QIMESSAGING_API TypeFloat: public Type
{
public:
  virtual double get(void* value) const = 0;
  virtual void set(void** storage, double value) = 0;
  virtual Kind kind() const { return Float;}
};

class QIMESSAGING_API TypeString: public Type
{
public:
  virtual std::string get(void* value) const = 0;
  virtual void set(void** storage, const std::string& value) = 0;
  virtual Kind kind() const { return String;}
};

class QIMESSAGING_API TypePointer: public Type
{
public:
  virtual Type* pointedType() const = 0;
  virtual GenericValue dereference(void* storage) = 0; // must not be destroyed
  virtual Kind kind() const { return Pointer;}
};

template<typename T>
class QIMESSAGING_API TypeIterator: public Type
{
public:
  virtual T dereference(void* storage) = 0; // must not be destroyed
  virtual void  next(void** storage) = 0;
  virtual bool equals(void* s1, void* s2) = 0;
};

class QIMESSAGING_API TypeListIterator: public TypeIterator<GenericValue>
{};

class QIMESSAGING_API TypeMapIterator: public TypeIterator<std::pair<GenericValue, GenericValue> >
{};


class QIMESSAGING_API TypeList: public Type
{
public:
  virtual Type* elementType(void* storage) const = 0;
  virtual GenericListIterator begin(void* storage) = 0; // Must be destroyed
  virtual GenericListIterator end(void* storage) = 0;  //idem
  virtual void pushBack(void* storage, void* valueStorage) = 0;
  virtual Kind kind() const { return List;}
};

class QIMESSAGING_API TypeMap: public Type
{
public:
  virtual Type* elementType(void* storage) const = 0;
  virtual Type* keyType(void* storage) const = 0;
  virtual GenericMapIterator begin(void* storage) = 0; // Must be destroyed
  virtual GenericMapIterator end(void* storage) = 0;  //idem
  virtual void insert(void* storage, void* keyStorage, void* valueStorage) = 0;
  virtual Kind kind() const { return Map;}
};

class QIMESSAGING_API TypeTuple: public Type
{
public:
  virtual std::vector<Type*> memberTypes(void*) = 0;
  virtual std::vector<void*> get(void* storage); // must not be destroyed
  virtual void* get(void* storage, unsigned int index) = 0; // must not be destroyed
  virtual void set(void** storage, std::vector<void*>);
  virtual void set(void** storage, unsigned int index, void* valStorage) = 0;
  virtual Kind kind() const { return Tuple;}
};

}

#include <qimessaging/details/typestring.hxx>
#include <qimessaging/details/typelist.hxx>
#include <qimessaging/details/typemap.hxx>
#include <qimessaging/details/typepointer.hxx>
#include <qimessaging/details/typetuple.hxx>
#endif
