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
  namespace version {

    static std::string eat_number(const std::string &str, unsigned int &index)
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

    static std::string eat_alpha(const std::string &str, unsigned int &index)
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

    ///split a version string into a list
    std::vector<std::string> explode(const std::string &version){
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

    //compare two substring
    static int compare_sub(const std::string &subA, const std::string &subB)
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

    /// compare two version
    int compare(const std::string &versionA, const std::string &versionB){
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

    std::string extract(const std::string &version) {
      //use cryptic var name for regex
      boost::regex sverptn("(([0-9]+)\\.){1,3}([0-9]+)((-rc[0-9]+)|(-oe[0-9]+)){0,1}");
      boost::match_results<std::string::const_iterator> what;
      if (boost::regex_search(version, what, sverptn))
        return what[0];
      return std::string();
    }

    std::string& Version::operator()()
    {
      return version;
    }

    const std::string& Version::operator()() const
    {
      return version;
    }

    bool Version::operator<(const Version& pi) const
    {
      return qi::version::compare(version, pi.version) < 0;
    }

    bool Version::operator==(const Version& pi) const
    {
      return !qi::version::compare(version, pi.version);
    }

    bool Version::operator>(const Version& pi) const
    {
      const Version& cpi = *this;
      return (cpi != pi) && !(cpi < pi);
    }

    bool Version::operator!=(const Version& pi) const
    {
      const Version& cpi = *this;
      return !(cpi == pi);
    }

    bool Version::operator<=(const Version& pi) const
    {
      const Version& cpi = *this;
      return !(cpi > pi);
    }

    bool Version::operator>=(const Version& pi) const
    {
      const Version& cpi = *this;
      return !(cpi < pi);
    }
  }
}
