/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_IOCOLOR_HPP_
# define _QI_IOCOLOR_HPP_

# include <qi/api.hpp>
# include <ostream>

namespace qi {

  /**
   * \brief The Stream Color enum.
   * \warning Only working under POSIX.
   * \includename{qi/iocolor.hpp}
   */
  enum StreamColor {
    //no color
    StreamColor_None      = 0,  ///< No Color

    //attributes control
    StreamColor_Reset     = 1,  ///< Reset the color and mode
    StreamColor_Bold      = 2,  ///< Bold mode
    StreamColor_Faint     = 3,  ///< Faint mode
    StreamColor_Standout  = 4,  ///< Standout mode
    StreamColor_Underline = 5,  ///< Underline mode
    StreamColor_Blink     = 6,  ///< Blink mode
    StreamColor_Overline  = 7,  ///< Overline mode

    //dark colors
    StreamColor_Black     = 8,  ///< Black
    StreamColor_DarkRed   = 9,  ///< Dark Red
    StreamColor_DarkGreen = 10, ///< Dark Green
    StreamColor_Brown     = 11, ///< Brown
    StreamColor_DarkBlue  = 12, ///< Dark Blue
    StreamColor_Purple    = 13, ///< Purple
    StreamColor_Teal      = 14, ///< Teal
    StreamColor_LightGray = 15, ///< LightGray

    //light colors
    StreamColor_DarkGray  = 16, ///< Dark Gray
    StreamColor_Red       = 17, ///< Red
    StreamColor_Green     = 18, ///< Green
    StreamColor_Yellow    = 19, ///< Yellow
    StreamColor_Blue      = 20, ///< Blue
    StreamColor_Fuchsia   = 21, ///< Fuchsia
    StreamColor_Turquoise = 22, ///< Turquoise
    StreamColor_White     = 23  ///< White
  };
}

namespace std {

  /**
   * \brief This operator (<<) applied to an output stream is known as insertion operator.
   * \param os Output stream.
   * \param col Enum color.
   */
  QI_API std::ostream& operator<<(std::ostream& os, qi::StreamColor col);
}

#endif  // _QI_IOCOLOR_HPP_

