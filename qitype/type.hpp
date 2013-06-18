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
   Furthermore we need that all "default template" types are registered (included)
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
  class QITYPE_API TypeInterface
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

    //warning update the C enum when updating this one.
    enum Kind
    {
      Void     = 0,
      Int      = 1,
      Float    = 2,
      String   = 3,
      List     = 4,
      Map      = 5,
      Object   = 6,
      Pointer  = 7,
      Tuple    = 8,
      Dynamic  = 9,
      Raw      = 10,
      Unknown  = 11,
      Iterator = 12,
    };


    virtual Kind kind() const;

    // Less must always work: compare pointers if you have to.
    virtual bool less(void* a, void* b) = 0;

    //TODO: DIE
    inline const char* infoString() { return info().asCString(); } // for easy gdb access

    /** @return the serialization signature corresponding to what the type
     * would emit
     * @param resolveDynamic: if true, resolve dynamic types as deep as possible
     * for example a list<GenericValuePtr> that happens to only contain int32
     * will return [i]
     * @warning if resolveDynamic is true, a valid storage must be given
    */
    qi::Signature signature(void* storage=0, bool resolveDynamic = false);

    ///@return a Type on which signature() returns sig.
    static TypeInterface* fromSignature(const qi::Signature& sig);
  };

  /// Runtime Type factory getter. Used by typeOf<T>()
  QITYPE_API TypeInterface*  getType(const std::type_info& type);

  /// Runtime Type factory setter.
  QITYPE_API bool registerType(const std::type_info& typeId, TypeInterface* type);

  /** Get type from a type. Will return a static TypeImpl<T> if T is not registered
   */
  template<typename T> TypeInterface* typeOf();

  /// Get type from a value. No need to delete the result
  template<typename T> TypeInterface* typeOf(const T& v)
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
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T>: public detail::ForbiddenInTypeSystem {};}

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

    ///@{
    /// Low level Internal API

    /** Store type and allocate storage of value.
     * @param type use this type for initialization
     */
    explicit GenericValuePtr(TypeInterface* type);

    /** Create a generic value with type and a value who should have
     * already been allocated.
     * @param type type of this generic value
     * @param value an already alloc place to store value
     */
    GenericValuePtr(TypeInterface* type, void* value) : type(type), value(value) {}

    /// @return the pair (convertedValue, trueIfCopiedAndNeedsDestroy)
    std::pair<GenericValuePtr, bool> convert(TypeInterface* targetType) const;

    std::pair<GenericValuePtr, bool> convert(ListTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(StructTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(MapTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(IntTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(FloatTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(RawTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(StringTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(PointerTypeInterface* targetType) const;
    std::pair<GenericValuePtr, bool> convert(DynamicTypeInterface* targetType) const;




    /** Return the typed pointer behind a GenericValuePtr. T *must* be the type
     * of the value.
     * @return a pointer to the value as a T or 0 if value is not a T.
     * @param check if false, does not validate type before converting
     */
    template<typename T> T* ptr(bool check = true);

    bool isValid() const;
    /// @return true if value is valid and not void
    bool isValue() const;

    /// Helper function that converts and always clone
    GenericValuePtr convertCopy(TypeInterface* targetType) const;

    // get item with key/ïndex 'key'. Return empty GVP or throw in case of failure
    GenericValuePtr _element(const GenericValuePtr& key, bool throwOnFailure);
    void _append(const GenericValuePtr& element);
    void _insert(const GenericValuePtr& key, const GenericValuePtr& val);
    TypeInterface*   type;
    void*   value;
    ///@}

    ///@{
    /** Construction and assign.
     */

    /** Construct a GenericValue with storage pointing to ptr.
     * @warning the GenericValuePtr will become invalid if ptr
     * is destroyed (if it gets deleted or points to the stack and goes
     * out of scope).
     */
    template<typename T>
    explicit GenericValuePtr(T* ptr);

    /** Assignment operator.
     *  Previous content is lost, and will leak if not deleted outside or
     *  with destroy().
     */
    GenericValuePtr& operator = (const GenericValuePtr& b);
    GenericValuePtr clone() const;
    /// Deletes storage.
    void destroy();
    ///@}


    ///@{
    /** The following methods return a typed copy of the stored value,
     * converting if necessary.
     * They throw in case of conversion failure.
     */

     /// Convert to anything or throw trying.
    template<typename T> T to() const;
    /// Similar to previous method, but uses a dummy value to get the target type
    template<typename T> T to(const T&) const;
    int64_t      toInt()    const;
    uint64_t     toUInt()   const;
    float        toFloat()  const;
    double       toDouble() const;
    std::string  toString() const;
    template<typename T> std::vector<T> toList() const;
    template<typename K, typename V> std::map<K, V> toMap() const;
    ObjectPtr    toObject() const;

    /** Convert the value to a tuple.
     * If value is currently a tuple, it will be returned.
     * If value is a list its elements will become the tuple components.
     * @param homogeneous if true, all tuple elements will be of the type
     * of the list element type. If false, the effective type of elements
     * of kind dynamic will be used.
     */
    GenericValue toTuple(bool homogeneous) const;
    ///@}

    qi::Signature signature(bool resolveDynamic = false) const;
    TypeInterface::Kind kind() const;

    ///@{
    /** Read and update functions
     *  The following functions access or modify the existing value.
     *  They never change the storage location or type.
     *  They will fail by throwing an exception if the requested operation
     * is incompatible with the current value type.
     *
     * @warning a GenericValuePtr referring to a container element will
     * become invalid as soon as the container is modified.
     *
    */

    /// Update the value with the one in b
    void  update(const GenericValuePtr& b);

    /** @return a typed reference to the underlying value
     *  @warning This method will only succeed if T exactly matches
     *  the type of the value stored. No conversion will be performed.
     *  So if you only want a value and not a reference, use to() instead.
     */
    template<typename T> T& as();
    int64_t& asInt64() { return as<int64_t>();}
    uint64_t& asUInt64() { return as<uint64_t>();}
    int32_t& asInt32() { return as<int32_t>();}
    uint32_t& asUInt32() { return as<uint32_t>();}
    int16_t& asInt16() { return as<int16_t>();}
    uint16_t& asUInt16() { return as<uint16_t>();}
    int8_t& asInt8() { return as<int8_t>();}
    uint8_t& asUInt8() { return as<uint8_t>();}
    double& asDouble() { return as<double>();}
    float& asFloat() { return as<float>();}
    std::string& asString() { return as<std::string>();}

    /** @return contained GenericValue or throw if type is not dynamic.
     * @note Returned GenericValuePtr might be empty.
    */
    GenericValuePtr asDynamic() const;

    /// @{
    /** Container partial unboxing.
     *  The following functions unbox the container-part of the value.
     *  The values in the contairer are exposed as GenericValuePtr.
     *  The values can be modified using the set and as function families,
     *  But the container itself is a copy.
     * @warning for better performances use the begin() and end() iterator API
    */
    std::vector<GenericValuePtr> asTupleValuePtr();
    std::vector<GenericValuePtr> asListValuePtr();
    std::map<GenericValuePtr, GenericValuePtr> asMapValuePtr();
    /// @}

    /// Update the value to val, which will be converted if required.
    template<typename T> void set(const T& val);
    void set(int64_t v) { setInt(v);}
    void set(int32_t v) { setInt(v);}
    void set(uint64_t v) { setUInt(v);}
    void set(uint32_t v) { setUInt(v);}
    void set(float v) { setFloat(v);}
    void set(double v) { setDouble(v);}
    void set(const std::string& v) { setString(v);}

    void  setInt(int64_t v);
    void  setUInt(uint64_t v);
    void  setFloat(float v);
    void  setDouble(double v);
    void  setString(const std::string& v);
    void  setDynamic(const GenericValuePtr &value);

    ///@{
    /// In-place container manipulation.

    /** Return a reference to container element at index or key idx.
     *  Use set methods on the result for inplace modification.
     *  Behavior depends on the container kind:
     *  - List or tuple: The key must be of integral type. Boundary checks
     *    are performed.
     *  - Map: The key must be of a convertible type to the container key type.
     *    If the key is not found in the container, a new default-valued
     *    Element will be created, inserted. and returned.
     *  @warning the returned value is only valid until owning container
     *  is changed.
    */
    template<typename K>
    GenericValueRef     operator[](const K& key);
    /// Call operator[](key).as<E>, element type must match E
    template<typename E, typename K> E& element(const K& key);
    size_t size() const;
    template<typename T> void append(const T& element);
    template<typename K, typename V> void insert(const K& key, const V& val);
    /** Similar to operator[](), but return an empty GenericValue
     * If the key is not present.
     */
    template<typename K> GenericValuePtr find(const K& key);

    /// Return an iterator on the beginning of the container
    GenericIterator begin() const; //we lie on const but GV does not honor it yet
    /// Return an iterator on the end of the container
    GenericIterator end() const;
    /// Dereference pointer or iterator
    GenericValueRef operator*();
    ///@}

    ///@}

  };

  QITYPE_API bool operator< (const GenericValuePtr& a, const GenericValuePtr& b);
  QITYPE_API bool operator==(const GenericValuePtr& a, const GenericValuePtr& b);
  QITYPE_API bool operator!=(const GenericValuePtr& a, const GenericValuePtr& b);
  /** GenericValuePtr with copy semantics
  */
  class QITYPE_API GenericValue: public GenericValuePtr
  {
  public:

    GenericValue();
    /// Share ownership of value with b.
    GenericValue(const GenericValue& b);
    explicit GenericValue(const GenericValuePtr& b, bool copy, bool free);
    explicit GenericValue(const GenericValuePtr& b);
    explicit GenericValue(const GenericValueRef& b);
    explicit GenericValue(qi::TypeInterface *type);
    /// Create and return a GenericValue of type T
    template<typename T> static GenericValue make();

    /// @{
    /** The following functions construct a GenericValue from containers of
     * GenericValuePtr.
    */
    static GenericValue makeTuple(const std::vector<GenericValuePtr>& values);
    template<typename T>
    static GenericValue makeList(const std::vector<GenericValuePtr>& values);
    static GenericValue makeGenericList(const std::vector<GenericValuePtr>& values);
    template<typename K, typename V>
    static GenericValue makeMap(const std::map<GenericValuePtr, GenericValuePtr>& values);
    static GenericValue makeGenericMap(const std::map<GenericValuePtr, GenericValuePtr>& values);

    /// @}

    ~GenericValue();
    void operator = (const GenericValuePtr& b);
    void operator = (const GenericValue& b);
    void reset();
    void reset(qi::TypeInterface *type);
    template <typename T>
    void set(const T& t) { GenericValuePtr::set<T>(t); }
    void reset(const GenericValuePtr& src);
    void reset(const GenericValuePtr& src, bool copy, bool free);
    void swap(GenericValue& b);

    template<typename T> static GenericValue from(const T& r) { return GenericValue(GenericValuePtr(&r));}

  private:
    //we dont accept GVP here.  (block set<T> with T=GVP)
    void set(const GenericValuePtr& t);
    bool _allocated;
  };

  /** GenericValue with Iterator kind, behaving as a STL-compatible iterator
  */
  class GenericIterator: public GenericValue
  {
  public:
    typedef GenericValueRef value_type;
    typedef GenericValueRef* pointer;
    typedef GenericValueRef& reference;
    typedef ptrdiff_t difference_type;
    typedef std::forward_iterator_tag iterator_category;
    GenericIterator();
    GenericIterator(const GenericValuePtr& p);
    GenericIterator(const GenericValue& v);
    template<typename T> GenericIterator(const T& ref);
    /// Iterator increment
    GenericIterator operator ++();
    /// Dereference
    GenericValueRef operator*();
  };
QITYPE_API bool operator==(const GenericIterator& a, const GenericIterator & b);
QITYPE_API bool operator !=(const GenericIterator & a, const GenericIterator& b);

  /** GenericValueRef with c++ ref semantics:
   *  - Must be constructed from a GenericValuePtr
   *  - Cannot be assigned later on
   *  - Behaves like a value
  */
  class QITYPE_API GenericValueRef: public GenericValuePtr
  {
  public:
    GenericValueRef(const GenericValuePtr&);
    template<typename T> GenericValueRef(const T& ref);
    template<typename T> GenericValueRef& operator =(const T& v);
  };


  /// Less than operator. Will compare the values within the GenericValue.
  QITYPE_API bool operator < (const GenericValue& a, const GenericValue& b);

  /// Value equality operator. Will compare the values within.
  QITYPE_API bool operator==(const GenericValue& a, const GenericValue& b);
  QITYPE_API bool operator!=(const GenericValue& a, const GenericValue& b);

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
    AutoGenericValuePtr(const GenericValuePtr &self)
      : GenericValuePtr(self)
    {}
    AutoGenericValuePtr(const GenericValueRef &self)
      : GenericValuePtr(self)
    {}
    template<typename T> AutoGenericValuePtr(const T& ptr);
  };

  /** @return the value encoded in JSON.
   */
  QITYPE_API std::string encodeJSON(const qi::AutoGenericValuePtr &val);

  /**
    * creates a GV representing a JSON string or throw on parse error.
    * @param JSON string to decode.
    * @return a GV representing the JSON string
    */
  QITYPE_API qi::GenericValue decodeJSON(const std::string &in);

  /**
    * set the input GV to represent the JSON sequence between two string iterators or throw on parse error.
    * @param iterator to the beginning of the sequence to decode.
    * @param iterator to the end of the sequence to decode.
    * @param GV to set. Not modified if an error occurred.
    * @return an iterator to the last read char + 1
    */
  QITYPE_API std::string::const_iterator decodeJSON(std::string::const_iterator begin,
                                         std::string::const_iterator end,
                                         GenericValue &target);

  class ListTypeInterface;

  class StructTypeInterface;

  // Interfaces for specialized types
  class QITYPE_API IntTypeInterface: public TypeInterface
  {
  public:
    virtual int64_t get(void* value) const = 0;
    virtual unsigned int size() const = 0; // size in bytes
    virtual bool isSigned() const = 0; // return if type is signed
    virtual void set(void** storage, int64_t value) = 0;
    virtual Kind kind() const { return Int;}
  };

  class QITYPE_API FloatTypeInterface: public TypeInterface
  {
  public:
    virtual double get(void* value) const = 0;
    virtual unsigned int size() const = 0; // size in bytes
    virtual void set(void** storage, double value) = 0;
    virtual Kind kind() const { return Float;}
  };

  class Buffer;
  class QITYPE_API StringTypeInterface: public TypeInterface
  {
  public:
    std::string getString(void* storage) const;
    virtual std::pair<char*, size_t> get(void* storage) const = 0;
    void set(void** storage, const std::string& value);
    virtual void set(void** storage, const char* ptr, size_t sz) = 0;
    virtual Kind kind() const { return String; }

  };

  class QITYPE_API RawTypeInterface: public TypeInterface
  {
  public:
    virtual std::pair<char*, size_t> get(void* storage) const = 0;
    virtual void set(void** storage, const char* ptr, size_t sz) = 0;
    virtual Kind kind() const { return Raw; }
  };

  class QITYPE_API PointerTypeInterface: public TypeInterface
  {
  public:
    enum PointerKind
    {
      Raw,
      Shared,
    };
    virtual PointerKind pointerKind() const = 0;
    virtual TypeInterface* pointedType() const = 0;
    virtual GenericValuePtr dereference(void* storage) = 0; // must not be destroyed
    // Set new pointee value. pointer must be a *pointer* to type pointedType()
    virtual void setPointee(void** storage, void* pointer) = 0;
    virtual Kind kind() const { return Pointer; }
  };

  class QITYPE_API IteratorTypeInterface: public TypeInterface
  {
  public:
    // Returned reference is expected to point to somewhere in the iterator, or the container
    virtual GenericValueRef dereference(void* storage) = 0;
    virtual void  next(void** storage) = 0;
    virtual bool equals(void* s1, void* s2) = 0;
    virtual Kind kind() const { return Iterator;}
  };

  class QITYPE_API ListTypeInterface: public TypeInterface
  {
  public:
    virtual TypeInterface* elementType() const = 0;
    virtual size_t size(void* storage) = 0;
    virtual GenericIterator begin(void* storage) = 0;
    virtual GenericIterator end(void* storage) = 0;
    virtual void pushBack(void** storage, void* valueStorage) = 0;
    virtual void* element(void* storage, int index);
    virtual Kind kind() const { return List;}
  };

  class QITYPE_API MapTypeInterface: public TypeInterface
  {
  public:
    virtual TypeInterface* elementType() const = 0;
    virtual TypeInterface* keyType() const = 0;
    virtual size_t size(void* storage) = 0;
    virtual GenericIterator begin(void* storage) = 0;
    virtual GenericIterator end(void* storage) = 0;
    virtual void insert(void** storage, void* keyStorage, void* valueStorage) = 0;
    virtual GenericValuePtr element(void** storage, void* keyStorage, bool autoInsert) = 0;
    virtual Kind kind() const { return Map; }
    // Since our typesystem has no erased operator < or operator ==,
    // MapTypeInterface does not provide a find()
  };

  class QITYPE_API StructTypeInterface: public TypeInterface
  {
  public:
    std::vector<GenericValuePtr> values(void* storage);
    virtual std::vector<TypeInterface*> memberTypes() = 0;
    virtual std::vector<void*> get(void* storage); // must not be destroyed
    virtual void* get(void* storage, unsigned int index) = 0; // must not be destroyed
    virtual void set(void** storage, std::vector<void*>);
    virtual void set(void** storage, unsigned int index, void* valStorage) = 0; // will copy
    virtual Kind kind() const { return Tuple; }
    virtual std::vector<std::string> elementsName() { return std::vector<std::string>();}
    virtual std::string className() { return std::string(); }
  };

  class QITYPE_API DynamicTypeInterface: public TypeInterface
  {
  public:
    virtual GenericValuePtr get(void* storage) = 0;
    virtual void set(void** storage, GenericValuePtr source) = 0;
    virtual Kind kind() const { return Dynamic; }
  };

  ///@return a Type of kind List that can contains elements of type elementType.
  QITYPE_API TypeInterface* makeListType(TypeInterface* elementType);

  ///@return a Type of kind Map with given key and element types
  QITYPE_API TypeInterface* makeMapType(TypeInterface* keyType, TypeInterface* ElementType);

  ///@return a Type of kind Tuple with givent memberTypes
  QITYPE_API TypeInterface* makeTupleType(const std::vector<TypeInterface*>& memberTypes, const std::string &name = std::string(), const std::vector<std::string>& elementNames = std::vector<std::string>());

  ///@return an allocated Tuple made from copies of \param values
  QITYPE_API GenericValuePtr makeGenericTuple(const std::vector<GenericValuePtr>& values);

  ///@return a Tuple pointing to \param values as its storage
  QITYPE_API GenericValuePtr makeGenericTuplePtr(
    const std::vector<TypeInterface*>&types,
    const std::vector<void*>&values);

  /** Declare a templated-type taking one type argument.
  * Required to be able to use QI_TEMPLATE_TYPE_GET
  */
  #define QI_TEMPLATE_TYPE_DECLARE(n) \
  namespace qi {              \
    template<typename T> class QITYPE_TEMPLATE_API TypeImpl<n<T> >: public TypeOfTemplateImpl<n, T> {}; \
  }
  /** Return a TemplateTypeInterface pointer if \p typeInst represents an instantiation
   * of template type templateName, 0 otherwise
   */
  #define QI_TEMPLATE_TYPE_GET(typeInst, templateName) \
   dynamic_cast< ::qi::TypeOfTemplate<templateName>*>(typeInst)
}

#include <qitype/details/typeimpl.hxx>
#include <qitype/details/type.hxx>
#include <qitype/details/genericvalue.hxx>
#include <qitype/details/typeint.hxx>
#include <qitype/details/typelist.hxx>
#include <qitype/details/typemap.hxx>
#include <qitype/details/typestring.hxx>
#include <qitype/details/typepointer.hxx>
#include <qitype/details/typetuple.hxx>
#include <qitype/details/typebuffer.hxx>

QI_NO_TYPE(qi::TypeInterface)
QI_NO_TYPE(qi::TypeInterface*)

#ifdef _MSC_VER
#  pragma warning( pop )
// restore the disabling of this warning
#  pragma warning( disable: 4503 )
#endif

#endif  // _QITYPE_TYPE_HPP_
