#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPESPECIALIZED_HPP_
#define _QIMESSAGING_TYPESPECIALIZED_HPP_

#include <qitype/type.hpp>

namespace qi
{
// Interfaces for specialized types
class GenericListIterator;
class GenericMapIterator;
class QITYPE_API TypeInt: public Type
{
public:
  virtual int64_t get(void* value) const = 0;
  virtual unsigned int size() const = 0; // size in bytes
  virtual bool isSigned() const = 0; // return if type is signed
  virtual void set(void** storage, int64_t value) = 0;
  virtual Kind kind() const { return Int;}
};

class QITYPE_API TypeFloat: public Type
{
public:
  virtual double get(void* value) const = 0;
  virtual unsigned int size() const = 0; // size in bytes
  virtual void set(void** storage, double value) = 0;
  virtual Kind kind() const { return Float;}
};

class Buffer;
class QITYPE_API TypeString: public Type
{
public:
  std::string getString(void* storage) const;
  virtual std::pair<char*, size_t> get(void* value) const = 0;
  void set(void** storage, const std::string& value);
  virtual void set(void** storage, const char* ptr, size_t sz) = 0;
  virtual Buffer* asBuffer(void* storage) { return 0;}
  virtual Kind kind() const { return String;}
};

class QITYPE_API TypePointer: public Type
{
public:
  virtual Type* pointedType() const = 0;
  virtual GenericValue dereference(void* storage) = 0; // must not be destroyed
  virtual Kind kind() const { return Pointer;}
};

template<typename T>
class QITYPE_API TypeIterator: public Type
{
public:
  virtual T dereference(void* storage) = 0; // must not be destroyed
  virtual void  next(void** storage) = 0;
  virtual bool equals(void* s1, void* s2) = 0;
};

class QITYPE_API TypeListIterator: public TypeIterator<GenericValue>
{};

class QITYPE_API TypeMapIterator: public TypeIterator<std::pair<GenericValue, GenericValue> >
{};


class QITYPE_API TypeList: public Type
{
public:
  virtual Type* elementType(void* storage) const = 0;
  virtual size_t size(void* storage) = 0;
  virtual GenericListIterator begin(void* storage) = 0; // Must be destroyed
  virtual GenericListIterator end(void* storage) = 0;  //idem
  virtual void pushBack(void* storage, void* valueStorage) = 0;
  virtual Kind kind() const { return List;}
};

class QITYPE_API TypeMap: public Type
{
public:
  virtual Type* elementType(void* storage) const = 0;
  virtual Type* keyType(void* storage) const = 0;
  virtual size_t size(void* storage) = 0;
  virtual GenericMapIterator begin(void* storage) = 0; // Must be destroyed
  virtual GenericMapIterator end(void* storage) = 0;  //idem
  virtual void insert(void* storage, void* keyStorage, void* valueStorage) = 0;
  virtual Kind kind() const { return Map;}
  // Since our typesystem has no erased operator < or operator ==,
  // TypeMap does not provide a find()
};

class QITYPE_API TypeTuple: public Type
{
public:
  std::vector<GenericValue> getValues(void* storage);
  virtual std::vector<Type*> memberTypes(void*) = 0;
  virtual std::vector<void*> get(void* storage); // must not be destroyed
  virtual void* get(void* storage, unsigned int index) = 0; // must not be destroyed
  virtual void set(void** storage, std::vector<void*>);
  virtual void set(void** storage, unsigned int index, void* valStorage) = 0; // will copy
  virtual Kind kind() const { return Tuple;}
};

class QITYPE_API TypeDynamic: public Type
{
public:
  // Convert storage to a GenericValue, that must be destroyed if res.second is true
  virtual std::pair<GenericValue, bool> get(void* storage) = 0;
  virtual void set(void** storage, GenericValue source) = 0;
  virtual Kind kind() const { return Dynamic;}
};

///@return a Type of kind List that can contains elements of type elementType.
QITYPE_API Type* defaultListType(Type* elementType);

///@return a Type of kind Map with given key and element types
QITYPE_API Type* defaultMapType(Type* keyType, Type* ElementType);

///@return a Type of kind Tuple with givent memberTypes
QITYPE_API Type* defaultTupleType(std::vector<Type*> memberTypes);

}


#endif
