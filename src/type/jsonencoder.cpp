/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <sstream>
#include <string>
#include <iomanip>
#ifdef WITH_BOOST_LOCALE
#  include <boost/locale.hpp>
#endif
#include <qi/jsoncodec.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/typedispatcher.hpp>

qiLogCategory("qitype.jsonencoder");

namespace qi {

  static void serialize(AnyReference val, std::stringstream& out, JsonOption jsonPrintOption, unsigned int indent);

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
  bool add_esc_char(char c, std::string& s, JsonOption jsonPrintOption)
  {
    if (!(jsonPrintOption & JsonOption_Expand))
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
    }
    else
    {
      s += c;
      return true;
    }
    return false;
  }

  //Taken from boost::json
  std::string add_esc_chars(const std::wstring& s, JsonOption jsonPrintOption)
  {
    using Iter_type = std::wstring::const_iterator;
    using Char_type = std::wstring::value_type;

    std::string result;
    const Iter_type end( s.end() );

    for(Iter_type i = s.begin(); i != end; ++i)
    {
      const Char_type c(*i);
      if(add_esc_char(c, result, jsonPrintOption))
        continue;
      const wint_t unsigned_c(c);

      // 127 is the end of printable characters in ASCII table.
      if(iswprint(unsigned_c) && unsigned_c < 127)
        result += c;
      else
        result += non_printable_to_string(unsigned_c);
    }
    return result;
  }

  std::string encodeJSON(const qi::AutoAnyReference &value, JsonOption jsonPrintOption) {
    std::stringstream ss;
    serialize(value, ss, jsonPrintOption, 0);
    return ss.str();
  }

  class SerializeJSONTypeVisitor
  {
  public:
    SerializeJSONTypeVisitor(std::stringstream& outd, JsonOption jsonPrintOptiond, unsigned int indentd)
      : out(outd)
      , jsonPrintOption(jsonPrintOptiond)
      , indent(indentd)
    {
      //force C local, for int and float formatting
      out.imbue(std::locale("C"));
    }

    void printIndent()
    {
      if (jsonPrintOption & qi::JsonOption_PrettyPrint)
      {
        out << std::endl;
        for (unsigned int i = 0; i < indent; ++i)
          out << "  ";
      }
    }

    void printColon()
    {
      if (jsonPrintOption & qi::JsonOption_PrettyPrint)
        out << ": ";
      else
        out << ":";
    }


    void visitUnknown(AnyReference v)
    {
      qiLogError() << "JSON Error: Type " << v.type()->infoString() <<" not serializable";
      out << "\"Error: no serialization for unknown type:" << v.type()->infoString() << "\"";
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
        qiLogError() << "Unknown integer type " << isSigned << " " << byteSize;
      }
    }

    void visitFloat(double value, int byteSize)
    {
      if (byteSize == 4)
      {
        out.precision(std::numeric_limits<float>::max_digits10);
        out << ((float)value);
      }
      else if (byteSize == 8)
      {
        out.precision(std::numeric_limits<double>::max_digits10);
        out << ((double)value);
      }
      else
      {
        qiLogError() << "serialize on unknown float type " << byteSize;
      }
    }

    void visitString(const char* data, size_t size)
    {
#ifdef WITH_BOOST_LOCALE
      out << "\"" << add_esc_chars(boost::locale::conv::to_utf<wchar_t>(std::string(data, size), "UTF-8"), jsonPrintOption) << "\"";
#else
      out << "\"" << add_esc_chars(std::wstring(data, data+size), jsonPrintOption) << "\"";
#endif
    }

    void visitList(AnyIterator begin, AnyIterator end)
    {
      out << "[";
      ++indent;
      const bool empty = begin == end;
      while (begin != end)
      {
        printIndent();
        serialize(*begin, out, jsonPrintOption, indent);
        ++begin;
        if (begin != end)
          out << ",";
      }
      --indent;
      if (!empty)
        printIndent();
      out << "]";
    }

    void visitVarArgs(AnyIterator begin, AnyIterator end)
    {
      visitList(begin, end);
    }

    void visitMap(AnyIterator begin, AnyIterator end)
    {
      out << "{";
      ++indent;
      const bool empty = begin == end;
      while (begin != end)
      {
        printIndent();
        AnyReference e = *begin;
        serialize(e[0], out, jsonPrintOption, indent);
        printColon();
        serialize(e[1], out, jsonPrintOption, indent);
        ++begin;
        if (begin != end)
          out << ",";
      }
      --indent;
      if (!empty)
        printIndent();
      out << "}";
    }

    void visitObject(GenericObject value)
    {
      // TODO: implement?
      qiLogError() << "JSON Error: Serializing an object without a shared pointer";
      out << "\"Error: no serialization for object\"";
    }

    void visitAnyObject(AnyObject& value)
    {
      // TODO: implement?
      qiLogError() << "JSON Error: Serializing an object without a shared pointer";
      out << "\"Error: no serialization for object\"";
    }

    void visitPointer(AnyReference pointee)
    {
      qiLogError() << "JSON Error: error a pointer!!!";
      out << "\"Error: no serialization for pointer\"";
    }

    void visitTuple(const std::string &name, const AnyReferenceVector &vals, const std::vector<std::string> &annotations)
    {
      //is the tuple is annotated serialize as an object
      if (annotations.size()) {
        out << "{";
        ++indent;
        for (unsigned i=0; i<vals.size();++i) {
          printIndent();
          visitString(annotations[i].data(), annotations[i].size());
          printColon();
          serialize(vals[i], out, jsonPrintOption, indent);
          if (i + 1 < vals.size())
            out << ",";
        }
        --indent;
        printIndent();
        out << "}";
        return;
      }

      out << "[";
      ++indent;
      for (unsigned i=0; i<vals.size();++i) {
        printIndent();
        serialize(vals[i], out, jsonPrintOption, indent);
        if (i + 1 < vals.size())
          out << ",";
      }
      --indent;
      printIndent();
      out << "]";
    }

    void visitDynamic(AnyReference pointee)
    {
      if (pointee.isValid()) {
        serialize(pointee, out, jsonPrintOption, indent);
      }
    }

    void visitRaw(AnyReference raw)
    {
      //TODO: implement buffer support
      qiLogError() << "JSON Error: raw data encoder not implemented!!!";
      out << "\"Error: no serialization for Buffer\"";
    }

    void visitIterator(AnyReference)
    {
      qiLogError() << "JSON Error: no serialization for iterator!!!";
      out << "\"Error: no serialization for iterator\"";
    }

    std::stringstream& out;
    JsonOption jsonPrintOption;
    unsigned int indent;
  };

  static void serialize(AnyReference val, std::stringstream& out, JsonOption jsonPrintOption, unsigned int indent)
  {
    SerializeJSONTypeVisitor stv(out, jsonPrintOption, indent);
    qi::typeDispatch(stv, val);
  }

};
