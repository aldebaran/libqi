#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_JSONCODEC_HPP_
#define _QI_TYPE_JSONCODEC_HPP_

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>

namespace qi {

  // Do not use enum here because we want to pipe those values and we don't want to cast them each time we pipe them
  using JsonOption = int;
  const unsigned int JsonOption_None = 0;
  const unsigned int JsonOption_PrettyPrint = 1;
  const unsigned int JsonOption_Expand = 2;

  /** @return the value encoded in JSON.
   * @param val Value to encode
   * @param jsonPrintOption Option to change JSON output
   */
  QI_API std::string encodeJSON(const qi::AutoAnyReference &val, JsonOption jsonPrintOption = JsonOption_None);

  /**
    * creates a GV representing a JSON string or throw on parse error.
    * @param in JSON string to decode.
    * @return a GV representing the JSON string
    */
  QI_API qi::AnyValue decodeJSON(const std::string &in);

  /**
    * set the input GV to represent the JSON sequence between two string iterators or throw on parse error.
    * @param begin iterator to the beginning of the sequence to decode.
    * @param end iterator to the end of the sequence to decode.
    * @param target GV to set. Not modified if an error occured.
    * @return an iterator to the last read char + 1
    */
  QI_API std::string::const_iterator decodeJSON(const std::string::const_iterator &begin,
                                         const std::string::const_iterator &end,
                                         AnyValue &target);



}

#endif  // _QITYPE_JSONCODEC_HPP_
