/*
*  Author(s):
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
*/

#include <iostream>
#include <boost/filesystem.hpp>

#include <qi/qi.hpp>
#include <qimessaging/file.hpp>

namespace qi
{
  class PrivateFile
  {
  public:
    PrivateFile()
      : _path("")
      , _flags(qi::Flag_Invalid)
    {
    };

  public:
    std::string _path;
    int         _flags;
  };

  File::File()
    : _p(new PrivateFile())
  {
  }

  File::~File()
  {
    delete _p;
  }

  const std::string &File::path()
  {
    return _p->_path;
  }

  int File::flags()
  {
    return _p->_flags;
  }

  bool File::open(int flags)
  {
    _p->_flags = flags;
    return true;
  }

  bool File::open(const std::string &path,
                  int                flags)
  {
    _p->_flags = flags;
    _p->_path = path;

    return boost::filesystem::exists(path);
  }

  bool File::save(const std::string &dst)
  {
    try
    {
      boost::filesystem::path filePath(_p->_path, qi::unicodeFacet());
      boost::filesystem::path oldFilePath(dst, qi::unicodeFacet());

      // Here, we need to be careful with windows and UTF-16 local encoding.
#ifndef _WIN32
      boost::filesystem::copy_file(filePath.make_preferred().string(qi::unicodeFacet()),
                                   oldFilePath.make_preferred().string(qi::unicodeFacet()),
                                   boost::filesystem::copy_option::overwrite_if_exists);
#else
      boost::filesystem::copy_file(filePath.make_preferred().wstring(qi::unicodeFacet()),
                                   oldFilePath.make_preferred().wstring(qi::unicodeFacet()),
                                   boost::filesystem::copy_option::overwrite_if_exists);
#endif
    }
    catch (...)
    {
      return false;
    }
    return true;
  }
}
