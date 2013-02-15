/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <sstream>
#include <iomanip>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>

namespace qi {

  static void serialize(GenericValuePtr val, std::stringstream& out);

  //Taken from boost::json
  inline char to_hex_char(unsigned int c)
  {
    assert( c <= 0xF );
    const char ch = static_cast<char>( c );
    if( ch < 10 )
      return '0' + ch;
    return 'A' - 10 + ch;
  }

  //Taken from boost::json
  std::string non_printable_to_string(unsigned int c)
  {
    std::string result( 6, '\\' );
    result[1] = 'u';
    result[ 5 ] = to_hex_char(c & 0x000F);
    c >>= 4;
    result[ 4 ] = to_hex_char(c & 0x000F);
    c >>= 4;
    result[ 3 ] = to_hex_char(c & 0x000F);
    c >>= 4;
    result[ 2 ] = to_hex_char(c & 0x000F);
    return result;
  }

  //Taken from boost::json
  bool add_esc_char(char c, std::string& s)
  {
    switch(c)
    {
      case '"':  s += "\\\""; return true;
      case '\\': s += "\\\\"; return true;
      case '\b': s += "\\b" ; return true;
      case '\f': s += "\\f" ; return true;
      case '\n': s += "\\n" ; return true;
      case '\r': s += "\\r" ; return true;
      case '\t': s += "\\t" ; return true;
    }
    return false;
  }

  //Taken from boost::json
  std::string add_esc_chars(const std::string& s)
  {
    typedef std::string::const_iterator Iter_type;
    typedef std::string::value_type     Char_type;

    std::string result;
    const Iter_type end( s.end() );

    for(Iter_type i = s.begin(); i != end; ++i)
    {
      const Char_type c(*i);
      if(add_esc_char(c, result))
        continue;
      const wint_t unsigned_c((c >= 0) ? c : 256 + c);
      if(iswprint(unsigned_c))
        result += c;
      else
        result += non_printable_to_string(unsigned_c);
    }
    return result;
  }

  std::string encodeJSON(const qi::GenericValue &value) {
    std::stringstream ss;
    serialize(value.data, ss);
    return ss.str();
  }

  class SerializeJSONTypeVisitor
  {
  public:
    SerializeJSONTypeVisitor(std::stringstream& outd)
      : out(outd)
    {
      //force C local, for int and float formatting
      out.imbue(std::locale("C"));
    }
    void visitUnknown(Type* type, void* storage)
    {
      qiLogError("qi.type") << "JSON Error: Type " << type->infoString() <<" not serializable";
      out << "\"Error: no serialization for unknown type:" << type->infoString() << "\"";
    }

    void visitVoid(Type*)
    {
      qiLogError("qi.type") << "JSON Warning: serialiazing void to 'null'";
      // Not an error, makes sense if encapsulated in a Dynamic for instance
      out << "\"Error: no serialization for void\"";
    }

    void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)
    {
      switch((isSigned ? 1 : -1) * byteSize)
      {
      case 0: {
        bool v = ((bool)value);
        if (v)
          out << "true";
        else
          out << "false";
        break;
      }
      case 1:  out << ((int8_t)value);  break;
      case -1: out << ((uint8_t)value); break;
      case 2:  out << ((int16_t)value); break;
      case -2: out << ((uint16_t)value);break;
      case 4:  out << ((int32_t)value); break;
      case -4: out << ((uint32_t)value);break;
      case 8:  out << ((int64_t)value); break;
      case -8: out << ((uint64_t)value);break;
      default:
        qiLogError("qi.type") << "Unknown integer type " << isSigned << " " << byteSize;
      }
    }

    void visitFloat(TypeFloat* type, double value, int byteSize)
    {
      if (byteSize == 4)
        out << ((float)value);
      else if (byteSize == 8)
        out << ((double)value);
      else
        qiLogError("qi.type") << "serialize on unknown float type " << byteSize;
    }

    void visitString(TypeString* type, void* storage)
    {
      std::pair<char*, size_t> data = type->get(storage);
      out << "\"" << add_esc_chars(std::string(data.first, data.second)) << "\"";
    }

    void visitList(GenericListPtr value)
    {
      out << "[ ";
      GenericListIteratorPtr it, end;
      it = value.begin();
      end = value.end();
      bool clear = it != end;
      for (; it != end; ++it) {
        serialize(*it, out);
        out << ", ";
      }
      if (clear)
        out.seekp(-2, std::ios_base::cur);
      it.destroy();
      end.destroy();
      out << " ]";
    }

    void visitMap(GenericMapPtr value)
    {
      out << "{ ";
      GenericMapIteratorPtr it, end;
      it = value.begin();
      end = value.end();
      bool clear = it != end;
      for(; it != end; ++it)
      {
        std::pair<GenericValuePtr, GenericValuePtr> v = *it;
        serialize(v.first, out);
        out << " : ";
        serialize(v.second, out);
        out << ", ";
      }
      if (clear)
        out.seekp(-2, std::ios_base::cur);
      out << " }";
      it.destroy();
      end.destroy();
    }

    void visitObject(GenericObject value)
    {
      // TODO: implement?
      qiLogError("qi.type") << "JSON Error: Serializing an object without a shared pointer";
      out << "\"Error: no serialization for object\"";
    }

    void visitPointer(TypePointer* type, void* storage, GenericValuePtr pointee)
    {
      qiLogError("qi.type") << "JSON Error: error a pointer!!!";
      out << "\"Error: no serialization for pointer\"";
    }

    void visitTuple(TypeTuple* type, void* storage)
    {
      out << "[ ";
      std::vector<GenericValuePtr> vals = type->getValues(storage);
      std::string tsig;
      for (unsigned i=0; i<vals.size(); ++i) {
        serialize(vals[i], out);
        if (i < vals.size() + 1)
          out << ", ";
      }
      if (vals.size())
        out.seekp(-2, std::ios_base::cur);
      out << " ]";
    }

    void visitDynamic(GenericValuePtr source, GenericValuePtr pointee)
    {
      serialize(pointee, out);
    }

    void visitRaw(TypeRaw* type, Buffer* buffer)
    {
      //TODO: implement buffer support
      qiLogError("qi.type") << "JSON Error: raw data encoder not implemented!!!";
      out << "\"Error: no serialization for Buffer\"";
    }

    std::stringstream& out;
  };

  static void serialize(GenericValuePtr val, std::stringstream& out)
  {
    SerializeJSONTypeVisitor stv(out);
    qi::typeDispatch(stv, val.type, &val.value);
  }

};
