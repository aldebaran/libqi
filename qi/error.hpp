/**
 * Author(s):
 *  - Herve CUCHE <hcuche@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief
 */

#pragma once
#ifndef _LIBQI_QI_ERROR_HPP_
#define _LIBQI_QI_ERROR_HPP_

# include <qi/macro.hpp>

# include <stdexcept>
# include <string>

namespace qi {
  namespace os {
    /** \class QiException error.hpp "qi/error.hpp"
     *  \brief Custom exception that may be thrown by QI methods.
     */
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable : 4251 )
#endif

    QI_API class QiException : public std::runtime_error
    {
    public:
      /**
       * \brief Constructor
       *
       * Create a message exception.
       *
       * \param message Exception message.
       */
      explicit QiException(const std::string &message)
        : std::runtime_error(message)
        , _message(message)
      {}

      /**
       * \brief Copy constructor.
       * \param message Exception message.
       */
      QiException(const QiException &e)
        : std::runtime_error(e.what())
      {}

      /** \brief Destructor */
      virtual ~QiException() throw()
      {}

      /** \brief Get the error message. */
      inline virtual const char* what() const throw ()
      {
        return _message.c_str();
      }

    private:
      /**
       * \var _message
       * \brief Exception message.
       */
      std::string _message;
#ifdef _MSC_VER
# pragma warning( pop )
#endif
    };
  }
}

#endif  // _LIBQI_QI_ERROR_HPP_
