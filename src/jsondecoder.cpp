#include <cerrno>
#include <qitype/type.hpp>

namespace qi {

struct JSONParseInfos
{
  JSONParseInfos(std::string::const_iterator it, std::string::const_iterator end)
    :it(it),
      end(end)
  {}
  std::string::const_iterator it;
  std::string::const_iterator const end;
};

static bool decodeValue(JSONParseInfos &infos, AnyValue &value);

static void skipWhiteSpaces(JSONParseInfos &infos)
{
  while (infos.it != infos.end && *infos.it == ' ')
    ++infos.it;
}

static bool getDigits(JSONParseInfos &infos, std::string &result)
{
  std::string::const_iterator begin = infos.it;

  while (infos.it != infos.end && *infos.it >= '0' && *infos.it <= '9')
    ++infos.it;
  if (infos.it == begin)
    return false;
  result = std::string(begin, infos.it);
  return true;
}

static bool getInteger(JSONParseInfos &infos, std::string &result)
{
  std::string::const_iterator save = infos.it;
  std::string integerStr;

  if (infos.it == infos.end)
    return false;
  if (*infos.it == '-')
  {
    ++infos.it;
    integerStr = "-";
  }
  std::string digitsStr;

  if (!getDigits(infos, digitsStr))
  {
    infos.it = save;
    return false;
  }
  integerStr += digitsStr;

  result = integerStr;
  return true;
}

static bool getInteger(JSONParseInfos &infos, qi::int64_t &result)
{
  std::string integerStr;

  if (!getInteger(infos, integerStr))
    return false;
  result = ::atol(integerStr.c_str());
  return true;
}

static bool getExponent(JSONParseInfos &infos, std::string &result)
{
  std::string::const_iterator save = infos.it;

  if (infos.it == infos.end || (*infos.it != 'e' && *infos.it != 'E'))
    return false;
  ++infos.it;
  std::string exponentStr;

  exponentStr += 'e';
  if (*infos.it == '+' || *infos.it == '-')
  {
    exponentStr += *infos.it;
    ++infos.it;
  }
  else
    exponentStr += '+';
  std::string integerStr;
  if (!getDigits(infos, integerStr))
  {
    infos.it = save;
    return false;
  }

  result = exponentStr + integerStr;
  return true;
}

static bool getFloat(JSONParseInfos &infos, double &result)
{
  std::string floatStr;
  std::string beforePoint;
  std::string afterPoint;
  std::string exponent;
  std::string::const_iterator save = infos.it;

  if (!getInteger(infos, beforePoint))
    return false;
  if (!getExponent(infos, exponent))
  {
    if (*infos.it != '.')
    {
      infos.it = save;
      return false;
    }
    ++infos.it;
    if (!getDigits(infos, afterPoint))
    {
      infos.it = save;
      return false;
    }
    getExponent(infos, exponent);
    floatStr = beforePoint + "." + afterPoint + exponent;
  }
  else
    floatStr = beforePoint + exponent;

  result = ::atof(floatStr.c_str());
  return true;
}

static bool decodeArray(JSONParseInfos &infos, AnyValue &value)
{
  std::string::const_iterator save = infos.it;

  if (infos.it == infos.end || *infos.it != '[')
    return false;
  ++infos.it;
  std::vector<AnyValue>   tmpArray;

  while (true)
  {
    AnyValue subElement;

    if (!decodeValue(infos, subElement))
      break;
    tmpArray.push_back(subElement);
    if (*infos.it != ',')
      break;
    ++infos.it;
  }
  if (*infos.it != ']')
  {
    infos.it = save;
    return false;
  }
  ++infos.it;
  value = AnyValue(tmpArray);
  return true;
}

static bool decodeFloat(JSONParseInfos &infos, AnyValue &value)
{
  double tmpFloat;

  if (!getFloat(infos, tmpFloat))
    return false;
  value = AnyValue(tmpFloat);
  return true;
}

static bool decodeInteger(JSONParseInfos &infos, AnyValue &value)
{
  qi::int64_t tmpInteger;

  if (!getInteger(infos, tmpInteger))
    return false;
  value = AnyValue(tmpInteger);
  return true;
}

static bool getCleanString(JSONParseInfos &infos, std::string &result)
{
  std::string::const_iterator save = infos.it;

  if (infos.it == infos.end || *infos.it != '"')
    return false;
  std::string tmpString;

  ++infos.it;
  for (;infos.it != infos.end && *infos.it != '"';)
  {
    if (*infos.it == '\\')
    {
      if (infos.it + 1 == infos.end)
      {
        infos.it = save;
        return false;
      }
      switch (*(infos.it + 1))
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
        infos.it = save;
        return false;
      }
      infos.it += 2;
    }
    else
    {
      tmpString += *infos.it;
      ++infos.it;
    }
  }
  if (infos.it == infos.end)
  {
    infos.it = save;
    return false;
  }
  ++infos.it;
  result = tmpString;
  return true;
}

static bool decodeString(JSONParseInfos &infos, AnyValue &value)
{
  std::string tmpString;

  if (!getCleanString(infos, tmpString))
    return false;
  value = AnyValue(tmpString);
  return true;
}

static bool decodeObject(JSONParseInfos &infos, AnyValue &value)
{
  std::string::const_iterator save = infos.it;

  if (infos.it == infos.end || *infos.it != '{')
    return false;
  ++infos.it;

  std::map<std::string, AnyValue> tmpMap;
  while (true)
  {
    skipWhiteSpaces(infos);
    std::string key;

    if (!getCleanString(infos, key))
      break;
    skipWhiteSpaces(infos);
    if (infos.it == infos.end || *infos.it != ':')
    {
      infos.it = save;
      return false;
    }
    ++infos.it;
    AnyValue tmpValue;
    if (!decodeValue(infos, tmpValue))
    {
      infos.it = save;
      return false;
    }
    if (infos.it == infos.end)
      break;
    tmpMap[key] = tmpValue;
    if (*infos.it != ',')
      break;
    ++infos.it;
  }
  if (infos.it == infos.end || *infos.it != '}')
  {
    infos.it = save;
    return false;
  }
  ++infos.it;
  value = AnyValue(tmpMap);
  return true;
}

static bool match(JSONParseInfos &infos, std::string const& expected)
{
  std::string::const_iterator save = infos.it;
  std::string::const_iterator begin = expected.begin();

  while (infos.it != infos.end && begin != expected.end())
  {
    if (*infos.it != *begin)
    {
      infos.it = save;
      return false;
    }
    ++infos.it;
    ++begin;
  }
  if (begin != expected.end())
    return false;
  return true;
}

static bool decodeSpecial(JSONParseInfos &infos, AnyValue &value)
{
  if (infos.it == infos.end)
    return false;
  if (match(infos, "true"))
    value = AnyValue(true);
  else if (match(infos, "false"))
    value = AnyValue::from(false);
  else if (match(infos, "null"))
    value = AnyValue();
  else
    return false;
  return true;
}

static bool decodeValue(JSONParseInfos &infos, AnyValue &value)
{
  skipWhiteSpaces(infos);
  if (decodeSpecial(infos, value)
      || decodeString(infos, value)
      || decodeFloat(infos, value)
      || decodeInteger(infos, value)
      || decodeArray(infos, value)
      || decodeObject(infos, value)
      )
  {
    skipWhiteSpaces(infos);
    return true;
  }
  return false;
}

std::string::const_iterator decodeJSON(std::string::const_iterator begin,
                                       std::string::const_iterator end,
                                       AnyValue &target)
{
  JSONParseInfos infos(begin, end);
  if (!decodeValue(infos, target))
    throw std::runtime_error("parse error");
  return infos.it;
}

AnyValue decodeJSON(const std::string &in)
{
  AnyValue value;
  decodeJSON(in.begin(), in.end(), value);
  return value;
}

}

