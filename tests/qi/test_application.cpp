#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/path.hpp>
#include <src/application_p.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace {

  boost::filesystem::path makeTempDir(const qi::Path& parentDirectoryPath)
  {
    const auto tmpFile = boost::filesystem::path(parentDirectoryPath) / boost::filesystem::unique_path();
    boost::filesystem::create_directory(tmpFile);
    return tmpFile;
  }

  boost::filesystem::path makeTmpFilePath(const qi::Path& directoryPath, const std::string& suffix = "")
  {
    return boost::filesystem::path(directoryPath) /
            boost::filesystem::path(boost::filesystem::unique_path().string() + suffix);
  }

  boost::filesystem::path makeFile(const qi::Path& directoryPath, const std::string& suffix = "")
  {
    const auto tmpPath = makeTmpFilePath(directoryPath, suffix);
    // Create new file
    boost::filesystem::ofstream tmpOfStream(tmpPath.string());
    tmpOfStream << "tmp" << std::endl;
    return tmpPath;
  }
}

TEST(TestSearchExecutableAbsolutePath, AlreadyAbsolutePathDoesNotChange)
{
  qi::path::ScopedDir tmpDir;
  const qi::Path tmpAbsoluteFilePath = makeFile(tmpDir);
  EXPECT_EQ(tmpAbsoluteFilePath, qi::details::searchExecutableAbsolutePath(tmpAbsoluteFilePath));
}

TEST(TestSearchExecutableAbsolutePath, RelativePathChanges)
{
  qi::path::ScopedDir tmpDirRelative;
  qi::path::ScopedDir tmpDir(makeTempDir(tmpDirRelative.path()));
  const qi::Path tmpRelativeFilePath = boost::filesystem::relative(makeFile(tmpDir), tmpDirRelative.path());
  EXPECT_EQ(tmpDir.path() / qi::Path(tmpRelativeFilePath.filename()),
            qi::details::searchExecutableAbsolutePath(tmpRelativeFilePath, tmpDirRelative.path().bfsPath()));
}

TEST(TestSearchExecutableAbsolutePath, ExecutableFromCurrentPathsChanges)
{
  qi::path::ScopedDir tmpDir;
  const qi::Path tmpExecutableFilePath = makeFile(tmpDir, ".exe");
  boost::filesystem::permissions(tmpExecutableFilePath, boost::filesystem::owner_exe);
  EXPECT_EQ(tmpDir.path() / qi::Path(tmpExecutableFilePath.filename()),
            qi::details::searchExecutableAbsolutePath(tmpExecutableFilePath.filename(), tmpDir.path()));
}

TEST(TestSearchExecutableAbsolutePath, ExecutableFromEnvironmentPathsChanges)
{
  qi::path::ScopedDir tmpDir;
  const qi::Path tmpExecutableFilePath = makeFile(tmpDir, ".exe");
  std::vector<boost::filesystem::path> pathlist;
  pathlist.push_back(tmpDir.path().bfsPath());
  boost::filesystem::permissions(tmpExecutableFilePath, boost::filesystem::owner_exe);
  EXPECT_EQ(tmpDir.path() / qi::Path(tmpExecutableFilePath.filename()),
            qi::details::searchExecutableAbsolutePath(tmpExecutableFilePath.filename(), boost::filesystem::current_path(), pathlist));
}

TEST(TestSearchExecutableAbsolutePath, NotExistingAbsolutePathSucceeds)
{
  qi::path::ScopedDir tmpDir;
  const qi::Path tmpAbsoluteFilePath = makeTmpFilePath(tmpDir);
  EXPECT_EQ(tmpAbsoluteFilePath, qi::details::searchExecutableAbsolutePath(tmpAbsoluteFilePath));
}

TEST(TestSearchExecutableAbsolutePath, NotExistingRelativePathSucceeds)
{
  qi::path::ScopedDir tmpDirRelative;
  qi::path::ScopedDir tmpDir(makeTempDir(tmpDirRelative.path()));
  const qi::Path tmpRelativeFilePath = boost::filesystem::relative(makeTmpFilePath(tmpDir), tmpDirRelative.path());
  EXPECT_EQ(tmpDir.path() / qi::Path(tmpRelativeFilePath.filename()),
            qi::details::searchExecutableAbsolutePath(tmpRelativeFilePath, tmpDirRelative.path().bfsPath()));
}

TEST(TestSearchExecutableAbsolutePath, NotExistingExecutableFromCurrentPathsFails)
{
  qi::path::ScopedDir tmpDir;
  const qi::Path tmpExecutableFilePath = makeTmpFilePath(tmpDir, ".exe");
  EXPECT_EQ(qi::Path(),
            qi::details::searchExecutableAbsolutePath(tmpExecutableFilePath.filename(), tmpDir.path()));
}

TEST(TestSearchExecutableAbsolutePath, NotExistingExecutableFromEnvironmentPathsFails)
{
  qi::path::ScopedDir tmpDir;
  const qi::Path tmpExecutableFilePath = makeTmpFilePath(tmpDir, ".exe");
  std::vector<boost::filesystem::path> pathlist;
  pathlist.push_back(tmpDir.path().bfsPath());
  EXPECT_EQ(qi::Path(),
            qi::details::searchExecutableAbsolutePath(tmpExecutableFilePath.filename(), boost::filesystem::current_path(), pathlist));
}

TEST(TestSearchExecutableAbsolutePath, RootPathDoesNotChange)
{
  qi::path::ScopedDir tmpDir;
  const auto tmpAbsoluteFilePath = qi::Path("/");
  EXPECT_EQ(tmpAbsoluteFilePath, qi::details::searchExecutableAbsolutePath(tmpAbsoluteFilePath));
}

TEST(TestSearchExecutableAbsolutePath, FileInRootPathDoesNotChange)
{
  qi::path::ScopedDir tmpDir;
  const auto tmpAbsoluteFilePath = qi::Path("/foo");
  EXPECT_EQ(tmpAbsoluteFilePath, qi::details::searchExecutableAbsolutePath(tmpAbsoluteFilePath));
}

TEST(Application, SetNoArguments)
{
  int argc = 0;
  char* arg = nullptr;
  char** argv = &arg;
  qi::Application::setArguments(argc, argv);
}

TEST(Application, SetEmptyArguments)
{
  qi::Application::setArguments({});
}
