#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_TYPE_HPP_
#define _QITYPE_TYPE_HPP_

#include <typeinfo>
#include <string>

#include <boost/preprocessor.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/mpl/if.hpp>

#include <qi/log.hpp>
#include <qitype/api.hpp>
#include <qitype/fwd.hpp>
#include <qitype/signature.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
   // C4503 decorated name length exceeded, name was truncated
   // The only workaround is to make structs to hide the template complexity
   // We don't want to have to do that
#  pragma warning( disable: 4503 )
#endif

/* A lot of class are found in this headers... to kill circular dependencies.
   Futhermore we need that all "default template" types are registered (included)
   when type.hpp is used. (for typeOf to works reliably)
*/

namespace qi{


  /** This class is used to uniquely identify a type.
   *
   */
  class QITYPE_API TypeInfo
  {
  public:
    TypeInfo();
    /// Construct a TypeInfo from a std::type_info
    TypeInfo(const std::type_info& info);
    /// Contruct a TypeInfo from a custom string.
    TypeInfo(const std::string& ti);

    std::string asString() const;
    std::string asDemangledString() const;

    //TODO: DIE
    const char* asCString() const;

    bool operator==(const TypeInfo& b) const;
    bool operator!=(const TypeInfo& b) const;
    bool operator<(const TypeInfo& b) const;

  private:
    const std::type_info* stdInfo;
    // C4251
    std::string           customInfo;
  };

  /** Interface for all the operations we need on any type:
   *
   *  - cloning/destruction in clone() and destroy()
   *  - Access to value from storage and storage creation in
   *    ptrFromStorage() and initializeStorage()
   *  - Type of specialized interface through kind()
   *
   * Our aim is to transport arbitrary values through:
   *  - synchronous calls: Nothing to do, values are just transported and
   *    converted.
   *  - asynchronous call/thread change: Values are copied.
   *  - process change: Values are serialized.
   *
   */
  class QITYPE_API Type
  {
  public:
    virtual const TypeInfo& info() =0;

    // Initialize and return a new storage, from nothing or a T*
    virtual void* initializeStorage(void* ptr=0)=0;

    // Get pointer to type from pointer to storage
    // Use a pointer and not a reference to avoid the case where the compiler makes a copy on the stack
    virtual void* ptrFromStorage(void**)=0;

    virtual void* clone(void*)=0;
    virtual void destroy(void*)=0;

    enum Kind
    {
      Void,
      Int,
      Float,
      String,
      List,
      Map,
      Object,
      Pointer,
      Tuple,
      Dynamic,
      Raw,
      Unknown,
    };

    virtual Kind kind() const;

    //TODO: DIE
    inline const char* infoString() { return info().asCString(); } // for easy gdb access

    std::string signature(void* storage=0, bool resolveDynamic = false);

    ///@return a Type on which signature() returns sig.
    static Type* fromSignature(const qi::Signature& sig);
  };

  /// Runtime Type factory getter. Used by typeOf<T>()
  QITYPE_API Type*  getType(const std::type_info& type);

  /// Runtime Type factory setter.
  QITYPE_API bool registerType(const std::type_info& typeId, Type* type);

  /** Get type from a type. Will return a static TypeImpl<T> if T is not registered
   */
  template<typename T> Type* typeOf();

  /// Get type from a value. No need to delete the result
  template<typename T> Type* typeOf(const T& v)
  {
    return typeOf<T>();
  }


//MACROS

  /// Declare that a type has no accessible default constructor.
  /// \warning Be careful to put the declaration outside any namespaces.
#define QI_TYPE_NOT_CONSTRUCTIBLE(T) \
  namespace qi { namespace detail {  \
  template<> struct TypeManager<T>: public TypeManagerNonDefaultConstructible<T> {};}}

  /// Declare that a type has no metatype and cannot be used in a Value
  /// \warning Be careful to put the declaration outside any namespaces.
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T> {};}

  /// Declare that a type has no accessible copy constructor
  /// \warning Be careful to put the declaration outside any namespaces.
#define QI_TYPE_NOT_CLONABLE(T)     \
  namespace qi { namespace detail { \
  template<> struct TypeManager<T>: public TypeManagerNull<T> {};}}

  /// Register TypeImpl<t> in runtime type factory for 't'. Must be called from a .cpp file
  /// \warning Be careful to put the declaration outside any namespaces.
#define QI_TYPE_REGISTER(t) \
  QI_TYPE_REGISTER_CUSTOM(t, qi::TypeImpl<t>)

  /// Register 'typeimpl' in runtime type factory for 'type'.
  /// \warning Be careful to put the declaration outside any namespaces.
#define QI_TYPE_REGISTER_CUSTOM(type, typeimpl) \
  static bool BOOST_PP_CAT(__qi_registration, __LINE__) = qi::registerType(typeid(type), new typeimpl)



  class GenericListPtr;
  class GenericMapPtr;
  class GenericObjectPtr;
  class GenericTuplePtr;

  /** Class that holds any value, with informations to manipulate it.
   *  operator=() makes a shallow copy.
   *
   * \warning GenericValuePtr should not be used directly as call arguments.
   * Use qi::GenericValue which has value semantics instead.
   *
   */
  class QITYPE_API GenericValuePtr
  {
  public:

    GenericValuePtr();

    /** Store type and allocate storage of value.
     * @param type use this type for initialization
     */
    explicit GenericValuePtr(Type* type);

    /** Create a generic value with type and a value who should have
     * already been allocated.
     * @param type type of this generic value
     * @param value an already alloc place to store value
     */
    GenericValuePtr(Type* type, void* value) : type(type), value(value) {}

    /** Return the typed pointer behind a GenericValuePtr. T *must* be the type
     * of the value.
     * @return a pointer to the value as a T or 0 if value is not a T.
     * @param check if false, does not validate type before converting
     */
    template<typename T> T* ptr(bool check = true);

    /// @return the pair (convertedValue, trueIfCopiedAndNeedsDestroy)
    std::pair<GenericValuePtr, bool> convert(Type* targetType) const;

    /// Helper function that converts and always clone
    GenericValuePtr convertCopy(Type* targetType) const;
    GenericValuePtr clone() const;
    std::string signature(bool resolveDynamic = false) const;
    void destroy();
    Type::Kind kind() const;

    int64_t          asInt() const;
    float            asFloat() const;
    double           asDouble() const;
    std::string      asString() const;
    GenericListPtr   asList() const;
    GenericMapPtr    asMap() const;
    GenericTuplePtr  asTuple() const;
    /** @return contained GenericValue or empty GenericValue if type is not dynamic.
    * \warning returned value is a copy and should be destroyed
    */
    GenericValuePtr  asDynamic() const;

    template<typename T> T as() const;
    // Helper function to obtain type T from a value. Argument value is not used.
    template<typename T> T as(const T&) const;
    template<typename T> static GenericValuePtr from(const T& t);

    Type*   type;
    void*   value;

  };

  /** Class that holds any value, with value semantics.
  */
  class QITYPE_API GenericValue
  {
    // We do not want cast to GenericValuePtr to be possible to avoid
    // mistakes, so do not inherit from GenericValuePtr.
  public:
    GenericValue();

    /** Store type and allocate storage of value.
     * @param type use this type for initialization
     */
    explicit GenericValue(Type* type);
    GenericValue(const GenericValue& b);
    GenericValue(const GenericValuePtr& b, bool copy = true);

    ~GenericValue();

    void operator = (const GenericValuePtr& b);
    void operator = (const GenericValue& b);

    template<typename T> static GenericValue from(const T& src, bool copy=true);

    std::string signature(bool resolveDynamic=false) const;

    /// Take ownership of data in \b. Reset b.
    static GenericValue take(GenericValuePtr& b);

    template<typename T> T as() const;

    Type::Kind kind() const;
    void reset();

    int64_t          asInt() const;
    float            asFloat() const;
    double           asDouble() const;
    std::string      asString() const;
    GenericListPtr   asList() const;
    GenericMapPtr    asMap() const;
    GenericTuplePtr  asTuple() const;
    /** @return contained GenericValue or empty GenericValue if type is not dynamic.
    * \warning returned value might become invalid if this object is destroyed.
    */
    GenericValue     asDynamic() const;

    GenericValuePtr data;
    bool            allocated;
  };

  /** @return the value encoded in JSON.
   */
  QITYPE_API std::string encodeJSON(const qi::GenericValue &val);

  QITYPE_API bool operator< (const qi::GenericValue& a, const qi::GenericValue& b);

  /** Generates GenericValuePtr from everything transparently.
   * To be used as type of meta-function call argument
   *
   *  Example:
   *    void metaCall(ValueGen arg1, ValueGen arg2);
   *  can be called with any argument type:
   *    metaCall("foo", 12);
   */
  class QITYPE_API AutoGenericValuePtr: public GenericValuePtr
  {
  public:
    AutoGenericValuePtr ();
    AutoGenericValuePtr(const AutoGenericValuePtr & b);

    template<typename T> AutoGenericValuePtr(const T& ptr);
  };



  template<typename T>
  class GenericIteratorPtr: public GenericValuePtr
  {
  public:
    void operator++();
    void operator++(int);
    T operator*();
    bool operator==(const GenericIteratorPtr& b) const;
    inline bool operator!=(const GenericIteratorPtr& b) const;
  };

  class QITYPE_API GenericListIteratorPtr: public GenericIteratorPtr<GenericValuePtr>
  {};

  class QITYPE_API GenericMapIteratorPtr: public GenericIteratorPtr<std::pair<GenericValuePtr, GenericValuePtr> >
  {};

  class TypeList;
  class QITYPE_API GenericListPtr: public GenericValuePtr
  {
  public:
    GenericListPtr();
    GenericListPtr(GenericValuePtr&);
    GenericListPtr(TypeList* type, void* value);

    size_t size();
    GenericListIteratorPtr begin();
    GenericListIteratorPtr end();
    void pushBack(GenericValuePtr val);
    Type* elementType();
  };

  class TypeMap;
  class QITYPE_API GenericMapPtr: public GenericValuePtr
  {
  public:
    GenericMapPtr();
    GenericMapPtr(GenericValuePtr&);
    GenericMapPtr(TypeMap* type, void* value);

    size_t size();
    GenericMapIteratorPtr begin();
    GenericMapIteratorPtr end();
    void insert(GenericValuePtr key, GenericValuePtr val);
    Type* keyType();
    Type* elementType();
  };

  class TypeTuple;
  class QITYPE_API GenericTuplePtr: public GenericValuePtr
  {
  public:
    GenericTuplePtr();
    GenericTuplePtr(GenericValuePtr&);
    GenericTuplePtr(TypeTuple* type, void* value);

    std::vector<GenericValuePtr> get();
    void set(const std::vector<GenericValuePtr>& data);
    std::vector<Type*> memberTypes();
  };

  // Interfaces for specialized types
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
    virtual std::pair<char*, size_t> get(void* storage) const = 0;
    void set(void** storage, const std::string& value);
    virtual void set(void** storage, const char* ptr, size_t sz) = 0;
    virtual Kind kind() const { return String; }

  };

  class QITYPE_API TypeRaw: public Type
  {
  public:
    virtual Buffer get(void *storage) = 0;
    virtual void set(void** storage, Buffer& value) = 0;
    virtual Kind kind() const { return Raw; }
  };

  class QITYPE_API TypePointer: public Type
  {
  public:
    enum PointerKind
    {
      Raw,
      Shared,
    };
    virtual PointerKind pointerKind() const = 0;
    virtual Type* pointedType() const = 0;
    virtual GenericValuePtr dereference(void* storage) = 0; // must not be destroyed
    virtual Kind kind() const { return Pointer; }
  };

  template<typename T>
  class QITYPE_API TypeIterator: public Type
  {
  public:
    virtual T dereference(void* storage) = 0; // must not be destroyed
    virtual void  next(void** storage) = 0;
    virtual bool equals(void* s1, void* s2) = 0;
  };

  class QITYPE_API TypeListIterator: public TypeIterator<GenericValuePtr>
  {};

  class QITYPE_API TypeMapIterator: public TypeIterator<std::pair<GenericValuePtr, GenericValuePtr> >
  {};

  class QITYPE_API TypeList: public Type
  {
  public:
    virtual Type* elementType() const = 0;
    virtual size_t size(void* storage) = 0;
    virtual GenericListIteratorPtr begin(void* storage) = 0; // Must be destroyed
    virtual GenericListIteratorPtr end(void* storage) = 0;  //idem
    virtual void pushBack(void** storage, void* valueStorage) = 0;
    virtual Kind kind() const { return List;}
  };

  class QITYPE_API TypeMap: public Type
  {
  public:
    virtual Type* elementType() const = 0;
    virtual Type* keyType() const = 0;
    virtual size_t size(void* storage) = 0;
    virtual GenericMapIteratorPtr begin(void* storage) = 0; // Must be destroyed
    virtual GenericMapIteratorPtr end(void* storage) = 0;  //idem
    virtual void insert(void** storage, void* keyStorage, void* valueStorage) = 0;
    virtual Kind kind() const { return Map; }
    // Since our typesystem has no erased operator < or operator ==,
    // TypeMap does not provide a find()
  };

  class QITYPE_API TypeTuple: public Type
  {
  public:
    std::vector<GenericValuePtr> getValues(void* storage);
    virtual std::vector<Type*> memberTypes() = 0;
    virtual std::vector<void*> get(void* storage); // must not be destroyed
    virtual void* get(void* storage, unsigned int index) = 0; // must not be destroyed
    virtual void set(void** storage, std::vector<void*>);
    virtual void set(void** storage, unsigned int index, void* valStorage) = 0; // will copy
    virtual Kind kind() const { return Tuple; }
  };

  class QITYPE_API TypeDynamic: public Type
  {
  public:
    // Convert storage to a GenericValuePtr, that must be destroyed if res.second is true
    virtual std::pair<GenericValuePtr, bool> get(void* storage) = 0;
    virtual void set(void** storage, GenericValuePtr source) = 0;
    virtual Kind kind() const { return Dynamic; }
  };

  ///@return a Type of kind List that can contains elements of type elementType.
  QITYPE_API Type* makeListType(Type* elementType);

  ///@return a Type of kind Map with given key and element types
  QITYPE_API Type* makeMapType(Type* keyType, Type* ElementType);

  ///@return a Type of kind Tuple with givent memberTypes
  QITYPE_API Type* makeTupleType(std::vector<Type*> memberTypes);

  ///@return a Tuple made from copies of \param values
  QITYPE_API GenericValuePtr makeGenericTuple(std::vector<GenericValuePtr> values);

}

#include <qitype/details/typeimpl.hxx>
#include <qitype/details/type.hxx>
#include <qitype/details/genericvaluespecialized.hxx>
#include <qitype/details/genericvalue.hxx>
#include <qitype/details/typeint.hxx>
#include <qitype/details/typelist.hxx>
#include <qitype/details/typemap.hxx>
#include <qitype/details/typestring.hxx>
#include <qitype/details/typepointer.hxx>
#include <qitype/details/typetuple.hxx>
#include <qitype/details/typebuffer.hxx>

#ifdef _MSC_VER
#  pragma warning( pop )
// restore the disabling of this warning
#  pragma warning( disable: 4503 )
#endif

#endif  // _QITYPE_TYPE_HPP_
