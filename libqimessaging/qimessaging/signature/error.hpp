#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2011 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_ERROR_HPP_
#define _QI_SIGNATURE_ERROR_HPP_

#include <qimessaging/api.hpp>
#include <stdexcept>

namespace qi {

  /** \class BadSignatureError signature.hpp "qi/signature.hpp"
   *  \brief Thrown when a signature is invalid
   */
  class QIMESSAGING_API BadSignatureError : public std::runtime_error
  {
  public:
    /**
     * \brief Constructor
     * Create a message exception.
     * \param message Exception message.
     */
    explicit BadSignatureError(const std::string &message)
      : std::runtime_error(message)
    {}

    /** \brief Destructor */
    virtual ~BadSignatureError() throw()
    {}
  };
}

#endif  // _QI_SIGNATURE_ERROR_HPP_
