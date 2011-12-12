/**
 * @author Victor Paleologue
 * @author Jerome Vuarand
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */

#include "project_boost.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <cerrno>
#include <cstring>
#include <sstream>

#include <boost/algorithm/string.hpp>
#ifdef BOOST_FILESYSTEM_VERSION
#undef BOOST_FILESYSTEM_VERSION
#endif
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <qi/path.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>

#include "archiveentryhelper.h"
#include "filehelper.h"
#include "memoryhelper.h"

#ifndef BUFFER_SIZE
#  define BUFFER_SIZE (4 * 1024 * 1024)
#endif

// uncomment next line once we have a libarchive version with bzip2 support
//#define Project_HAS_BZIP2 1

// uncomment next line once we have a libarchive version with zip writing support
//#define Project_HAS_ZIP_WRITING 1

namespace std
{
  /**
   * @brief Implement swap for our pimpl class.
   */
  template<>
  void swap<qi::Project>(qi::Project &a, qi::Project &b) { a.swap(b); }
}

namespace boost
{
  namespace filesystem
  {
    void copy_all_custom(
      path sourcePath
    , path destPath)
    {
      create_directory(destPath);
      directory_iterator itEnd;
      directory_iterator itf;
      for(itf = directory_iterator(sourcePath);
        itf != itEnd; ++itf)
      {
        path lPath = itf->path();
        if(is_directory(lPath))
        {
          copy_all_custom(lPath, destPath / lPath.filename());
        }
        else
        {
          copy_file(lPath, destPath / lPath.filename());
        }
      }
    }
  }
}

namespace bfs = boost::filesystem;

using std::map;
using std::list;
using std::deque;
using std::string;
using std::stringstream;
using boost::shared_ptr;
using bfs::path;

namespace qi
{
  Project::Project() throw()
  : fImpl()
  { }

  Project::Project(const Project &other) throw()
  : fImpl(new ProjectImpl(other.projectPath()))
  { }

  Project::Project(const std::string &projectDir) throw()
  : fImpl(new ProjectImpl(projectDir))
  { }

  Project::~Project() throw()
  {
    delete fImpl;
  }

  Project& Project::operator =(const Project& other) throw()
  {
    Project* p = new Project(other);
    return *p;
  }

  std::string Project::projectPath() const throw()
  {
    return fImpl->projectPath.native();
  }

  Project* Project::create(
    const std::string pathStr
  ) throw(PathNotFound, iowriteerror, AlreadyExists)
  {
    if(pathStr.empty())
    {
      throw PathNotFound(pathStr);
    }

    bfs::path lPath(pathStr);
    if(bfs::exists(lPath))
    {
      throw AlreadyExists(pathStr);
    }

    try
    {
      bfs::create_directories(lPath);
    }
    catch(const std::string& e)
    {
      throw iowriteerror(pathStr);
    }

    ProjectImpl::xWriteMetadata(lPath / ".metadata",
                                  std::map<std::string, std::string>());

    return new Project(pathStr);
  }

  bool Project::valid() const throw()
  {
    return bfs::exists(projectPath()) && bfs::is_directory(projectPath());
  }

  std::map<std::string, std::string> Project::getMetadata() const throw()
  {
    try
    {
      return ProjectImpl::xReadMetadata(fImpl->projectPath / ".metadata");
    }
    catch(const std::exception&)
    {
      return std::map<std::string, std::string>();
    }
  }

  std::string Project::getMetadata(const std::string &key) const throw()
  {
    return getMetadata()[key];
  }

  void Project::setMetadata(
    const std::map<std::string, std::string> &metadata
  ) throw(iowriteerror)
  {
    ProjectImpl::xWriteMetadata(fImpl->projectPath / ".metadata", metadata);
  }

  void Project::setMetadata(
    const std::string& key
  , const std::string& value
  ) throw(iowriteerror)
  {
    std::map<std::string, std::string> metadata = getMetadata();
    metadata[key] = value;
    setMetadata(metadata);
  }

  void Project::importProject(
    const std::string& sourcePath
  , const std::string& destPath
  , const ArchiveFormat& requestedFormat
  ) throw(PathNotFound, ioerror,
          UnsupportedFormat, CorruptedFile)
  {
    if(!bfs::exists(sourcePath))
    {
      throw PathNotFound(sourcePath);
    }

    if(destPath.empty())
    {
      throw PathNotFound(destPath);
    }

    bool destExisted;
    if(!bfs::exists(destPath))
    {
      bfs::path lPath = destPath;
      if(!bfs::exists(lPath.parent_path()))
      {
        throw PathNotFound(lPath.native());
      }
      bfs::create_directory(destPath);
      destExisted = false;
    }
    else
    {
      if(!bfs::is_directory(destPath))
      {
        throw iowriteerror(destPath + " is not a directory.");
      }
      bfs::directory_iterator itEnd;
      if(bfs::directory_iterator(destPath) != itEnd)
      {
        throw iowriteerror(destPath + " already contains files.");
      }
      destExisted = true;
    }

    try
    {
      ArchiveFormat format = requestedFormat;
      if(format == ArchiveFormat_AutoDetect
      || format == ArchiveFormat_Default)
      {
        format = detectFormat(sourcePath);
      }

      switch(format)
      {
      case ArchiveFormat_Directory:
        try
        {
          bfs::copy_all_custom(sourcePath, destPath);
        }
        catch(const std::exception& e)
        {
          throw iowriteerror(destPath + string(": ") + e.what());
        }
        break;

  #if Project_HAS_ZIP_WRITING
      case ArchiveFormat_ZIP: // :TODO: implement ZIP with libzip
  #endif

  #if Project_HAS_BZIP2
      case ArchiveFormat_TAR_BZ2:
  #endif

      case ArchiveFormat_TAR:
      case ArchiveFormat_TAR_GZ:
        ProjectImpl::importSimpleArchive(sourcePath, destPath, format);
        break;

      case ArchiveFormat_DEB:
        ProjectImpl::importDebianPackage(sourcePath, destPath);
        break;

      default:
        throw UnsupportedFormat(sourcePath, format);
      }
    }
    catch(const std::exception&)
    {
      if(!destExisted)
      {
        bfs::remove_all(destPath);
        throw;
      }
    }
  }

  void Project::exportProject(
    const std::string &sourcePath
  , const std::string &destPath
  , const ArchiveFormat &format
  ) throw(PathNotFound, ioerror)
  {
    switch(format)
    {
    case ArchiveFormat_Directory:
      try
      {
        bfs::copy_all_custom(sourcePath, destPath);
      }
      catch(const std::exception&)
      {
        throw iowriteerror(destPath);
      }
      break;

#if Project_HAS_BZIP2
    case ArchiveFormat_TAR_BZ2:
#endif

#if Project_HAS_ZIP_WRITING
    case ArchiveFormat_ZIP: // :TODO: implement ZIP with libzip
#endif

    case ArchiveFormat_TAR:
    case ArchiveFormat_TAR_GZ:
      ProjectImpl::exportSimpleArchive(sourcePath, destPath, format);
      break;

    case ArchiveFormat_AutoDetect:
    case ArchiveFormat_Default:
    case ArchiveFormat_DEB:
      ProjectImpl::exportDebianPackage(sourcePath, destPath);
      break;

    default:
      throw UnsupportedFormat(destPath, format);
    }
  }

  Project::ArchiveFormat Project::detectFormat(
    const std::string& projectPath
  ) throw(
    PathNotFound
  , UnsupportedFormat)
  {
    ArchiveFormat format = ArchiveFormat_AutoDetect;

    // Autodetect from file extension and directory flag
    // XXX: Shouldn't it be using only libarchive? (Magic numbers)
    if (bfs::is_directory(projectPath))
      format = ArchiveFormat_Directory;
    else if (bfs::is_regular_file(projectPath))
    {
      string extension = bfs::path(projectPath).extension().native();
      std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
      if(extension == ".tar")
        format = ArchiveFormat_TAR;
      else if(extension == ".crg")
        format = ArchiveFormat_DEB;
      else if(extension == ".zip")
        format = ArchiveFormat_ZIP;
      else if(extension == ".tgz")
        format = ArchiveFormat_TAR_GZ;
  #if Project_HAS_BZIP2
      else if(extension == ".tbz")
        format = Project::ArchiveFormat_TAR_BZ2;
      else if(extension == ".tbz2")
        format = Project::ArchiveFormat_TAR_BZ2;
  #endif
      else if(extension == ".gz")
      {
        bfs::path unzipped = bfs::path(projectPath).parent_path() / bfs::path(projectPath).stem();
        string extension2 = unzipped.extension().native();
        std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
        if (extension2==".tar")
          format = ArchiveFormat_TAR_GZ;
      }
  #if Project_HAS_BZIP2
      else if (extension==".bz2")
      {
        bfs::path unzipped = bfs::path(path).parent_path() / path.stem();
        string extension2 = unzipped.extension().file_string();
        std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
        if (extension2==".tar")
          format = ArchiveFormat_TAR_BZ2;
      }
  #endif
    }

    // Fallback to libarchive if extension does not match
    if(format == ArchiveFormat_AutoDetect)
    {
      struct archive* a = archive_read_new();
      archive_read_support_format_all(a);
      FileHelper data(projectPath);
      int result = archive_read_open(a, static_cast<void*>(&data), FileHelper::openForRead, FileHelper::read, FileHelper::close);
      if (result == ARCHIVE_OK)
      {
        struct archive_entry* entry;
        if (archive_read_next_header(a, &entry) == ARCHIVE_OK)
        {
          int compression = archive_compression(a);
          int archiveFormat = archive_format(a);
          if ((archiveFormat & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_TAR && compression == ARCHIVE_COMPRESSION_NONE)
            format = ArchiveFormat_TAR;
          else if ((archiveFormat & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_TAR && compression == ARCHIVE_COMPRESSION_GZIP)
            format = ArchiveFormat_TAR_GZ;
  #if Project_HAS_BZIP2
          else if ((archiveFormat & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_TAR && compression == ARCHIVE_COMPRESSION_BZIP2)
            format = Project::ArchiveFormat_TAR_BZ2;
  #endif
          else if ((archiveFormat & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_ZIP)
            format = ArchiveFormat_ZIP;
          else if ((archiveFormat & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_AR)
            format = ArchiveFormat_DEB;
        }
        archive_entry_free(entry);
      }
      archive_read_finish(a);
    }

    if(format == ArchiveFormat_AutoDetect)
    {
      throw UnsupportedFormat(projectPath, format);
    }

    return format;
  }

  ProjectImpl::ProjectImpl() throw()
  : projectPath()
  { }

  ProjectImpl::ProjectImpl(const std::string& projectDir) throw()
  : projectPath(projectDir)
  {
    if(!bfs::exists(projectPath))
    {
      qiLogWarning("qi.project") << "No project directory found at "
                                 << projectPath << ". Project instance will "
                                 << "be invalid.";
    }
    else if(!bfs::is_directory(projectPath))
    {
      qiLogWarning("qi.project") << "File found at " << projectPath << " "
                                 << "is not a directory. Please use "
                                 << "importProject to load archived project "
                                 << "directories.";
    }
    else if(!bfs::exists(projectPath / ".metadata"))
    {
      qiLogWarning("qi.project") << "No metadata found." << std::endl
                                 << projectPath << " does not seem to "
                                 << "be a regular project directory.";
    }
  }

  ProjectImpl::~ProjectImpl() throw()
  { }

  std::map<std::string, std::string> ProjectImpl::xReadMetadata(
    const bfs::path &metaPath
  ) throw(
    Project::PathNotFound
  , ioreaderror)
  {
    // Check file existence
    if(!bfs::exists(metaPath))
    {
      throw Project::PathNotFound(metaPath.native());
    }

    // Read file content
    std::string content;
    FILE* metaDataFile = qi::os::fopen(metaPath.c_str(), "rb");
    if(metaDataFile)
    {
      char buffer[fBlocksize];
      size_t read;
      while ((read = fread(buffer, 1, fBlocksize, metaDataFile)) > 0)
      {
        copy(buffer, buffer+read, back_inserter(content));
      }
      fclose(metaDataFile);
    }
    else
    {
      throw ioreaderror(metaPath.native());
    }

    // Parse file content
    std::map<std::string, std::string> metadata;
    std::string strwithoutr = content;
    boost::algorithm::erase_all(strwithoutr, "\r");

    for (
      size_t a = 0, c = strwithoutr.find("\n");
      c != string::npos;
      a = c + 1, c = strwithoutr.find("\n", c + 1))
    {
      size_t b = strwithoutr.find(": ", a);
      if (b != string::npos && b < c)
      {
        string name = strwithoutr.substr(a, b-a);
        string value = strwithoutr.substr(b+2, c-b-2);
        metadata[name] = value;
      }
    }

    return metadata;
  }

  void ProjectImpl::xWriteMetadata(
    const bfs::path& metaPath
  , const std::map<std::string, std::string>& metadata
  ) throw(
    Project::PathNotFound
  , iowriteerror)
  {
    if(!bfs::exists(metaPath.parent_path()))
    {
      throw Project::PathNotFound(metaPath.native());
    }

    FILE* metaDataFile = qi::os::fopen(metaPath.c_str(), "wb");
    if(!metaDataFile)
      throw iowriteerror(metaPath.native());

    for(map<string, string>::const_iterator it = metadata.begin();
        it != metadata.end(); ++it)
    {
      fprintf(metaDataFile, "%s: %s\n", it->first.c_str(), it->second.c_str());
    }
    fclose(metaDataFile);
  }

  const size_t ProjectImpl::fBlocksize = 32 * 1024; // 32kB

  // Supports .tar, .tar.gz and .tar.bz2, .zip upon valid flags.
  void ProjectImpl::exportSimpleArchive(
    const bfs::path& sourceProject
  , const bfs::path& destPath
  , const Project::ArchiveFormat& format
  ) throw(
    Project::PathNotFound
  , ioreaderror
  , iowriteerror)
  {
    // create archive object
    archive* a = archive_write_new();

    // :KLUDGE: has to open the file before setting format, or archive_write_finish may crash
    FileHelper file(destPath);
    int openResult = FileHelper::openForWrite(a, static_cast<void*>(&file));
    if (openResult != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while trying to open archive " << destPath.native() << ": " << archive_error_string(a);
      archive_write_finish(a);
      throw iowriteerror(msg.str());
    }

    // select format
    switch (format)
    {
      case Project::ArchiveFormat_TAR:
      case Project::ArchiveFormat_TAR_GZ:
  #if Project_HAS_BZIP2
      case Project::ArchiveFormat_TAR_BZ2:
  #endif
        if (archive_write_set_format_ustar(a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while setting format to tar: " << archive_error_string(a) << std::endl;
        break;
  #if Project_HAS_ZIP_WRITING
      case Project::ArchiveFormat_ZIP:
        if (archive_write_set_format_zip(a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while setting format to zip: " << archive_error_string(a) << std::endl;
        break;
  #endif
      default:
        throw Project::UnsupportedFormat(destPath.native(), format);
    }

    // set compression
    switch (format)
    {
      case Project::ArchiveFormat_TAR:
        archive_write_set_compression_none(a);
        break;
      case Project::ArchiveFormat_TAR_GZ:
        archive_write_set_compression_gzip(a);
        break;
  #if Project_HAS_BZIP2
      case Project::ArchiveFormat_TAR_BZ2:
        archive_write_set_compression_bzip2(a);
        break;
  #endif
      default:
        throw Project::UnsupportedFormat(destPath.native(), format);
    }

    // open archive for writing
    int result = archive_write_open(a, static_cast<void*>(&file), NULL, FileHelper::write, FileHelper::close);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while trying to open archive " << destPath.native() << ": " << archive_error_string(a);
      archive_write_finish(a);
      throw iowriteerror(msg.str());
    }

    // write each file from the temporary directory in the archive
    std::set<std::string> lFiles = localFiles(sourceProject);
    std::set<std::string>::const_iterator it;
    for(it = lFiles.begin(); it != lFiles.end(); ++it)
    {
      // open file in temporary directory for reading
      bfs::path lPath = sourceProject / *it;
      FILE* fentry = qi::os::fopen(lPath.c_str(), "rb");
      if (!fentry)
      {
        int err = errno;
        stringstream msg;
        msg << "could not open file " << lPath.native() << " for read while saving project: " << strerror(err);
        archive_write_finish(a);
        throw ioreaderror(lPath.native() + ": " + msg.str());
      }

      // create archive entry
      archive_entry* entry = archive_entry_new();

      // configure entry stats
      struct stat st;
      qi::os::stat(lPath.c_str(), &st); // :TODO: handle return value
      archive_entry_set_mode(entry, S_IFREG | 0644);
      archive_entry_set_size(entry, st.st_size);

      // set entry path
      archive_entry_set_pathname(entry, it->c_str());

      // write entry header
      result = archive_write_header(a, entry);
      if (result != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while writing entry " << *it << " header: " << archive_error_string(a);
        fclose(fentry);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // copy file content into archive entry
      char buffer[fBlocksize];
      size_t len = fread(buffer, 1, fBlocksize, fentry);
      while (len > 0)
      {
        __LA_SSIZE_T written = archive_write_data(a, buffer, len);
        if (written < (__LA_SSIZE_T)len)
        {
          stringstream msg;
          msg << "error while writing entry " << *it << ": " << archive_error_string(a);
          fclose(fentry);
          archive_entry_free(entry);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }
        len = fread(buffer, 1, fBlocksize, fentry);
      }
      if (len < fBlocksize && ferror(fentry))
      {
        int err = errno;
        stringstream msg;
        msg << "error while reading entry " << lPath.native() << " file: " << strerror(err);
        fclose(fentry);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw ioreaderror(msg.str());
      }

      //archive_write_finish_entry(a); // not necessary

      // free entry object
      archive_entry_free(entry);

      // close entry file
      result = fclose(fentry);
      if (result != 0)
      {
        int err = errno;
        stringstream msg;
        msg << "error while closing entry " << *it << " file: " << strerror(err);
        throw ioreaderror(msg.str());
      }
    }

    // close archive
    result = archive_write_finish(a);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while closing project archive " << destPath.native() << ": " << archive_error_string(a);
      throw iowriteerror(msg.str());
    }
  }

  void ProjectImpl::exportDebianPackage(
    const bfs::path& sourceProject
  , const bfs::path& destPath
  ) throw(
    Project::PathNotFound
  , ioreaderror
  , iowriteerror)
  {
    // create main archive object
    struct archive* a = archive_write_new();

    // :KLUDGE: has to open the file before setting format, or archive_write_finish may crash
    FileHelper file(destPath);
    int openResult = FileHelper::openForWrite(a, static_cast<void*>(&file));
    if (openResult != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while trying to open archive " << destPath.native() << ": " << archive_error_string(a);
      archive_write_finish(a);
      throw iowriteerror(msg.str());
    }

    // select format
    if (archive_write_set_format_ar_svr4(a) != ARCHIVE_OK)
      qiLogWarning("behavior.Project") << "error while setting format to ar: " << archive_error_string(a) << std::endl;

    // set compression
    archive_write_set_compression_none(a);

    // open main archive for writing
    int result = archive_write_open(a, static_cast<void*>(&file), NULL, FileHelper::write, FileHelper::close);

    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while trying to open archive " << destPath.native() << ": " << archive_error_string(a);
      archive_write_finish(a);
      throw iowriteerror(msg.str());
    }

  //----------------------------------------------------------------------------
    // write "2.0\n" in debian-binary
    {
      string debian_binary = "2.0\n";

      // create archive entry
      archive_entry* entry = archive_entry_new();

      // configure entry stats
      archive_entry_set_mode(entry, S_IFREG | 0644);
      archive_entry_set_size(entry, debian_binary.size());

      // set entry path
      archive_entry_set_pathname(entry, "debian-binary");

      // write entry header
      result = archive_write_header(a, entry);
      if (result != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while writing CRG debian-binary entry header: " << archive_error_string(a);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // write debian-binary content into archive entry
      __LA_SSIZE_T written = archive_write_data(a, debian_binary.c_str(), debian_binary.size());
      if (written < (__LA_SSIZE_T)debian_binary.size())
      {
        stringstream msg;
        msg << "error while writing CRG debian-binary entry: " << archive_error_string(a);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      //archive_write_finish_entry(a); // not necessary

      // free entry object
      archive_entry_free(entry);
    }

  //----------------------------------------------------------------------------
    // write metadata in control in control.tar.gz in the archive
    {
      string metadatum;
      std::map<std::string, std::string> metadata(
        xReadMetadata(sourceProject / ".metadata"));
      if (metadata.begin() != metadata.end())
      {
        // collect metadata
        stringstream ss;
        for (map<string, string>::const_iterator it=metadata.begin(), itEnd=metadata.end(); it!=itEnd; ++it)
          ss << it->first << ": " << it->second << "\n";
        metadatum = ss.str();
      }

      // build a sub-archive in memory
      {
        // create sub-archive object
        struct archive* sub_a = archive_write_new();

        // :KLUDGE: has to open the file before setting format, or archive_write_finish may crash
        MemoryHelper data;
        int openResult = MemoryHelper::openForWrite(a, static_cast<void*>(&data));
        if (openResult != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while trying to open CRG control.tar.gz sub-archive " << destPath.native() << ": " << archive_error_string(sub_a);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }

        // set compression
        archive_write_set_compression_gzip(sub_a);

        // select format
        if (archive_write_set_format_ustar(sub_a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while setting format to tar: " << archive_error_string(a) << std::endl;

        // open sub-archive for writing
        int result = archive_write_open(sub_a, static_cast<void*>(&data), NULL, MemoryHelper::write, MemoryHelper::close);

        //size_t size = 32*1024*1024, used = 0;
        //unsigned char* data = new unsigned char[size];
        //int result = archive_write_open_memory(sub_a, data, size, &used);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while trying to open CRG control.tar.gz sub-archive " << destPath.native() << ": " << archive_error_string(sub_a);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }

        // create sub archive entry
        archive_entry* sub_entry = archive_entry_new();

        // configure sub-entry stats
        archive_entry_set_mode(sub_entry, S_IFREG | 0644);
        archive_entry_set_size(sub_entry, metadatum.size());

        // set sub-entry path
        archive_entry_set_pathname(sub_entry, "control");

        // write sub-entry header
        result = archive_write_header(sub_a, sub_entry);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while writing CRG control.tar.gz entry header: " << archive_error_string(sub_a);
          archive_entry_free(sub_entry);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }

        // write metadata into sub-archive entry
        __LA_SSIZE_T written = archive_write_data(sub_a, metadatum.c_str(), metadatum.size());
        if (written < (__LA_SSIZE_T)metadatum.size())
        {
          stringstream msg;
          msg << "error while writing CRG control.tar.gz entry: " << archive_error_string(sub_a);
          archive_entry_free(sub_entry);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }

        //archive_write_finish_entry(sub_a); // not necessary

        // free sub-entry object
        archive_entry_free(sub_entry);

        // close sub-archive
        result = archive_write_finish(sub_a);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while closing CRG control.tar.gz sub-archive: " << archive_error_string(sub_a);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }

        metadatum = data.content();
        //metadata = string(data, data+used);
      }

      // create main archive entry
      archive_entry* entry = archive_entry_new();

      // configure main entry stats
      archive_entry_set_mode(entry, S_IFREG | 0644);
      archive_entry_set_size(entry, metadatum.size());

      // set main entry path
      archive_entry_set_pathname(entry, "control.tar.gz");

      // write main entry header
      result = archive_write_header(a, entry);
      if (result != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while writing CRG control.tar.gz entry header: " << archive_error_string(a);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // write metadata into archive entry
      __LA_SSIZE_T written = archive_write_data(a, metadatum.c_str(), metadatum.size());
      if (written < (__LA_SSIZE_T)metadatum.size())
      {
        stringstream msg;
        msg << "error while writing CRG control.tar.gz entry: " << archive_error_string(a);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      //archive_write_finish_entry(a); // not necessary

      // free main entry object
      archive_entry_free(entry);
    }

  //----------------------------------------------------------------------------
    // write each file from the temporary directory in data.tar.gz in the archive
    {
      // build a sub-archive in a temporary file
      bfs::path tmpPath(qi::os::tmpdir());
      bfs::path dataPath;
      int pid = getpid();
      int i = -1;
      do
      {
        ++i;
        std::stringstream filename;
        filename << "Project_data_" << pid << "_" << i << ".tar.gz";
        dataPath = tmpPath / filename.str();
      } while (bfs::exists(dataPath));

      // create sub-archive object
      struct archive* sub_a = archive_write_new();

      // :KLUDGE: has to open the file before setting format, or archive_write_finish may crash
      FileHelper file(dataPath);
      int openResult = FileHelper::openForWrite(a, static_cast<void*>(&file));
      if (openResult != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while trying to open CRG data.tar.gz sub-archive " << destPath.native() << ": " << archive_error_string(sub_a);
        archive_write_finish(sub_a);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // set compression
      archive_write_set_compression_gzip(sub_a);

      // select format
      if (archive_write_set_format_ustar(sub_a) != ARCHIVE_OK)
        qiLogWarning("behavior.Project") << "error while setting format to tar: " << archive_error_string(sub_a) << std::endl;

      // open sub-archive for writing
      int result = archive_write_open(sub_a, static_cast<void*>(&file), NULL, FileHelper::write, FileHelper::close);

      if (result != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while trying to open CRG data.tar.gz sub-archive " << destPath.native() << ": " << archive_error_string(sub_a);
        archive_write_finish(sub_a);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // write each entry
      std::set<std::string> lFiles = localFiles(sourceProject);
      std::set<std::string>::const_iterator it;
      for(it = lFiles.begin(); it != lFiles.end(); ++it)
      {
        // Ignore metadata file, saved in control.tar.gz
        if(*it == ".metadata")
          continue;

        // open file in temporary directory for reading
        bfs::path filePath = sourceProject / *it;
        FILE* fentry = qi::os::fopen(filePath.c_str(), "rb");
        if(!fentry)
        {
          int err = errno;
          stringstream msg;
          msg << "could not open file " << filePath.native() << " for read while saving project: " << strerror(err);
          archive_write_finish(a);
          throw ioreaderror(msg.str());
        }

        // create archive sub-entry
        archive_entry* sub_entry = archive_entry_new();

        // configure sub-entry stats
        struct stat st;
        qi::os::stat(filePath.c_str(), &st); // :TODO: handle return value
        archive_entry_set_mode(sub_entry, S_IFREG | 0644);
        archive_entry_set_size(sub_entry, st.st_size);

        // set sub-entry path
        archive_entry_set_pathname(sub_entry, it->c_str());

        // write sub-entry header
        result = archive_write_header(sub_a, sub_entry);
        if(result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while writing entry " << *it << " header: " << archive_error_string(sub_a);
          fclose(fentry);
          archive_entry_free(sub_entry);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }

        // copy file content into archive entry
        char buffer[fBlocksize];
        size_t len = fread(buffer, 1, fBlocksize, fentry);
        while (len > 0)
        {
          __LA_SSIZE_T written = archive_write_data(sub_a, buffer, len);
          if (written < (__LA_SSIZE_T)len)
          {
            stringstream msg;
            msg << "error while writing entry " << *it << ": " << archive_error_string(sub_a);
            fclose(fentry);
            archive_entry_free(sub_entry);
            archive_write_finish(sub_a);
            archive_write_finish(a);
            throw iowriteerror(msg.str());
          }
          len = fread(buffer, 1, fBlocksize, fentry);
        }
        if (len < fBlocksize && ferror(fentry))
        {
          int err = errno;
          stringstream msg;
          msg << "error while reading entry " << filePath.native() << " file: " << strerror(err);
          fclose(fentry);
          archive_entry_free(sub_entry);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw ioreaderror(msg.str());
        }

        //archive_write_finish_entry(sub_a); // not necessary

        // free entry object
        archive_entry_free(sub_entry);

        // close entry file
        result = fclose(fentry);
        if (result != 0)
        {
          int err = errno;
          stringstream msg;
          msg << "error while closing entry " << filePath.native() << " file: " << strerror(err);
          archive_write_finish(sub_a);
          archive_write_finish(a);
          throw ioreaderror(msg.str());
        }
      }

      // close sub-archive
      result = archive_write_finish(sub_a);
      if (result != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while closing CRG data.tar.gz sub-archive: " << archive_error_string(sub_a);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // open temporary file for reading
      FILE* fentry = qi::os::fopen(dataPath.c_str(), "rb");
      if (!fentry)
      {
        int err = errno;
        stringstream msg;
        msg << "could not open CRG data.tar.gz file for read while saving project: " << strerror(err);
        bfs::remove(dataPath);
        archive_write_finish(a);
        throw ioreaderror(msg.str());
      }

      // create main archive entry
      archive_entry* entry = archive_entry_new();

      // configure main entry stats
      struct stat st;
      qi::os::stat(dataPath.c_str(), &st); // :TODO: handle return value
      archive_entry_set_mode(entry, S_IFREG | 0644);
      archive_entry_set_size(entry, st.st_size);

      // set main entry path
      archive_entry_set_pathname(entry, "data.tar.gz");

      // write main entry header
      result = archive_write_header(a, entry);
      if (result != ARCHIVE_OK)
      {
        stringstream msg;
        msg << "error while writing CRG data.tar.gz entry header: " << archive_error_string(a);
        fclose(fentry);
        bfs::remove(dataPath);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw iowriteerror(msg.str());
      }

      // copy file content into archive entry
      char buffer[fBlocksize];
      size_t len = fread(buffer, 1, fBlocksize, fentry);
      while (len > 0)
      {
        __LA_SSIZE_T written = archive_write_data(a, buffer, len);
        if (written < (__LA_SSIZE_T)len)
        {
          stringstream msg;
          msg << "error while writing CRG data.tar.gz entry: " << archive_error_string(a);
          fclose(fentry);
          bfs::remove(dataPath);
          archive_entry_free(entry);
          archive_write_finish(a);
          throw iowriteerror(msg.str());
        }
        len = fread(buffer, 1, fBlocksize, fentry);
      }
      if (len < fBlocksize && ferror(fentry))
      {
        int err = errno;
        stringstream msg;
        msg << "error while reading entry CRG data.tar.gz file: " << strerror(err);
        fclose(fentry);
        bfs::remove(dataPath);
        archive_entry_free(entry);
        archive_write_finish(a);
        throw ioreaderror(msg.str());
      }

      //archive_write_finish_entry(a); // not necessary

      // free main entry object
      archive_entry_free(entry);

      // close temporary file
      result = fclose(fentry);
      if (result != 0)
      {
        int err = errno;
        stringstream msg;
        msg << "error while closing CRG data.tar.gz file: " << strerror(err);
        bfs::remove(dataPath);
        throw ioreaderror(msg.str());
      }

      // remove temporary file
      bfs::remove(dataPath);
    }

  //----------------------------------------------------------------------------

    // close archive
    result = archive_write_finish(a);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while closing project archive " << destPath.native() << ": " << archive_error_string(a);
      throw iowriteerror(msg.str());
    }
  }

  void ProjectImpl::importSimpleArchive(
    const bfs::path &sourcePath
  , const bfs::path &destPath
  , const Project::ArchiveFormat &format
  ) throw(
    Project::PathNotFound
  , ioreaderror
  , iowriteerror)
  {
    // create archive object
    struct archive* a = archive_read_new();

    // load format handler
    switch (format)
    {
      case Project::ArchiveFormat_TAR:
      case Project::ArchiveFormat_TAR_GZ:
  #if Project_HAS_BZIP2
      case ArchiveFormat_TAR_BZ2:
  #endif
        if (archive_read_support_format_tar(a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for tar format: " << archive_error_string(a) << std::endl;
        break;
      case Project::ArchiveFormat_ZIP:
        if (archive_read_support_format_zip(a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for zip format: " << archive_error_string(a) << std::endl;
        break;
      default:
        throw Project::UnsupportedFormat(sourcePath.native(), format);
    }

    // load compression handler
    switch (format)
    {
      case Project::ArchiveFormat_TAR_GZ:
        if (archive_read_support_compression_gzip(a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for gzip compression: " << archive_error_string(a) << std::endl;
        break;
  #if Project_HAS_BZIP2
      case Project::ArchiveFormat_TAR_BZ2:
        if (archive_read_support_compression_bzip2(a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for bzip2 compression: " << archive_error_string(a) << std::endl;
        break;
  #endif
      default:
        throw Project::UnsupportedFormat(sourcePath.native(), format);
    }

    // open archive file for reading
    FileHelper data(sourcePath);
    int result = archive_read_open(a, static_cast<void*>(&data), FileHelper::openForRead, FileHelper::read, FileHelper::close);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while trying to open project archive " << sourcePath.native() << ": " << archive_error_string(a);
      archive_read_finish(a);
      throw ioreaderror(msg.str());
    }

    // for each file write it to the temporary directory
    struct archive_entry* entry;
    while ((result = archive_read_next_header(a, &entry)) == ARCHIVE_OK) // :FIXME: handle errors
    {
      // skip directory entries
      if (archive_entry_mode(entry) & S_IFDIR)
      {
        archive_read_data_skip(a);
      }
      else
      {
        // open entry file for writing in temporary directory
        bfs::path relative = archive_entry_pathname(entry);
        bfs::path path = destPath / relative;
        if(!bfs::exists(path.parent_path()))
        {
          try
          {
            bfs::create_directories(path.parent_path());
          }
          catch(const std::exception&)
          {
            archive_read_finish(a);
            throw;
          }
        }
        FILE* fentry = qi::os::fopen(path.c_str(), "wb");
        if (!fentry)
        {
          int err = errno;
          stringstream msg;
          msg << "could not open file " << path.native() << " for write while loading project: " << strerror(err);
          archive_read_finish(a);
          throw iowriteerror(msg.str());
        }

        // copy entry from archive to file
        char buffer[fBlocksize];
        int len;
        len = archive_read_data(a, buffer, fBlocksize);
        while (len > 0)
        {
          int written = fwrite(buffer, 1, len, fentry);
          if (written < len && ferror(fentry))
          {
            int err = errno;
            stringstream msg;
            msg << "error while writing project member " << path.native() << " to disk: " << strerror(err);
            fclose(fentry);
            archive_read_finish(a);
            throw iowriteerror(msg.str());
          }
          len = archive_read_data(a, buffer, fBlocksize);
        }
        if (len < 0)
        {
          stringstream msg;
          msg << "error while reading project member " << relative.native() << " from archive: " << archive_error_string(a);
          fclose(fentry);
          archive_read_finish(a);
          throw ioreaderror(msg.str());
        }

        // close entry file
        result = fclose(fentry);
        if (result != 0)
        {
          int err = errno;
          stringstream msg;
          msg << "error while closing project member " << path.native() << " on disk: " << strerror(err);
          archive_read_finish(a);
          throw ioerror(msg.str());
        }
      }

      //archive_read_finish_entry(a); // not necessary
    }
    if (result != ARCHIVE_EOF)
    {
      stringstream msg;
      msg << "error while reading archive " << sourcePath.native() << " entry: " << archive_error_string(a);
      archive_read_finish(a);
      throw ioreaderror(msg.str());
    }

    // :TODO: load metadata from archive

    // close archive
    result = archive_read_finish(a);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while closing project archive " << sourcePath.native() << ": " << archive_error_string(a);
      throw ioerror(msg.str());
    }
  }

  void ProjectImpl::importDebianPackage(
    const bfs::path& sourcePath
  , const bfs::path& destPath
  ) throw(
    Project::PathNotFound
  , ioreaderror
  , iowriteerror)
  {
    // create main archive object
    struct archive* a = archive_read_new();

    // load format handler
    if (archive_read_support_format_ar(a) != ARCHIVE_OK)
      qiLogWarning("behavior.Project") << "error while adding support for ar format: " << archive_error_string(a) << std::endl;

    // open main archive file for reading
    FileHelper data(sourcePath);
    int result = archive_read_open(a, static_cast<void*>(&data), FileHelper::openForRead, FileHelper::read, FileHelper::close);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while trying to open project archive " << sourcePath.native() << ": " << archive_error_string(a);
      archive_read_finish(a);
      throw ioerror(msg.str());
    }

    // the main archive should contain control.tar.gz and data.tar.gz
    struct archive_entry* entry;
    while ((result = archive_read_next_header(a, &entry)) == ARCHIVE_OK) // :FIXME: handle errors
    {
      // open entry file for writing in temporary directory
      bfs::path relative = archive_entry_pathname(entry);
      if (relative == "control.tar.gz")
      {
        // create archive object
        struct archive* sub_a = archive_read_new();

        // load format handler
        if (archive_read_support_format_tar(sub_a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for tar format: " << archive_error_string(sub_a) << std::endl;

        // load compression handler
        if (archive_read_support_compression_gzip(sub_a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for gzip compression: " << archive_error_string(sub_a) << std::endl;

        // open sub-archive file for reading
        ArchiveEntryHelper data(a);
        int result = archive_read_open(sub_a, static_cast<void*>(&data), ArchiveEntryHelper::openForRead, ArchiveEntryHelper::read, ArchiveEntryHelper::close);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while trying to open project archive " << sourcePath.native() << ": " << archive_error_string(sub_a);
          archive_read_finish(sub_a);
          archive_read_finish(a);
          throw ioerror(msg.str());
        }

        // look for control in control.tar.gz
        struct archive_entry* entry;
        while ((result = archive_read_next_header(sub_a, &entry)) == ARCHIVE_OK) // :FIXME: handle errors
        {
          // open entry file for reading metadata
          bfs::path relative = archive_entry_pathname(entry);
          if (relative == "control")
          {
            // load metadata from entry
            char buffer[fBlocksize];
            string content;
            int read;
            while ((read = archive_read_data(sub_a, buffer, fBlocksize)) > 0)
            {
              copy(buffer, buffer+read, back_inserter(content));
            }
            if (read < 0)
            {
              stringstream msg;
              msg << "error while reading project metadata member from archive: " << archive_error_string(sub_a);
              archive_read_finish(sub_a);
              archive_read_finish(a);
              throw ioreaderror(msg.str());
            }

            // Write raw metadata file content into .metadata
            bfs::path metaPath = destPath / ".metadata";
            FILE* f = qi::os::fopen(metaPath.c_str(), "wb");
            if(!f)
            {
              throw iowriteerror(metaPath.native());
            }
            fprintf(f, "%s", content.c_str());
            fclose(f);
          }
          else
          {
            // ignore unknown control.tar.gz members
            archive_read_data_skip(sub_a);
          }

          //archive_read_finish_entry(sub_a); // not necessary
        }
        if (result != ARCHIVE_EOF)
        {
          stringstream msg;
          msg << "error while reading archive " << sourcePath.native() << " entry: " << archive_error_string(sub_a);
          archive_read_finish(sub_a);
          archive_read_finish(a);
          throw ioreaderror(msg.str());
        }

        // close sub-archive
        result = archive_read_finish(sub_a);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while closing project archive " << sourcePath.native() << ": " << archive_error_string(sub_a);
          archive_read_finish(a);
          throw ioerror(msg.str());
        }
      }
      else if (relative == "data.tar.gz")
      {
        // create sub-archive object
        struct archive* sub_a = archive_read_new();

        // load format handler
        if (archive_read_support_format_tar(sub_a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for tar format: " << archive_error_string(sub_a) << std::endl;

        // load compression handler
        if (archive_read_support_compression_gzip(sub_a) != ARCHIVE_OK)
          qiLogWarning("behavior.Project") << "error while adding support for gzip compression: " << archive_error_string(sub_a) << std::endl;

        // open sub-archive file for reading
        ArchiveEntryHelper data(a);
        int result = archive_read_open(sub_a, static_cast<void*>(&data), ArchiveEntryHelper::openForRead, ArchiveEntryHelper::read, ArchiveEntryHelper::close);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while trying to open project archive " << sourcePath.native() << ": " << archive_error_string(sub_a);
          archive_read_finish(sub_a);
          archive_read_finish(a);
          throw ioerror(msg.str());
        }

        // for each file write it to the temporary directory
        struct archive_entry* sub_entry;
        while ((result = archive_read_next_header(sub_a, &sub_entry)) == ARCHIVE_OK) // :FIXME: handle errors
        {
          // skip directory entries
          if (archive_entry_mode(sub_entry) & S_IFDIR)
          {
            archive_read_data_skip(sub_a);
          }
          else
          {
            // open entry file for writing in temporary directory
            bfs::path relative = archive_entry_pathname(sub_entry);

            bfs::path path = destPath / relative;
            if (!bfs::exists(path.parent_path()))
            {
              try
              {
                bfs::create_directories(path.parent_path());
              }
              catch(const std::exception&)
              {
                archive_read_finish(a);
                throw;
              }
            }
            FILE* fentry = qi::os::fopen(path.c_str(), "wb");
            if (!fentry)
            {
              int err = errno;
              stringstream msg;
              msg << "could not open file " << path.native() << " for write while loading project: " << strerror(err);
              archive_read_finish(sub_a);
              archive_read_finish(a);
              throw ioerror(msg.str());
            }

            // copy entry from sub-archive to file
            char buffer[fBlocksize];
            int len;
            len = archive_read_data(sub_a, buffer, fBlocksize);
            while (len > 0)
            {
              int written = fwrite(buffer, 1, len, fentry);
              if (written < len && ferror(fentry))
              {
                int err = errno;
                stringstream msg;
                msg << "error while writing project member " << path.native() << " to disk: " << strerror(err);
                fclose(fentry);
                archive_read_finish(sub_a);
                archive_read_finish(a);
                throw iowriteerror(msg.str());
              }
              len = archive_read_data(sub_a, buffer, fBlocksize);
            }
            if (len < 0)
            {
              stringstream msg;
              msg << "error while reading project member " << relative.native() << " from archive: " << archive_error_string(sub_a);
              fclose(fentry);
              archive_read_finish(sub_a);
              archive_read_finish(a);
              throw ioreaderror(msg.str());
            }

            // close entry file
            result = fclose(fentry);
            if (result != 0)
            {
              int err = errno;
              stringstream msg;
              msg << "error while closing project member " << path.native() << " on disk: " << strerror(err);
              archive_read_finish(sub_a);
              archive_read_finish(a);
              throw ioerror(msg.str());
            }
          }

          //archive_read_finish_entry(sub_a); // not necessary
        }
        if (result != ARCHIVE_EOF)
        {
          stringstream msg;
          msg << "error while reading archive " << sourcePath.native() << " entry: " << archive_error_string(sub_a);
          archive_read_finish(sub_a);
          archive_read_finish(a);
          throw ioreaderror(msg.str());
        }

        // close sub-archive
        result = archive_read_finish(sub_a);
        if (result != ARCHIVE_OK)
        {
          stringstream msg;
          msg << "error while closing project archive " << sourcePath.native() << ": " << archive_error_string(sub_a);
          archive_read_finish(a);
          throw ioerror(msg.str());
        }
      }
      else
      {
        // ignore unknown main archive members
        archive_read_data_skip(a);
      }

      //archive_read_finish_entry(a); // not necessary
    }
    if (result != ARCHIVE_EOF)
    {
      stringstream msg;
      msg << "error while reading archive " << sourcePath.native() << " entry: " << archive_error_string(a);
      archive_read_finish(a);
      throw ioreaderror(msg.str());
    }

    // close main archive
    result = archive_read_finish(a);
    if (result != ARCHIVE_OK)
    {
      stringstream msg;
      msg << "error while closing project archive " << sourcePath.native() << ": " << archive_error_string(a);
      throw ioerror(msg.str());
    }
  }

  void xLocalTreeRecursion(
    const bfs::path& directory
  , const bfs::path& prefix
  , std::set<std::string>& fileList
  )
  {
    bfs::directory_iterator itEnd;
    bfs::directory_iterator itf;
    for(itf = bfs::directory_iterator(directory);
      itf != itEnd; ++itf)
    {
      bfs::path lPath = itf->path();
      if(bfs::is_directory(lPath))
      {
        xLocalTreeRecursion(
          lPath, prefix / lPath.filename()
        , fileList);
      }
      else
      {
        fileList.insert((prefix / lPath.filename()).native());
      }
    }
  }

  std::set<std::string> ProjectImpl::localFiles(
    const bfs::path& directory
  ) throw(
    Project::PathNotFound)
  {
    if(!bfs::exists(directory))
    {
      throw Project::PathNotFound(directory.native());
    }
    if(!bfs::is_directory(directory))
    {
      throw Project::PathNotFound(directory.native() + " is not a directory");
    }

    std::set<std::string> fileList;
    xLocalTreeRecursion(directory, "", fileList);
    return fileList;
  }
} // End of namespace AL
