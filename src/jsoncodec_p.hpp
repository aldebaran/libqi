#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _JSONPARSER_P_HPP_
# define _JSONPARSER_P_HPP_

# include <string>
# include <qitype/anyvalue.hpp>
# include <qitype/jsoncodec.hpp>

namespace qi {

  class JsonDecoderPrivate
  {
  private:
    static const std::string  _strictJsonTokenSet;
    static const std::string  _permissiveJsonTokenSet;

  public:
    JsonDecoderPrivate(const std::string &in);
    JsonDecoderPrivate(const std::string::const_iterator &begin,
                      const std::string::const_iterator &end);
    std::string::const_iterator decode(AnyValue &out, JSONRule rule=JSONRule_Strict);

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

    bool isJsonToken(const std::string::const_iterator &it) const;
    bool isEndOfJsonString(const std::string::const_iterator &it) const;
    bool isBeginningOfJsonString(const std::string::const_iterator &it) const;


  private:
    std::string::const_iterator const _begin;
    std::string::const_iterator const _end;
    std::string::const_iterator       _it;
    JSONRule                          _rule;
  };

}

#endif  // _JSONPARSER_P_HPP_
