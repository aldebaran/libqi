/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/type/detail/type.hpp>
#include <qi/type/typeinterface.hpp>

namespace qi {

  namespace detail {
  static std::string kindToString(TypeKind kind) {
    switch(kind) {
    case TypeKind_Void    : {
      static std::string r("void");
      return r;
    }
    case TypeKind_Int     :{
      static std::string r("int");
      return r;
    }
    case TypeKind_Float   :{
      static std::string r("float");
      return r;
    }
    case TypeKind_String  :{
      static std::string r("string");
      return r;
    }
    case TypeKind_List    :{
      static std::string r("list");
      return r;
    }
    case TypeKind_Map     :{
      static std::string r("map");
      return r;
    }
    case TypeKind_Object  :{
      static std::string r("object");
      return r;
    }
    case TypeKind_Pointer :{
      static std::string r("pointer");
      return r;
    }
    case TypeKind_Tuple   :{
      static std::string r("tuple");
      return r;
    }
    case TypeKind_Dynamic :{
      static std::string r("dynamic");
      return r;
    }
    case TypeKind_Raw     :{
      static std::string r("raw");
      return r;
    }
    case TypeKind_Unknown :{
      static std::string r("unknown");
      return r;
    }
    case TypeKind_Iterator:{
      static std::string r("iterator");
      return r;
    }
    case TypeKind_Function:{
      static std::string r("function");
      return r;
    }
    case TypeKind_Signal:{
      static std::string r("signal");
      return r;
    }
    case TypeKind_Property:{
      static std::string r("property");
      return r;
    }
    case TypeKind_VarArgs:{
      static std::string r("vargs");
      return r;
    }
    }
    return "unhandled";
  }


 #define PLOUF(OP) throw std::runtime_error(std::string("Operation ") + std::string(#OP) + std::string("not implemented for this kind of type:") + kindToString(kind()))


  AnyType::AnyType(TypeInterface *typeInterface)
    : _type(typeInterface)
  {
  }

  AnyType::AnyType()
    : _type(0)
  {
  }

  AnyType::AnyType(const AnyType& rhs)
    : _type(rhs.type())
  {
  }

  AnyType &AnyType::operator=(const AnyType&rhs) {
    _type = rhs.type();
    return *this;
  }

  //copy/dtor/assign

  TypeKind      AnyType::kind() const {
    //TODO
    return (TypeKind)_type->kind();
  }

  TypeSignature AnyType::signature() const {
    return _type->signature();
  }

  //Mandatory
  std::string AnyType::name() const {
    return _type->info().asString();
  }

  std::string AnyType::className() const {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return std::string("TODO");
    case TypeKind_Tuple:
      //TODO
      return std::string("TODO");
    default:
      break;
    }
    PLOUF(className);
  }

  //NamedTuple/Tuple/List/Map/Methods/Signal/Properties/pointer/Dynamic/Objects
  AnyTypeVector AnyType::elements() {
    AnyTypeVector ret;
    switch(kind()) {
    case TypeKind_VarArgs :
    case TypeKind_List    :
      ret.push_back(AnyType(static_cast<ListTypeInterface*>(_type)->elementType()));
      return ret;
    case TypeKind_Map     :
      ret.push_back(AnyType(static_cast<MapTypeInterface*>(_type)->keyType()));
      ret.push_back(AnyType(static_cast<MapTypeInterface*>(_type)->elementType()));
      return ret;
    case TypeKind_Object  :
      //const qi::MetaObject &mo = static_cast<TypeObject*>(_type)->metaObject();
      //TODO
      return ret;
    case TypeKind_Pointer :
      ret.push_back(AnyType(static_cast<PointerTypeInterface*>(_type)->pointedType()));
      return ret;
    case TypeKind_Tuple   :
      //TODO
      return ret;
    case TypeKind_Function:
      //TODO
      return ret;
    default:
      break;
    }
    PLOUF(elements);
  }

  //Struct/Object
  FieldInfoVector AnyType::members() {
    FieldInfoVector ret;
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return ret;
    case TypeKind_Tuple:
      //TODO
      return ret;
    default:
      break;
    }
    PLOUF(members);
  }

  //Object
  FieldInfoVector AnyType::methods() {
    FieldInfoVector ret;
    switch(kind()) {
    case TypeKind_Object:
      return ret;
    default:
      break;
    }
    PLOUF(methods);
  }

  FieldInfo       AnyType::method(const uint32_t id) {
    return FieldInfo();
    PLOUF(method);
  }

  FieldInfoVector AnyType::methodOverloads(const std::string& name) {
    FieldInfoVector ret;
    return ret;
    PLOUF(methodOverloads);
  }

  FieldInfoVector AnyType::sigs() {
    FieldInfoVector ret;
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return ret;
    default:
      break;
    }
    PLOUF(sigs);
  }

  FieldInfo       AnyType::signal(const uint32_t id) {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return FieldInfo();
    default:
      break;
    }
    PLOUF(signal);
  }

  FieldInfo       AnyType::signal(const std::string& id) {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return FieldInfo();
    default:
      break;
    }
    PLOUF(signal);
  }


  FieldInfoVector AnyType::properties() {
    FieldInfoVector ret;
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return ret;
    default:
      break;
    }
    PLOUF(properties);
  }

  FieldInfo       AnyType::property(const uint32_t id) {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return FieldInfo();
    default:
      break;
    }
    PLOUF(property);
  }

  FieldInfo       AnyType::property(const std::string& name) {
    switch(kind()) {
    case TypeKind_Object:
      return FieldInfo();
    default:
      break;
    }
    PLOUF(property);
  }


  //methods/signals/properties
  FieldInfoVector AnyType::paramsIn() {
    switch(kind()) {
    case TypeKind_Function:
      break;
    case TypeKind_Signal:
      break;
    default:
      break;
    }
    PLOUF(parametersIn);
  }

  FieldInfoVector AnyType::paramsOut() {
    switch(kind()) {
    case TypeKind_Function:
      break;
    default:
      break;
    }
    PLOUF(parametersOut);
  }

  //List/Map/Pointer/Properties
  AnyType AnyType::element() {
    switch(kind()) {
    case TypeKind_VarArgs :
    case TypeKind_List    :
      return AnyType(static_cast<ListTypeInterface*>(_type)->elementType());
    case TypeKind_Map     :
      return AnyType(static_cast<MapTypeInterface*>(_type)->elementType());
    case TypeKind_Pointer :
      return AnyType(static_cast<PointerTypeInterface*>(_type)->pointedType());
    case TypeKind_Property:
      //TODO
      break;
    default:
      break;
    }
    PLOUF(element);
  }

  //Map
  AnyType AnyType::key() {
    switch(kind()) {
    case TypeKind_Map     :
      return AnyType(static_cast<MapTypeInterface*>(_type)->keyType());
    default:
      break;
    }
    PLOUF(key);
  }
  //NO MORE SUGAR

  //TODO: remove and fix TypeInt/TypeFloat
  static int bits2bits(int b) {
    switch(b) {
    case 0:
      return 1;
    case 1:
      return 8;
    case 2:
      return 16;
    case 4:
      return 32;
    case 8:
      return 64;
    default:
      break;
    }
    return -1;
  }


  //Float/Int
  int AnyType::bits() {
    switch(kind()) {
    case TypeKind_Int:
      return bits2bits(static_cast<IntTypeInterface*>(_type)->size());
    case TypeKind_Float:
      return bits2bits(static_cast<FloatTypeInterface*>(_type)->size());
    default:
      break;

    }
    PLOUF(bits);
  }

  int AnyType::isSigned()
  {
    switch(kind()) {
    case TypeKind_Int:
      return static_cast<IntTypeInterface*>(_type)->isSigned();
    default:
      break;
    }
    PLOUF(isSigned);
  }

  }

}
