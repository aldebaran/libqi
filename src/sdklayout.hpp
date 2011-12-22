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

/** @file
 *  @brief find bin/lib/data/conf in a 'qi' standard sdk layout
 */

#pragma once
#ifndef _LIBQI_QI_PATH_SDKLAYOUT_HPP_
#define _LIBQI_QI_PATH_SDKLAYOUT_HPP_

# include <vector>
# include <string>
# include <qi/config.hpp>

/**
 * \namespace qi
 * \brief SDKLayout implementation.
 *
 * \warning Every string MUST be encoded in UTF8 and return UTF-8.
 *
 */
namespace qi
{

  class PrivateSDKLayout;

  /** \class SDKLayout sdklayout.hpp "qi/path/sdklayout.hpp"
   *  \brief A class to handle SDK Layouts.
   *
   *  This class allow to get various path information:
   *   - user writable data and configuration path
   *   - library and binary path
   *   - static readonly configuration and data files
   */
  class QI_API SDKLayout //: boost::noncopyable
  {
  public:

    /** \brief This constructor use argc/argv stored by qi::init(argc, argv)
     *  to determine the sdk layout.
     *
     *  If qi::init has not been called before creating this class,
     *  call to methods of this class will throw an exception.
     */
    SDKLayout();

    /** \brief Construct a SDKLayout using prefix as the sdk prefix.
     *  \param prefix a valid sdk prefix.
     *  \param mode "" by default, use to check sdk initialization.
     */
    explicit SDKLayout(const std::string &prefix, const std::string &mode = "");

    SDKLayout(const SDKLayout &rhs);
    SDKLayout &operator=(const SDKLayout &rhs);

    virtual ~SDKLayout();



    /** @copydoc qi::path::getSdkPrefix */
    std::string sdkPrefix() const;

    /** @copydoc qi::path::getSdkPrefixes */
    std::vector<std::string> getSdkPrefixes() const;

    /** @copydoc qi::path::addOptionalSdkPrefix */
    void addOptionalSdkPrefix(const char *prefix);

    /** @copydoc qi::path::clearOptionalSdkPrefix */
    void clearOptionalSdkPrefix();



    /** @copydoc qi::path::findBinary */
    std::string findBin(const std::string &name) const;

    /** @copydoc qi::path::findLibrary */
    std::string findLib(const std::string &name) const;

    /** @copydoc qi::path::findConfiguration */
    std::string findConf(const std::string &applicationName,
                                  const std::string &filename) const;

    /** @copydoc qi::path::findData */
    std::string findData(const std::string &applicationName,
                         const std::string &filename) const;



    /** @copydoc qi::path::getConfigurationPaths */
    std::vector<std::string> confPaths(const std::string &applicationName="") const;

    /** @copydoc qi::path::getDataPaths */
    std::vector<std::string> dataPaths(const std::string &applicationName="") const;

    /** @copydoc qi::path::getBinaryPaths */
    std::vector<std::string> binPaths() const;

    /** @copydoc qi::path::getLibraryPaths */
    std::vector<std::string> libPaths() const;



    /** @copydoc qi::path::getUserWritableDataPath */
    std::string userWritableDataPath(const std::string &applicationName,
                                        const std::string &filename) const;

    /** @copydoc qi::path::getUserWritableConfigurationPath */
    std::string userWritableConfPath(const std::string &applicationName,
                                                 const std::string &filename="") const;

  private:
    // Pimpl
    PrivateSDKLayout* _private;
  };

}

#endif  // _LIBQI_QI_PATH_SDKLAYOUT_HPP_
