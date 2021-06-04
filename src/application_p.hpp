#pragma once

# include <qi/api.hpp>
# include <qi/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/process/search_path.hpp>

namespace qi
{
  namespace details
  {
    // @brief Try to construct an absolute path from the given path,
    // using the platform specificity and the current program location.
    //
    // If the input path is absolute it will not change.
    // If the input path is relative and has parent information (./bar or foo/bar)
    // it will be searched from the current directory.
    // Otherwise, it will be searched in the folders given in input (first currentDirectory, then environmentPaths).
    // In this case, the file pointed by the input path within the folder shall exist and be executable.
    //
    // @param path Path to transform.
    // @param currentDirectory Path to search relative paths from. Default value: bfs::current_path()
    // @param environmentPaths List of directories to search relative path with no parent information (e.g. bar).
    // Same role as 'PATH' environment variable
    // Default value: ::boost::this_process::path()
    // @return An absolute path. The empty path if an error occurred.

    QI_API_TESTONLY Path searchExecutableAbsolutePath(const Path& path,
                                                      const boost::filesystem::path& currentDirectory = boost::filesystem::current_path(),
                                                      std::vector<boost::filesystem::path> environmentPaths = ::boost::this_process::path());
  }
}
