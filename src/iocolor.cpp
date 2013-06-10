/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <qi/iocolor.hpp>
#include <iostream>
#include <sstream>
#include <qi/os.hpp>

static std::string makeCol(char c, char modifier = -1) {
  std::stringstream ret;
  ret << "\033[" << (int)c;
  if (modifier > 0)
    ret << ";" << (int)modifier;
  ret << "m";
  return ret.str();
}

static void posix_print(std::ostream& os, qi::StreamColor col) {
  switch (col) {
  case qi::StreamColor_None:
    break;
  case qi::StreamColor_Reset:
    os << makeCol(0);
    break;
  case qi::StreamColor_Bold:
    os << makeCol(1);
    break;
  case qi::StreamColor_Faint:
    os << makeCol(2);
    break;
  case qi::StreamColor_Standout:
    os << makeCol(3);
    break;
  case qi::StreamColor_Underline:
    os << makeCol(4);
    break;
  case qi::StreamColor_Blink:
    os << makeCol(5);
    break;
  case qi::StreamColor_Overline:
    os << makeCol(6);
    break;

  case qi::StreamColor_Black:
    os << makeCol(30, 0);
    break;
  case qi::StreamColor_DarkRed:
    os << makeCol(31, 0);
    break;
  case qi::StreamColor_DarkGreen:
    os << makeCol(32, 0);
    break;
  case qi::StreamColor_Brown:
    os << makeCol(33, 0);
    break;
  case qi::StreamColor_DarkBlue:
    os << makeCol(34, 0);
    break;
  case qi::StreamColor_Purple:
    os << makeCol(35, 0);
    break;
  case qi::StreamColor_Teal:
    os << makeCol(36, 0);
    break;
  case qi::StreamColor_LightGray:
    os << makeCol(37, 0);
    break;

  case qi::StreamColor_DarkGray:
    os << makeCol(30, 1);
    break;
  case qi::StreamColor_Red:
    os << makeCol(31, 1);
    break;
  case qi::StreamColor_Green:
    os << makeCol(32, 1);
    break;
  case qi::StreamColor_Yellow:
    os << makeCol(33, 1);
    break;
  case qi::StreamColor_Blue:
    os << makeCol(34, 1);
    break;
  case qi::StreamColor_Fuchsia:
    os << makeCol(35, 1);
    break;
  case qi::StreamColor_Turquoise:
    os << makeCol(36, 1);
    break;
  case qi::StreamColor_White:
    os << makeCol(37, 1);
    break;
  };
}

namespace std {

  std::ostream& operator<<(std::ostream& os, qi::StreamColor col)
  {
    if (os == std::cout && !qi::os::isatty(1))
      return os;
    if (os == std::cerr && !qi::os::isatty(2))
      return os;
#ifdef __linux__
    posix_print(os, col);
#endif
    return os;
  }

}
