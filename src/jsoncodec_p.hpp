#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _JSONPARSER_P_HPP_
# define _JSONPARSER_P_HPP_

# include <string>
# include <qitype/anyvalue.hpp>

namespace qi {

  class JsonDecoderPrivate
  {
  public:
    JsonDecoderPrivate(const std::string &in);
    JsonDecoderPrivate(const std::string::const_iterator &begin,
                      const std::string::const_iterator &end);
    std::string::const_iterator decode(AnyValue &out);

  private:
    void skipWhiteSpaces();
    bool getDigits(std::string &result);
    bool getInteger(std::string &result);
    bool getInteger(qi::int64_t &result);
    bool getExponent(std::string &result);
    bool getFloat(double &result);
    bool decodeArray(AnyValue &value);
    bool decodeFloat(AnyValue &value);
    bool decodeInteger(AnyValue &value);
    bool getCleanString(std::string &result);
    bool decodeString(AnyValue &value);
    bool decodeObject(AnyValue &value);
    bool match(std::string const& expected);
    bool decodeSpecial(AnyValue &value);
    bool decodeValue(AnyValue &value);

  private:
    std::string::const_iterator const _begin;
    std::string::const_iterator const _end;
    std::string::const_iterator       _it;
  };

}

#endif  // _JSONPARSER_P_HPP_
