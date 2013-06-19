/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <sstream>
#include <iomanip>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>

namespace qi {

  static void serialize(AnyReference val, std::stringstream& out);

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

      // 127 is the end of printable characters in ASCII table.
      if(iswprint(unsigned_c) && unsigned_c < 127)
        result += c;
      else
        result += non_printable_to_string(unsigned_c);
    }
    return result;
  }

  std::string encodeJSON(const qi::AutoAnyReference &value) {
    std::stringstream ss;
    serialize(value, ss);
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
    void visitUnknown(AnyReference v)
    {
      qiLogError("qi.type") << "JSON Error: Type " << v.type->infoString() <<" not serializable";
      out << "\"Error: no serialization for unknown type:" << v.type->infoString() << "\"";
    }

    void visitVoid()
    {
      // Not an error, makes sense if encapsulated in a Dynamic for instance
      out << "null";
    }

    void visitInt(int64_t value, bool isSigned, int byteSize)
    {
      switch((isSigned ? 1 : -1) * byteSize)
      {
      case 0: {
        bool v = value != 0;
        if (v)
          out << "true";
        else
          out << "false";
        break;
      }
      case 1:
      case 2:
      case 4:
      case 8:  out << ((int64_t)value); break;
      case -1:
      case -2:
      case -4:
      case -8: out << ((uint64_t)value);break;

      default:
        qiLogError("qi.type") << "Unknown integer type " << isSigned << " " << byteSize;
      }
    }

    void visitFloat(double value, int byteSize)
    {
      if (byteSize == 4)
        out << ((float)value);
      else if (byteSize == 8)
        out << ((double)value);
      else
        qiLogError("qi.type") << "serialize on unknown float type " << byteSize;
    }

    void visitString(const char* data, size_t size)
    {
      out << "\"" << add_esc_chars(std::string(data, size)) << "\"";
    }

    void visitList(GenericIterator begin, GenericIterator end)
    {
      out << "[ ";
      bool clear = begin != end;
      while (begin != end)
      {
        serialize(*begin, out);
        out << ", ";
        ++begin;
      }
      if (clear)
        out.seekp(-2, std::ios_base::cur);
      out << " ]";
    }

    void visitMap(GenericIterator begin, GenericIterator end)
    {
      out << "{ ";
      bool clear = begin != end;
      while (begin != end)
      {
        AnyReference e = *begin;
        serialize(e[0], out);
        out << " : ";
        serialize(e[1], out);
        out << ", ";
        ++begin;
      }
      if (clear)
        out.seekp(-2, std::ios_base::cur);
      out << " }";
    }

    void visitObject(GenericObject value)
    {
      // TODO: implement?
      qiLogError("qi.type") << "JSON Error: Serializing an object without a shared pointer";
      out << "\"Error: no serialization for object\"";
    }

    void visitAnyObject(AnyObject& value)
    {
      // TODO: implement?
      qiLogError("qi.type") << "JSON Error: Serializing an object without a shared pointer";
      out << "\"Error: no serialization for object\"";
    }

    void visitPointer(AnyReference pointee)
    {
      qiLogError("qi.type") << "JSON Error: error a pointer!!!";
      out << "\"Error: no serialization for pointer\"";
    }

    void visitTuple(const std::string &name, const std::vector<AnyReference> &vals, const std::vector<std::string> &annotations)
    {
      //is the tuple is annotated serialize as an object
      if (annotations.size()) {
        out << "{ ";
        std::string tsig;
        for (unsigned i=0; i<vals.size(); ++i) {
          visitString(annotations[i].data(), annotations[i].size());
          out << " : ";
          serialize(vals[i], out);
          if (i < vals.size() + 1)
            out << ", ";
        }
        if (vals.size())
          out.seekp(-2, std::ios_base::cur);
        out << " }";
        return;
      }
      out << "[ ";
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

    void visitDynamic(AnyReference pointee)
    {
      if (pointee.isValid()) {
        serialize(pointee, out);
      }
    }

    void visitRaw(AnyReference raw)
    {
      //TODO: implement buffer support
      qiLogError("qi.type") << "JSON Error: raw data encoder not implemented!!!";
      out << "\"Error: no serialization for Buffer\"";
    }

    void visitIterator(AnyReference)
    {
      qiLogError("qi.type") << "JSON Error: no serialization for iterator!!!";
      out << "\"Error: no serialization for iterator\"";
    }

    std::stringstream& out;
  };

  static void serialize(AnyReference val, std::stringstream& out)
  {
    SerializeJSONTypeVisitor stv(out);
    qi::typeDispatch(stv, val);
  }

};
