/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#ifndef _LIBQI_QI_EXCEPTION_HPP_
#define _LIBQI_QI_EXCEPTION_HPP_

#include <stdexcept>

namespace qi
{
  /// @brief Thrown generic exception for i/o errors.
  class ioerror : public std::runtime_error
  {
  public:
    ioerror(const std::string& what) : std::runtime_error(what) { }
  };

  /// @brief Thrown on read errors.
  class ioreaderror : public ioerror
  {
  public:
    ioreaderror(const std::string& what) : ioerror(what) { }
  };

  /// @brief Thrown on write errors.
  class iowriteerror : public ioerror
  {
  public:
    iowriteerror(const std::string& what) : ioerror(what) { }
  };

  /// @brief Thrown on expected / observed format mismatch.
  class corrupted : public std::runtime_error
  {
  public:
    corrupted(const std::string& what) : std::runtime_error(what) { }
  };
}

#endif // _LIBQI_QI_EXCEPTION_HPP_
