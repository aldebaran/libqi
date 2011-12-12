/**
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2011 All Rights Reserved
 */

#pragma once

#ifndef _LIBQI_QI_PROJECT_HPP_
#define _LIBQI_QI_PROJECT_HPP_

#include <map>
#include <list>
#include <string>
#include <sstream>
#include <stdexcept>

#include <qi/exception.hpp>

namespace qi
{
  class ProjectImpl;

  class Project
  {
  public:

    /**
     * @brief Supported project formats.
     */
    enum ArchiveFormat
    {
      ArchiveFormat_AutoDetect = -2,
      ArchiveFormat_Default = -1,
      ArchiveFormat_Directory = 0,
      ArchiveFormat_TAR,
      ArchiveFormat_ZIP,
      ArchiveFormat_TAR_GZ,
      ArchiveFormat_TAR_BZ2,
      ArchiveFormat_DEB
    };

    /**
     * @brief Thrown by invalid projects on operations requiring validation.
     */
    class InvalidProject : public std::exception
    {
    public:
      InvalidProject() : std::exception() { }
      virtual const char* what() const throw()
      { return "invalid project instance"; }
    };

    /**
     * @brief Thrown by methods requiring valid paths arguments.
     */
    class PathNotFound : public std::runtime_error
    {
    public:
      PathNotFound(const std::string& path) : std::runtime_error(path) { }
    };

    /**
     * @brief Thrown when a project format does not match or could not be recognized
     */
    class UnsupportedFormat : public std::exception
    {
    public:
      UnsupportedFormat(
        const std::string& triedPath
      , const ArchiveFormat& triedFormat)
      : std::exception()
      , path(triedPath)
      , format(triedFormat)
      { }

      ~UnsupportedFormat() throw()
      { }

      const std::string path;
      const ArchiveFormat format;

      virtual const char* what() const throw()
      {
        std::stringstream message;
        message << path << ": " << format;
        return message.str().c_str();
      }
    };

    /**
     * @brief Thrown when the project content does not match the format.
     */
    class CorruptedFile : public std::runtime_error { };

    /**
     * @brief Thrown upon content creation where content already exists.
     */
    class AlreadyExists : public std::runtime_error
    {
    public:
      AlreadyExists(const std::string& path) : runtime_error(path) { }
    };

    /**
     * @brief Default Constructor.
     * Creates a dummy (and invalid) instance of ALProject.
     */
    Project() throw();

    /**
     * @brief Copy Constructor.
     */
    Project(const Project& other) throw();

    /**
     * @brief Represents the project directory found at the given path.
     * Throws if the path is not found or is not readable.
     * @param projectDir the absolute path towards the project directory.
     */
    Project(const std::string& projectDir) throw();

    /**
     * @brief Destroy the project instance.
     * Does not alter filesystem.
     */
    virtual ~Project() throw();

    /**
     * @brief Assignment operation.
     */
    Project& operator =(const Project& other) throw();

    /**
     * @brief Swap (effective c++ recommendation)
     */
     void swap(Project& other) { std::swap(fImpl, other.fImpl); }

    /**
     * @brief The path tracked by this project instance.
     */
    virtual std::string projectPath() const throw();

    /**
     * @brief Check whether the tracked project directory still exists.
     * An "inexistant" project instance behavior is undefined.
     * Instance methods should throw InvalidInstance exception.
     * @return true if the project directory exists, false otherwise.
     */
    bool valid() const throw();

    /**
     * @brief Get all the project's metadata.
     * Metadata are named non-empty strings. It uses the HTTP header syntax,
     * but is case-sensitive.
     * @return a map containing all the metadata.
     */
    std::map<std::string, std::string> getMetadata() const throw();

    /**
     * @brief Get a single metadata value.
     * @param key the name of the metada to request.
     * @return the value of the metadata or an empty string if key not found.
     */
    std::string getMetadata(const std::string& key) const throw();

    /**
     * @brief Set all the project's metadata.
     * All previous metadata will be cleared and replaced.
     * @param metadata a map containing every metadata.
     * @throw whenever we cannot write metadata.
     */
    void setMetadata(
      const std::map<std::string, std::string>& metadata
    ) throw(iowriteerror);

    /**
     * @brief Set a single metadata value.
     * Will not alter other metadata keys.
     * @param key the metadata name to set.
     * @param value the new value to set.
     * @throw whenever we cannot write metadata.
     */
    void setMetadata(
      const std::string& key
    , const std::string& value
    ) throw(iowriteerror);

  private:
    ProjectImpl* fImpl;

  public:
    /**
     * @brief Creates an empty project directory at the given path.
     * If the path already exists, AlreadyExists exception is thrown.
     * If it cannot create the project directory, NotWritable is thrown.
     * If no path is given, the project will be created in a temporary
     * @param path where to create the project.
     * @return a newly created project reference.
     */
    static Project* create(
      const std::string projectPath
    ) throw(PathNotFound, iowriteerror, AlreadyExists);

    /**
     * Exports a project directory to a project archive or directory.
     * @param sourceProject the path towards the original project.
     * @param destPath the path towards the project export result.
     * @param format the exportation format.
     */
    void exportProject(
      const std::string& destPath
    , const ArchiveFormat& format
    ) throw(PathNotFound, ioerror)
    { exportProject(projectPath(), destPath, format); }

    /**
     * Creates a project directory out of a project archive or directory.
     * @param sourcePath the path to the source project to import.
     * @param destDir the path to the new project directory.
     * @param format the format of the project to import.
     */
    static void importProject(
      const std::string& sourcePath
    , const std::string& destPath
    , const ArchiveFormat& format = ArchiveFormat_AutoDetect
    ) throw(PathNotFound, ioerror, UnsupportedFormat, CorruptedFile);

    /**
     * Exports a project directory to a project archive or directory.
     * @param sourceProject the path towards the original project.
     * @param destPath the path towards the project export result.
     * @param format the exportation format.
     */
    static void exportProject(
      const std::string& sourceProject
    , const std::string& destPath
    , const ArchiveFormat& format
    ) throw(PathNotFound, ioerror);

    /**
     * Detect the project format at the given path.
     * @param projectPath the path towards the project to analyse.
     * @throw PathNotFound or UnsupportedFormat if unrecognized.
     */
    static ArchiveFormat detectFormat(
      const std::string& projectPath
    ) throw(
      PathNotFound
    , UnsupportedFormat);

  }; // End of class Project

} // End of namespace qi

#endif // _LIBQI_QI_PROJECT_HPP_
