#pragma once
/*
**  Copyright (C) 2010 Aldebaran Robotics
**  See COPYING for the license
*/


#ifndef _QI_VERSION_HPP_
# define _QI_VERSION_HPP_

// scoped_ptr needs to have dll-interface to be used
# ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
# endif

# include <qi/api.hpp>
# include <vector>
# include <string>
# include <boost/scoped_ptr.hpp>

namespace qi {
  /**
   * \brief Version numbering API.
   * \includename{qi/version.hpp}
   */
  namespace version {

    class VersionPrivate;

    /**
     * \brief Compare version strings
     * Simple class that allow comparing two version number.
     */
    class QI_API Version
    {
    public:
      /// These constructors are implicit by design.
      Version();
      /// Copy constructor.
      Version(const Version &other);
      /// Constructor converting a string.
      Version(const std::string &version);
      /// Constructor converting a char*.
      Version(const char *version);
      /// Destructor.
      ~Version();

      /// Assignation opertator
      Version &operator= (const Version& rhs);

      /// Converting the Version to a String
      operator const std::string&() const;

      /**
       * \brief operator <
       * \param pi
       * \return true if pi is superior
       */
      bool operator< (const Version& pi) const;
      /**
       * \brief operator >
       * \param pi
       * \return true if pi is inferior
       */
      bool operator> (const Version& pi) const;
      /**
       * \brief operator ==
       * \param pi
       * \return true if pi is equal
       */
      bool operator==(const Version& pi) const;
      /**
       * \brief operator !=
       * \param pi
       * \return true if pi is different
       */
      bool operator!=(const Version& pi) const;
      /**
       * \brief operator <=
       * \param pi
       * \return true if pi is superior or equal
       */
      bool operator<=(const Version& pi) const;
      /**
       * \brief operator >=
       * \param pi
       * \return true if pi is inferior or equal
       */
      bool operator>=(const Version& pi) const;

    private:
      boost::scoped_ptr<VersionPrivate> _p;
    };

    /**
     * \brief Explode a version string to an array of strings.
     * \param version The string to explode.
     * \return An array of strings (including ".").
     *
     * for example "1.2.3" become { "1", "2", "3" }
     */
    QI_API std::vector<std::string> explode(const std::string &version);

    /**
     * \brief Compare version numbers.
     * \param versionA Version number.
     * \param versionB Version number.
     * \return like strcmp, 0 if equal, -1 if a < b, 1 if a > b
     */
    QI_API int                      compare(const std::string &versionA,
                                            const std::string &versionB);

    /**
     * \brief Extract version number from string.
     * \param version The string containing the version number.
     * \return The version number, if any.
     */
    QI_API std::string              extract(const std::string &version);
  }
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QI_VERSION_HPP_
