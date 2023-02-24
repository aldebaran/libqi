/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/type/detail/type.hpp>
#include <qi/type/typeinterface.hpp>
#include <boost/utility/string_ref.hpp>

namespace qi {

  namespace detail {
  static boost::string_ref kindToString(TypeKind kind)
  {
    switch(kind) {
    case TypeKind_Void:     return "void";
    case TypeKind_Int:      return "int";
    case TypeKind_Float:    return "float";
    case TypeKind_String:   return "string";
    case TypeKind_List:     return "list";
    case TypeKind_Map:      return "map";
    case TypeKind_Object:   return "object";
    case TypeKind_Pointer:  return "pointer";
    case TypeKind_Tuple:    return "tuple";
    case TypeKind_Dynamic:  return "dynamic";
    case TypeKind_Raw:      return "raw";
    case TypeKind_Unknown:  return "unknown";
    case TypeKind_Iterator: return "iterator";
    case TypeKind_Function: return "function";
    case TypeKind_Signal:   return "signal";
    case TypeKind_Property: return "property";
    case TypeKind_VarArgs:  return "vargs";
    case TypeKind_Optional: return "optional";
    }
    return "unhandled";
  }


#define QI_THROW_OP_NOT_IMPL(OP) throw std::runtime_error("Operation " #OP \
  "not implemented for this kind of type:" + kindToString(kind()).to_string())


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
    QI_THROW_OP_NOT_IMPL(className);
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
    case TypeKind_Optional :
      ret.push_back(AnyType(static_cast<OptionalTypeInterface*>(_type)->valueType()));
      return ret;
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(elements);
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
    QI_THROW_OP_NOT_IMPL(members);
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
    QI_THROW_OP_NOT_IMPL(methods);
  }

  FieldInfo       AnyType::method(const uint32_t /*id*/) {
    return FieldInfo();
    QI_THROW_OP_NOT_IMPL(method);
  }

  FieldInfoVector AnyType::methodOverloads(const std::string& /*name*/) {
    FieldInfoVector ret;
    return ret;
    QI_THROW_OP_NOT_IMPL(methodOverloads);
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
    QI_THROW_OP_NOT_IMPL(sigs);
  }

  FieldInfo       AnyType::signal(const uint32_t /*id*/) {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return FieldInfo();
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(signal);
  }

  FieldInfo       AnyType::signal(const std::string& /*id*/) {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return FieldInfo();
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(signal);
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
    QI_THROW_OP_NOT_IMPL(properties);
  }

  FieldInfo       AnyType::property(const uint32_t /*id*/) {
    switch(kind()) {
    case TypeKind_Object:
      //TODO
      return FieldInfo();
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(property);
  }

  FieldInfo       AnyType::property(const std::string& /*name*/) {
    switch(kind()) {
    case TypeKind_Object:
      return FieldInfo();
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(property);
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
    QI_THROW_OP_NOT_IMPL(parametersIn);
  }

  FieldInfoVector AnyType::paramsOut() {
    switch(kind()) {
    case TypeKind_Function:
      break;
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(parametersOut);
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
    case TypeKind_Optional :
      return AnyType(static_cast<OptionalTypeInterface*>(_type)->valueType());
    case TypeKind_Property:
      //TODO
      break;
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(element);
  }

  //Map
  AnyType AnyType::key() {
    switch(kind()) {
    case TypeKind_Map     :
      return AnyType(static_cast<MapTypeInterface*>(_type)->keyType());
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(key);
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
    QI_THROW_OP_NOT_IMPL(bits);
  }

  int AnyType::isSigned()
  {
    switch(kind()) {
    case TypeKind_Int:
      return static_cast<IntTypeInterface*>(_type)->isSigned();
    default:
      break;
    }
    QI_THROW_OP_NOT_IMPL(isSigned);
  }

#undef QI_THROW_OP_NOT_IMPL

  }
}
