/**
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2011 All Rights Reserved
 */

#ifndef _LIBQI_QI_PROJECT_BOOST_HPP_
#define _LIBQI_QI_PROJECT_BOOST_HPP_

#include <set>
#include <boost/filesystem.hpp>
#include <qi/project.hpp>

/* Forward decl */
struct archive; // Libarchive

namespace qi
{
  class ProjectImpl
  {
  public:
    const boost::filesystem::path projectPath;

    // For dummy (invalid) projects
    ProjectImpl() throw();

    // Normal constructor
    ProjectImpl(const std::string& projectDir) throw();

    virtual ~ProjectImpl() throw();

    // Reads metadata from the given metadata file
    static std::map<std::string, std::string> xReadMetadata(
      const boost::filesystem::path& metaPath
    ) throw(
      Project::PathNotFound
    , ioreaderror);

    // Saves metadata to the given metadata file
    static void xWriteMetadata(
      const boost::filesystem::path& metaPath
    , const std::map<std::string, std::string>& metadata
    ) throw(
      Project::PathNotFound
    , iowriteerror);

    // Blocksize for archive export.
    static const size_t fBlocksize;

    // Supports .tar, .tar.gz and .tar.bz2, .zip upon valid flags
    static void exportSimpleArchive(
      const boost::filesystem::path& sourcePath
    , const boost::filesystem::path& destPath
    , const Project::ArchiveFormat& format
    ) throw(
      Project::PathNotFound
    , ioreaderror
    , iowriteerror);

    static void exportDebianPackage(
      const boost::filesystem::path& sourcePath
    , const boost::filesystem::path& destPath
    ) throw(
      Project::PathNotFound
    , ioreaderror
    , iowriteerror);

    static void importSimpleArchive(
      const boost::filesystem::path& sourcePath
    , const boost::filesystem::path& destPath
    , const Project::ArchiveFormat& format
    ) throw(
      Project::PathNotFound
    , ioreaderror
    , iowriteerror);

    static void importDebianPackage(
      const boost::filesystem::path& sourcePath
    , const boost::filesystem::path& destPath
    ) throw(
      Project::PathNotFound
    , ioreaderror
    , iowriteerror);

    static std::set<std::string> localFiles(
      const boost::filesystem::path& directory
    ) throw(
      Project::PathNotFound);
  };
}


#endif // _LIBQI_QI_PROJECT_BOOST_HPP_
