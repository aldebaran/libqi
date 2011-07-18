#pragma once
/**
 * Author(s):
 *  - Herve CUCHE <hcuche@aldebaran-robotics.com>
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief find bin/lib/data/conf in a 'qi' standard sdk layout
 */

#ifndef _QI_SDKLAYOUT_HPP_
#define _QI_SDKLAYOUT_HPP_

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
  /** \class PathException sdklayout.hpp "qi/path/sdklayout.hpp"
   *  \brief Custom exception that may be thrown by these methods.
   */
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable : 4251 )
#endif

  class QI_API PathException : public std::exception
  {
  public:
    /** \brief Constructor
     *
     *  Create a message exception.
     *
     *  \param message Exception message.
     */
    explicit PathException(const std::string &message)
      : _message(message)
    {}

    /** \brief Destructor */
    virtual ~PathException() throw()
    {}

    /** \brief Get the error message. */
    inline virtual const char* what() const throw ()
    {
      return _message.c_str();
    }

  private:
    /** \brief No default constructor */
    PathException()
     : _message()
    {}

    /** \var _message
     *  \brief Exception message.
     */
    std::string _message;
#ifdef _MSC_VER
# pragma warning( pop )
#endif
  };


  class PrivateSDKLayout;

  /** \class SDKLayout sdklayout.hpp "qi/path/sdklayout.hpp"
   *  \brief A class to handle SDK Layouts.
   *
   *  This class allow to get various path information:
   *   - user writable data and configuration path
   *   - library and binary path
   *   - static readonly configuration and data files
   */
  class QI_API SDKLayout //: qi::noncopyable
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
     *  \param mode "" by default, could be DEBUG or RELEASE when running in the build folder under MSVC.
     */
    explicit SDKLayout(const std::string &prefix, const std::string &mode = "");

    SDKLayout(const SDKLayout &rhs);
    const SDKLayout &operator=(const SDKLayout &rhs);

    virtual ~SDKLayout();



    /** @copydoc qi::path::getSdkPrefix */
    std::string getSdkPrefix() const;

    /** @copydoc qi::path::getSdkPrefixes */
    std::vector<std::string> getSdkPrefixes() const;

    /** @copydoc qi::path::addOptionalSdkPrefix */
    void addOptionalSdkPrefix(const char *prefix);

    /** @copydoc qi::path::clearOptionalSdkPrefix */
    void clearOptionalSdkPrefix();



    /** @copydoc qi::path::findBinary */
    std::string findBinary(const std::string &name) const;

    /** @copydoc qi::path::findLibrary */
    std::string findLibrary(const std::string &name) const;

    /** @copydoc qi::path::findConfiguration */
    std::string findConfiguration(const std::string &applicationName,
                                  const std::string &filename) const;

    /** @copydoc qi::path::findData */
    std::string findData(const std::string &applicationName,
                         const std::string &filename) const;



    /** @copydoc qi::path::getConfigurationPaths */
    std::vector<std::string> getConfigurationPaths(const std::string &applicationName="") const;

    /** @copydoc qi::path::getDataPaths */
    std::vector<std::string> getDataPaths(const std::string &applicationName="") const;

    /** @copydoc qi::path::getBinaryPaths */
    std::vector<std::string> getBinaryPaths() const;

    /** @copydoc qi::path::getLibraryPaths */
    std::vector<std::string> getLibraryPaths() const;



    /** @copydoc qi::path::getUserWritableDataPath */
    std::string getUserWritableDataPath(const std::string &applicationName,
                                        const std::string &filename) const;

    /** @copydoc qi::path::getUserWritableConfigurationPath */
    std::string getUserWritableConfigurationPath(const std::string &applicationName,
                                                 const std::string &filename="") const;

  private:
    // Pimpl
    PrivateSDKLayout* _private;
  };

}

#endif  // _QI_SDKLAYOUT_HPP_
