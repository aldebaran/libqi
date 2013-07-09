/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/types.hpp>
#include <qi/version.hpp>
#include <boost/regex.hpp>
#include <cctype>
#include <locale>

namespace qi {
  class VersionPrivate {
    public:
      VersionPrivate();
      VersionPrivate(const std::string& version);

      int compare(const std::string &versionA, const std::string &versionB);
      std::vector<std::string> explode(const std::string &version);

      std::string _version;

    private:
      int compare_sub(const std::string &subA, const std::string &subB);
      std::string eat_number(const std::string &str, unsigned int &index);
      std::string eat_alpha(const std::string &str, unsigned int &index);
  };

  VersionPrivate::VersionPrivate()
    : _version(std::string())
  { }

  VersionPrivate::VersionPrivate(const std::string& version)
  {
    _version = version;
  }

  int VersionPrivate::compare(const std::string &versionA, const std::string &versionB)
  {
    std::locale              loc("C");
    std::vector<std::string> vA = explode(versionA);
    std::vector<std::string> vB = explode(versionB);
    int                      ret = 0;
    int                      sepA = 0;
    int                      sepB = 0;
    std::string              cA;
    std::string              cB;

    while(1) {
      if (!vA.size())
        cA = std::string();
      else {
        cA = vA.front();
        vA.erase(vA.begin());
      }
      if (!vB.size())
        cB = std::string();
      else {
        cB = vB.front();
        vB.erase(vB.begin());
      }

      if (cA.empty() && cB.empty())
        return 0;
      if (cA.empty())
        return -1;
      if (cB.empty())
        return 1;
      //dont ask why... but that how the python code works
      if (!std::isdigit(cA[0], loc))
        sepA = (cA == "." || cA == "-");
      if (!std::isdigit(cB[0], loc))
        sepB = (cB == "." || cB == "-");
      if (sepA && !sepB)
        return -1;
      if (!sepA && sepB)
        return 1;
      ret = compare_sub(cA, cB);
      if (ret)
        return ret;
    }
    return 0;
  }

  std::vector<std::string> VersionPrivate::explode(const std::string &version)
  {
    std::locale              loc("C");
    std::vector<std::string> result;
    unsigned int             index = 0;

    while (index < version.length()) {
      if (std::isdigit(version[index], loc))
        result.push_back(eat_number(version, index));
      else if (std::isalpha(version[index], loc))
        result.push_back(eat_alpha(version, index));
      else
      {
        result.push_back(std::string(version, index, 1));
        index++;
      }
    }
    return result;
  }

  int VersionPrivate::compare_sub(const std::string &subA, const std::string &subB)
  {
    std::locale loc("C");
    int         digitA;
    int         digitB;

    digitA = std::isdigit(subA[0], loc);
    digitB = std::isdigit(subB[0], loc);

    // string > int
    if (digitA && !digitB)
      return -1;
    if (!digitA && digitB)
      return 1;
    if (digitA && digitB)
    {
      int intA = atoi(subA.c_str());
      int intB = atoi(subB.c_str());
      if (intA > intB)
        return 1;
      if (intA < intB)
        return -1;
    }
    else
    {
      //compare string
      if (subA > subB)
        return 1;
      if (subA < subB)
        return -1;
    }
    return 0;
  }

  std::string VersionPrivate::eat_number(const std::string &str, unsigned int &index)
  {
    std::locale              loc("C");
    unsigned int             first = index;

    while (index < str.length())
    {
      if (!std::isdigit(str[index], loc))
        break;
      index++;
    }
    return std::string(str, first, index - first);
  }

  std::string VersionPrivate::eat_alpha(const std::string &str, unsigned int &index)
  {
    std::locale              loc("C");
    unsigned int             first = index;

    while (index < str.length())
    {
      if (!std::isalpha(str[index], loc))
        break;
      index++;
    }
    return std::string(str, first, index - first);
  }

  Version::Version()
    :_p(new VersionPrivate)
  { }

  Version::Version(const std::string& version)
    : _p(new VersionPrivate(version))
  { }

  Version::Version(const char* version)
    : _p(new VersionPrivate(version))
  { }

  const std::string& Version::toString() const
  {
    return _p->_version;
  }

  bool Version::operator<(const Version& rhs) const
  {
    return _p->compare(_p->_version, rhs.toString()) < 0;
  }

  bool Version::operator==(const Version& rhs) const
  {
    return !_p->compare(_p->_version, rhs.toString());
  }

  bool Version::operator>(const Version& rhs) const
  {
    const Version& cpi = *this;
    return (cpi != rhs) && !(cpi < rhs);
  }

  bool Version::operator!=(const Version& rhs) const
  {
    const Version& cpi = *this;
    return !(cpi == rhs);
  }

  bool Version::operator<=(const Version& rhs) const
  {
    const Version& cpi = *this;
    return !(cpi > rhs);
  }

  bool Version::operator>=(const Version& rhs) const
  {
    const Version& cpi = *this;
    return !(cpi < rhs);
  }
}
