/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/jsoncodec.hpp>
#include <qitype/anyvalue.hpp>

#include "jsoncodec_p.hpp"

namespace qi {

  JsonDecoderPrivate::JsonDecoderPrivate(const std::string &in)
    :_begin(in.begin()),
      _end(in.end()),
      _it(_begin)
  {}

  JsonDecoderPrivate::JsonDecoderPrivate(const std::string::const_iterator &begin,
                    const std::string::const_iterator &end)
    :_begin(begin),
      _end(end),
      _it(_begin)
  {}

  std::string::const_iterator JsonDecoderPrivate::decode(AnyValue &out)
  {
    _it = _begin;
    if (!decodeValue(out))
      throw std::runtime_error("parse error");
    return _it;
  }

  void JsonDecoderPrivate::skipWhiteSpaces()
  {
    while (_it != _end && *_it == ' ')
      ++_it;
  }

  bool JsonDecoderPrivate::getDigits(std::string &result)
  {
    std::string::const_iterator begin = _it;

    while (_it != _end && *_it >= '0' && *_it <= '9')
      ++_it;
    if (_it == begin)
        return false;
    result = std::string(begin, _it);
    return true;
  }

  bool JsonDecoderPrivate::getInteger(std::string &result)
  {
    std::string::const_iterator save = _it;
    std::string integerStr;

    if (_it == _end)
      return false;
    if (*_it == '-')
    {
      ++_it;
      integerStr = "-";
    }
    std::string digitsStr;

    if (!getDigits(digitsStr))
    {
      _it = save;
      return false;
    }
    integerStr += digitsStr;

    result = integerStr;
    return true;
  }

  bool JsonDecoderPrivate::getInteger(qi::int64_t &result)
  {
    std::string integerStr;

    if (!getInteger(integerStr))
      return false;
    result = ::atol(integerStr.c_str());
    return true;
  }

  bool JsonDecoderPrivate::getExponent(std::string &result)
  {
    std::string::const_iterator save = _it;

    if (_it == _end || (*_it != 'e' && *_it != 'E'))
      return false;
    ++_it;
    std::string exponentStr;

    exponentStr += 'e';
    if (*_it == '+' || *_it == '-')
    {
      exponentStr += *_it;
      ++_it;
    }
    else
      exponentStr += '+';
    std::string integerStr;
    if (!getDigits(integerStr))
    {
      _it = save;
      return false;
    }

    result = exponentStr + integerStr;
    return true;
  }

  bool JsonDecoderPrivate::getFloat(double &result)
  {
    std::string floatStr;
    std::string beforePoint;
    std::string afterPoint;
    std::string exponent;
    std::string::const_iterator save = _it;

    if (!getInteger(beforePoint))
      return false;
    if (!getExponent(exponent))
    {
      if (_it == _end || *_it != '.')
      {
        _it = save;
        return false;
      }
      ++_it;
      if (!getDigits(afterPoint))
      {
        _it = save;
        return false;
      }
      getExponent(exponent);
      floatStr = beforePoint + "." + afterPoint + exponent;
    }
    else
      floatStr = beforePoint + exponent;

    result = ::atof(floatStr.c_str());
    return true;
  }

  bool JsonDecoderPrivate::decodeArray(AnyValue &value)
  {
    std::string::const_iterator save = _it;

    if (_it == _end || *_it != '[')
      return false;
    ++_it;
    std::vector<AnyValue>   tmpArray;

    while (true)
    {
      AnyValue subElement;

      if (!decodeValue(subElement))
        break;
      tmpArray.push_back(subElement);
      if (*_it != ',')
        break;
      ++_it;
    }
    if (*_it != ']')
    {
      _it = save;
      return false;
    }
    ++_it;
    value = AnyValue(tmpArray);
    return true;
  }

  bool JsonDecoderPrivate::decodeFloat(AnyValue &value)
  {
    double tmpFloat;

    if (!getFloat(tmpFloat))
      return false;
    value = AnyValue(tmpFloat);
    return true;
  }

  bool JsonDecoderPrivate::decodeInteger(AnyValue &value)
  {
    qi::int64_t tmpInteger;

    if (!getInteger(tmpInteger))
      return false;
    value = AnyValue(tmpInteger);
    return true;
  }

  bool JsonDecoderPrivate::getCleanString(std::string &result)
  {
    std::string::const_iterator save = _it;

    if (_it == _end || *_it != '"')
      return false;
    std::string tmpString;

    ++_it;
    for (;_it != _end && *_it != '"';)
    {
      if (*_it == '\\')
      {
        if (_it + 1 == _end)
        {
          _it = save;
          return false;
        }
        switch (*(_it + 1))
        {
        case '"' :tmpString += '"'; break;
        case '\\':tmpString += '\\';break;
        case '/' :tmpString += '/'; break;
        case 'b' :tmpString += '\b';break;
        case 'f' :tmpString += '\f';break;
        case 'n' :tmpString += '\n';break;
        case 'r' :tmpString += '\r';break;
        case 't' :tmpString += '\t';break;
        default:
          _it = save;
          return false;
        }
        _it += 2;
      }
      else
      {
        tmpString += *_it;
        ++_it;
      }
    }
    if (_it == _end)
    {
      _it = save;
      return false;
    }
    ++_it;
    result = tmpString;
    return true;
  }

  bool JsonDecoderPrivate::decodeString(AnyValue &value)
  {
    std::string tmpString;

    if (!getCleanString(tmpString))
      return false;
    value = AnyValue(tmpString);
    return true;
  }

  bool JsonDecoderPrivate::decodeObject(AnyValue &value)
  {
    std::string::const_iterator save = _it;

    if (_it == _end || *_it != '{')
      return false;
    ++_it;

    std::map<std::string, AnyValue> tmpMap;
    while (true)
    {
      skipWhiteSpaces();
      std::string key;

      if (!getCleanString(key))
        break;
      skipWhiteSpaces();
      if (_it == _end || *_it != ':')
      {
        _it = save;
        return false;
      }
      ++_it;
      AnyValue tmpValue;
      if (!decodeValue(tmpValue))
      {
        _it = save;
        return false;
      }
      if (_it == _end)
        break;
      tmpMap[key] = tmpValue;
      if (*_it != ',')
        break;
      ++_it;
    }
    if (_it == _end || *_it != '}')
    {
      _it = save;
      return false;
    }
    ++_it;
    value = AnyValue(tmpMap);
    return true;
  }

  bool JsonDecoderPrivate::match(std::string const& expected)
  {
    std::string::const_iterator save = _it;
    std::string::const_iterator begin = expected.begin();

    while (_it != _end && begin != expected.end())
    {
      if (*_it != *begin)
      {
        _it = save;
        return false;
      }
      ++_it;
      ++begin;
    }
    if (begin != expected.end())
    {
      _it = save;
      return false;
    }
    return true;
  }

  bool JsonDecoderPrivate::decodeSpecial(AnyValue &value)
  {
    if (_it == _end)
      return false;
    if (match("true"))
      value = AnyValue(true);
    else if (match("false"))
      value = AnyValue::from(false);
    else if (match("null"))
      value = AnyValue();
    else
      return false;
    return true;
  }

  bool JsonDecoderPrivate::decodeValue(AnyValue &value)
  {
    skipWhiteSpaces();
    if (decodeSpecial(value)
        || decodeString(value)
        || decodeFloat(value)
        || decodeInteger(value)
        || decodeArray(value)
        || decodeObject(value)
        )
    {
      skipWhiteSpaces();
      return true;
    }
    return false;
  }

  std::string::const_iterator decodeJSON(const std::string::const_iterator &begin,
                                         const std::string::const_iterator &end,
                                         AnyValue &target)
  {
    JsonDecoderPrivate parser(begin, end);
    return parser.decode(target);
  }

  AnyValue decodeJSON(const std::string &in)
  {
    AnyValue value;
    JsonDecoderPrivate parser(in);

    parser.decode(value);
    return value;
  }

}
