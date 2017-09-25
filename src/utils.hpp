#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _SRC_UTILS_HPP_
#define _SRC_UTILS_HPP_

# include <string>
# include <vector>
# include <type_traits>
# include <qi/type/traits.hpp>

std::string fsconcat(const std::vector<std::string>& paths);

template <typename... S,
          typename = qi::traits::EnableIf<
              qi::traits::Conjunction<std::is_convertible<S, std::string>...>::value>>
std::string fsconcat(S&&... paths)
{
  return fsconcat(
      std::vector<std::string>{ std::initializer_list<std::string>{ std::forward<S>(paths)... } });
}

std::string randomstr(std::string::size_type sz);
std::wstring wrandomstr(std::wstring::size_type sz);

#endif  // _SRC_UTILS_HPP_
