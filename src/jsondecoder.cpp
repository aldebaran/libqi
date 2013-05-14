#include <cerrno>
#include <qitype/type.hpp>

namespace qi {

static bool decodeValue(std::string::const_iterator &it, std::string::const_iterator &end, GenericValue &value);

static void skipWhiteSpaces(std::string::const_iterator &it, std::string::const_iterator &end)
{
  while (it != end && *it == ' ')
    ++it;
}

static bool getDigits(std::string::const_iterator &it, std::string::const_iterator &end, std::string &result)
{
  std::string::const_iterator begin = it;

  while (it != end && *it >= '0' && *it <= '9')
    ++it;
  if (it == begin)
    return false;
  result = std::string(begin, it);
  return true;
}

static bool getInteger(std::string::const_iterator &it, std::string::const_iterator &end, std::string &result)
{
  std::string::const_iterator save = it;
  std::string integerStr;

  if (it == end)
    return false;
  if (*it == '-')
  {
    ++it;
    integerStr = "-";
  }
  std::string digitsStr;

  if (!getDigits(it, end, digitsStr))
  {
    it = save;
    return false;
  }
  integerStr += digitsStr;
  result = integerStr;
  return true;
}

static bool getInteger(std::string::const_iterator &it, std::string::const_iterator &end, qi::int64_t &result)
{
  std::string integerStr;

  if (!getInteger(it, end, integerStr))
    return false;
  result = ::atol(integerStr.c_str());
  return true;
}

static bool getExponent(std::string::const_iterator &it, std::string::const_iterator &end, std::string &result)
{
  std::string::const_iterator save = it;

  if (it == end)
    return false;
  if (*it != 'e' && *it != 'E')
    return false;
  ++it;
  std::string exponentStr;

  exponentStr += 'e';
  if (*it == '+' || *it == '-')
  {
    exponentStr += *it;
    ++it;
  }
  else
    exponentStr += '+';
  std::string integerStr;
  if (!getDigits(it, end, integerStr))
  {
    it = save;
    return false;
  }
  result = exponentStr + integerStr;
  return true;
}

static bool getFloat(std::string::const_iterator &it, std::string::const_iterator &end, double &result)
{
  std::string floatStr;
  std::string beforePoint;
  std::string afterPoint;
  std::string exponent;
  std::string::const_iterator save = it;

  if (!getInteger(it, end, beforePoint))
    return false;
  if (!getExponent(it, end, exponent))
  {
    if (*it != '.')
    {
      it = save;
      return false;
    }
    ++it;
    if (!getDigits(it, end, afterPoint))
    {
      it = save;
      return false;
    }
    getExponent(it, end, exponent);
    floatStr = beforePoint + "." + afterPoint + exponent;
  }
  else
    floatStr = beforePoint + exponent;
  result = ::atof(floatStr.c_str());
  return true;
}

static bool decodeArray(std::string::const_iterator &it, std::string::const_iterator &end, GenericValue &value)
{
  std::string::const_iterator save = it;

  if (it == end)
    return false;
  if (*it != '[')
    return false;
  ++it;
  std::vector<GenericValue>   tmpArray;

  while (true)
  {
    GenericValue subElement;

    if (!decodeValue(it, end, subElement))
      break;
    tmpArray.push_back(subElement);
    skipWhiteSpaces(it, end);
    if (*it != ',')
      break;
    ++it;
  }
  if (*it != ']')
  {
    it = save;
    return false;
  }
  ++it;
  value = GenericValue(tmpArray);
  return true;
}

static bool decodeFloat(std::string::const_iterator &it, std::string::const_iterator &end, GenericValue &value)
{
  double tmpFloat;

  if (!getFloat(it, end, tmpFloat))
    return false;
  value = GenericValue(tmpFloat);
  return true;
}

static bool decodeInteger(std::string::const_iterator &it, std::string::const_iterator &end, GenericValue &value)
{
  qi::int64_t tmpInteger;

  if (!getInteger(it, end, tmpInteger))
    return false;
  value = GenericValue(tmpInteger);
  return true;
}

static bool getCleanString(std::string::const_iterator &it, std::string::const_iterator &end, std::string &result)
{
  std::string::const_iterator save = it;

  if (it == end || *it != '"')
    return false;
  std::string tmpString;

  ++it;
  for (;it != end && *it != '"';)
  {
    if (*it == '\\')
    {
      if (it + 1 == end)
      {
        it = save;
        return false;
      }
      switch (*(it + 1))
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
        it = save;
        return false;
      }
      it += 2;
    }
    else
    {
      tmpString += *it;
      ++it;
    }
  }
  if (it == end)
  {
    it = save;
    return false;
  }
  ++it;
  result = tmpString;
  return true;
}

static bool decodeString(std::string::const_iterator &it, std::string::const_iterator &end, GenericValue &value)
{
  std::string tmpString;

  if (!getCleanString(it, end, tmpString))
    return false;
  value = GenericValue(tmpString);
  return true;
}

static bool decodeValue(std::string::const_iterator &it, std::string::const_iterator &end, GenericValue &value)
{
  skipWhiteSpaces(it, end);
  if (decodeString(it, end, value)
      || decodeFloat(it, end, value)
      || decodeInteger(it, end, value)
      || decodeArray(it, end, value)
      )
    return true;
  return false;
}

GenericValue decodeJSON(const std::string &in)
{
  GenericValue value;
  std::string::const_iterator begin = in.begin();
  std::string::const_iterator end = in.end();

  if (decodeValue(begin, end, value) && begin == end)
    return value;
  throw std::runtime_error("parse error");
}

}

