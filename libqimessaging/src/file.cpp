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

#define MAX_SIZE_STREAM_CONTENT 1024

namespace qi
{
  class PrivateFile
  {
  public:
    PrivateFile()
      : path("")
      , fileName("")
      , flags(qi::Flag_Invalid)
    {
    };

  public:
    std::string path;
    std::string fileName;
    int         flags;
  };

  File::File()
    : p(new PrivateFile())
  {
  }

  File::~File()
  {
    delete p;
  }

  const std::string &File::path() const
  {
    return p->path;
  }

  const std::string &File::fileName() const
  {
    return p->fileName;
  }

  int File::flags() const
  {
    return p->flags;
  }

  bool File::open(int flags)
  {
    p->flags = flags;
    return true;
  }

  bool File::open(const std::string &path,
                  int                flags)
  {
    p->flags = flags;
    p->path = path;
    p->fileName = boost::filesystem::path(p->path, qi::unicodeFacet()).filename().string(qi::unicodeFacet());

    return boost::filesystem::exists(path);
  }

  bool File::save(const std::string &dst)
  {
    try
    {
      boost::filesystem::path filePath(p->path, qi::unicodeFacet());
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

  qi::DataStream &operator<<(qi::DataStream &stream,
                             const qi::File &sfile)
  {
    stream << sfile.flags();
    stream << sfile.fileName();

    boost::filesystem::path filePath(sfile.p->path, qi::unicodeFacet());
    FILE *f = qi::os::fopen(filePath.string(qi::unicodeFacet()).c_str(), "r");
    int readSize = boost::filesystem::file_size(filePath);
    char buf[readSize + 1];

    readSize = fread(buf, 1, readSize, f);
    buf[readSize] = '\0';
    stream << buf;

    fclose(f);
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream,
                             qi::File &sfile)
  {
    stream >> sfile.p->flags;
    stream >> sfile.p->fileName;

    std::string buffer;
    boost::filesystem::path filePath(qi::os::mktmpdir(),
                                     qi::unicodeFacet());
    filePath.append(sfile.p->fileName, qi::unicodeFacet());
    sfile.p->path = filePath.make_preferred().string(qi::unicodeFacet()).c_str();

    FILE *f = qi::os::fopen(sfile.p->path.c_str(), "w+");
    stream >> buffer;
    fwrite(buffer.c_str(), 1, buffer.size(), f);

    fclose(f);
    return stream;
  }
}
