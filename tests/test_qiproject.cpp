/**
 * @author Victor Paleologue
 * @author Jerome Vuarand
 * Copyright (c) 2011 Aldebaran Robotics  All Rights Reserved
 */
#include <gtest/gtest.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <qi/project.hpp>
#include <qi/os.hpp>

using namespace qi;
using namespace std;
namespace bfs = boost::filesystem;

class ALProjectTests_Create : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    fPath = bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
    fProject = Project::create(fPath.generic_string());
  }
  virtual void TearDown()
  {
    delete fProject;
    bfs::remove_all(fPath);
  }

public:
  Project* fProject;
  bfs::path fPath;
};

TEST_F(ALProjectTests_Create, construction)
{
  ASSERT_EQ(fPath, fProject->projectPath());
  ASSERT_TRUE(bfs::exists(fProject->projectPath()));
}

TEST_F(ALProjectTests_Create, destruction)
{
  delete fProject;
  fProject = 0;
  bfs::remove_all(fPath);
  ASSERT_FALSE(bfs::exists(fPath));
}

TEST_F(ALProjectTests_Create, invalidation)
{
  ASSERT_TRUE(fProject->valid());
  bfs::remove_all(fPath);
  ASSERT_FALSE(bfs::exists(fPath));
  ASSERT_FALSE(fProject->valid());
}

TEST_F(ALProjectTests_Create, metadataAll)
{
  std::map<std::string, std::string> metadata;
  metadata["Type"] = "ChoregrapheBehavior";
  metadata["Version"] = "2.0";
  fProject->setMetadata(metadata);
  std::map<std::string, std::string> readMetadata = fProject->getMetadata();
  ASSERT_TRUE(metadata == readMetadata);
}

TEST_F(ALProjectTests_Create, metadataSingle)
{
  fProject->setMetadata("Type", "ChoregrapheBehavior");
  fProject->setMetadata("Version", "2.0");
  ASSERT_EQ("ChoregrapheBehavior", fProject->getMetadata("Type"));
  ASSERT_EQ("2.0", fProject->getMetadata("Version"));
}

class ALProjectTests_Existing : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    fContentSummary = map<string, size_t>();
    fPath = bfs::path(os::tmpdir()).parent_path() / "ALProjectTests_Load";
    bfs::create_directories(fPath);
    fToRemove.insert(fPath);

    FILE* fileHandle;
    fileHandle = os::fopen((fPath / ".metadata").c_str(), "wb");
    fprintf(fileHandle, "Type: ChoregrapheBoxLibrary\nVersion: 2.0\n");
    fclose(fileHandle);
    fContentSummary[".metadata"] = strlen("Type: ChoregrapheBoxLibrary\nVersion: 2.0\n");
    fMetadata["Type"] = "ChoregrapheBoxLibrary";
    fMetadata["Version"] = "2.0";

    fXalInfoContent =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
      "<Folder name=\"\" expanded=\"no\">\n"
      "    <Node name=\"BoxName\" />\n"
      "</Folder>\n";
    fileHandle = os::fopen((fPath / "xalinfo").c_str(), "wb");
    fwrite(fXalInfoContent.c_str(), 1, fXalInfoContent.size(), fileHandle);
    fclose(fileHandle);
    fContentSummary["xalinfo"] = fXalInfoContent.size();

    bfs::create_directories(fPath / "BoxName");
    fXarContent =
      "<?xml version='1.0' encoding='UTF-8' ?>\n"
      "<ChoregrapheProject xmlns='http://www.aldebaran-robotics.com/schema/choregraphe/project.xsd' xar_version='3'>\n"
      "    <Box name='root' robot='' id='-1' tooltip='Root box of Choregraphe&apos;s project. Highest level possible.' bitmap_expanded='1' plugin='' x='0' y='0'>\n"
      "        <bitmap>media/images/box/root.png</bitmap>\n"
      "        <script language='4'>\n"
      "            <content>\n"
      "                <![CDATA[]]>\n"
      "</content>\n"
      "        </script>\n"
      "        <Input name='onLoad' type='1' type_size='1' nature='0' inner='1' tooltip='Signal sent when diagram is loaded.' id='1' />\n"
      "        <Input name='onStart' type='1' type_size='1' nature='2' inner='0' tooltip='Box behavior starts when a signal is received on this input.' id='2' />\n"
      "        <Input name='onStop' type='1' type_size='1' nature='3' inner='0' tooltip='Box behavior stops when a signal is received on this input.' id='3' />\n"
      "        <Output name='onStopped' type='1' type_size='1' nature='1' inner='0' tooltip='Signal sent when box behavior is finished.' id='4' />\n"
      "        <Timeline fps='10' resources_acquisition='0' size='300' enable='0' start_frame='0' end_frame='-1' scale='10'>\n"
      "            <watches />\n"
      "            <BehaviorLayer name='behavior_layer1' mute='0'>\n"
      "                <BehaviorKeyframe name='keyframe1' index='1' bitmap=''>\n"
      "                    <Diagram scale='-1' />\n"
      "                </BehaviorKeyframe>\n"
      "            </BehaviorLayer>\n"
      "            <ActuatorList />\n"
      "        </Timeline>\n"
      "    </Box>\n"
      "</ChoregrapheProject>\n";
    fileHandle = os::fopen((fPath / "BoxName" / "box.xar").c_str(), "wb");
    fwrite(fXarContent.c_str(), 1, fXarContent.size(), fileHandle);
    fclose(fileHandle);
    fContentSummary["BoxName/box.xar"] = fXarContent.size();
  }
  virtual void TearDown()
  {
    set<bfs::path>::iterator it;
    for(it = fToRemove.begin(); it != fToRemove.end(); ++it)
    {
      bfs::remove_all(*it);
    }
    fToRemove.clear();
    fMetadata.clear();
  }

private:
  static void xSummarizeContentRecursion(
    bfs::path directory
  , bfs::path prefix
  , map<string, size_t>& contentSummary)
  {
    bfs::directory_iterator itEnd;
    bfs::directory_iterator itf;
    for(itf = bfs::directory_iterator(directory);
      itf != itEnd; ++itf)
    {
      bfs::path lPath = itf->path();
      if(bfs::is_directory(lPath))
      {
        xSummarizeContentRecursion(
          lPath, prefix / lPath.filename()
        , contentSummary);
      }
      else
      {
        contentSummary[(prefix / lPath.filename()).generic_string()] =
          bfs::file_size(lPath);
      }
    }
  }

public:
  static map<string, size_t> summarizeContent(bfs::path directory)
  {
    map<string, size_t> contentSummary;
    xSummarizeContentRecursion(directory, "", contentSummary);
    return contentSummary;
  }

  bfs::path fPath;
  string fXalInfoContent;
  string fXarContent;
  map<string, size_t> fContentSummary;
  map<string, string> fMetadata;
  set<bfs::path> fToRemove;
};

class ALProjectTests_Load : public ALProjectTests_Existing
{
protected:
  virtual void SetUp()
  {
    ALProjectTests_Existing::SetUp();
    fProject = new Project(fPath.generic_string());
  }
  virtual void TearDown()
  {
    delete fProject;
    ALProjectTests_Existing::TearDown();
  }

public:
  Project* fProject;
};

TEST_F(ALProjectTests_Load, construction)
{
  ASSERT_EQ(fPath, fProject->projectPath());
  ASSERT_TRUE(bfs::exists(fProject->projectPath()));
}

TEST_F(ALProjectTests_Load, exportImportDir)
{
  bfs::path tmpPath1 =
    bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
  ASSERT_FALSE(bfs::exists(tmpPath1));

  ASSERT_NO_THROW(Project::exportProject(
    fPath.generic_string()
  , tmpPath1.generic_string()
  , Project::ArchiveFormat_Directory));
  ASSERT_TRUE(bfs::exists(tmpPath1));
  fToRemove.insert(tmpPath1);

  try
  {
    ASSERT_THROW(Project::importProject(
      fPath.generic_string()
    , tmpPath1.generic_string()
    , Project::ArchiveFormat_AutoDetect), qi::iowriteerror);
  }
  catch(const Project::UnsupportedFormat&) { }
  ASSERT_TRUE(bfs::exists(tmpPath1));

  bfs::path tmpPath2 = bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
  ASSERT_FALSE(bfs::exists(tmpPath2));

  try
  {
    ASSERT_THROW(Project::importProject(
      tmpPath1.generic_string()
    , tmpPath2.generic_string()
    , (Project::ArchiveFormat)42), Project::UnsupportedFormat);
  }
  catch(const Project::UnsupportedFormat&) { }
  ASSERT_FALSE(bfs::exists(tmpPath2));

  Project::importProject(
    tmpPath1.generic_string()
  , tmpPath2.generic_string()
  , Project::ArchiveFormat_Directory);
  ASSERT_TRUE(bfs::exists(tmpPath2));
  fToRemove.insert(tmpPath2);

  map<string, size_t> contentSummary = summarizeContent(tmpPath2);
  ASSERT_TRUE(fContentSummary == contentSummary);

  Project tmpProj(tmpPath2.generic_string());
  ASSERT_TRUE(fMetadata == tmpProj.getMetadata());
}

TEST_F(ALProjectTests_Load, exportImportCRG)
{
  bfs::path tmpPath1 =
    bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
  tmpPath1.replace_extension(".crg");
  ASSERT_FALSE(bfs::exists(tmpPath1));

  ASSERT_NO_THROW(Project::exportProject(
    fPath.generic_string()
  , tmpPath1.generic_string()
  , Project::ArchiveFormat_DEB));
  ASSERT_TRUE(bfs::exists(tmpPath1));
  fToRemove.insert(tmpPath1);

  ASSERT_TRUE(bfs::is_regular_file(tmpPath1));
  ASSERT_GT(bfs::file_size(tmpPath1), 0);

  // Can we open a project at the CRG file location?
  Project tmpProj1(tmpPath1.generic_string());
  ASSERT_NO_THROW(tmpProj1 = Project(tmpPath1.generic_string()));
  ASSERT_FALSE(tmpProj1.valid());
  ASSERT_FALSE(bfs::is_directory(tmpPath1));

  bfs::path tmpPath2 =
    bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
  ASSERT_FALSE(bfs::exists(tmpPath2));
  ASSERT_NO_THROW(Project::importProject(
    tmpPath1.generic_string()
  , tmpPath2.generic_string()
  , Project::ArchiveFormat_AutoDetect));
  ASSERT_TRUE(bfs::exists(tmpPath2));
  fToRemove.insert(tmpPath2);

  map<string, size_t> contentSummary = summarizeContent(tmpPath2);
  ASSERT_TRUE(fContentSummary == contentSummary);

  Project tmpProj(tmpPath2.generic_string());
  ASSERT_TRUE(fMetadata == tmpProj.getMetadata());
}

TEST_F(ALProjectTests_Load, exportImportTGZ)
{
  bfs::path tmpPath1 =
    bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
  tmpPath1.replace_extension(".tgz");
  ASSERT_FALSE(bfs::exists(tmpPath1));

  ASSERT_NO_THROW(Project::exportProject(
    fPath.generic_string()
  , tmpPath1.generic_string()
  , Project::ArchiveFormat_TAR_GZ));
  ASSERT_TRUE(bfs::exists(tmpPath1));
  fToRemove.insert(tmpPath1);

  ASSERT_TRUE(bfs::is_regular_file(tmpPath1));
  ASSERT_GT(bfs::file_size(tmpPath1), 0);

  // Can we open a project at the TGZ file location?
  Project tmpProj1(tmpPath1.generic_string());
  ASSERT_NO_THROW(tmpProj1 = Project(tmpPath1.generic_string()));
  ASSERT_FALSE(tmpProj1.valid());
  ASSERT_FALSE(bfs::is_directory(tmpPath1));

  bfs::path tmpPath2 =
    bfs::path(os::tmpdir()).parent_path() / bfs::unique_path();
  ASSERT_FALSE(bfs::exists(tmpPath2));
  ASSERT_NO_THROW(Project::importProject(
    tmpPath1.generic_string()
  , tmpPath2.generic_string()
  , Project::ArchiveFormat_AutoDetect));
  ASSERT_TRUE(bfs::exists(tmpPath2));
  fToRemove.insert(tmpPath2);

  map<string, size_t> contentSummary = summarizeContent(tmpPath2);
  ASSERT_TRUE(fContentSummary == contentSummary);

  Project tmpProj2(tmpPath2.generic_string());
  ASSERT_TRUE(fMetadata == tmpProj2.getMetadata());
}
