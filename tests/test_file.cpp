/*
 *  Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <string>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qimessaging/file.hpp>
#include <qi/buffer.hpp>

class TestFile: public ::testing::Test
{
public:
  TestFile()
    : a_newPath()
    , a_path()
    , a_fileName("plop")
    , a_content("Nothing to say!\n")
  {
  }

protected:
  void SetUp()
  {
    a_newPath = qi::os::mktmpdir("QiFileTest");
    a_newPath.append(a_fileName, qi::unicodeFacet());
    a_path = qi::os::mktmpdir("QiFileTest");
    a_path.append(a_fileName, qi::unicodeFacet());
    FILE *fileHandle = qi::os::fopen(a_newPath.string(qi::unicodeFacet()).c_str(), "w+");
    fprintf(fileHandle, "%s", a_content.c_str());
    fclose(fileHandle);
  }

  void TearDown()
  {
    if (boost::filesystem::exists(a_newPath))
    {
      try
      {
        boost::filesystem::remove_all(a_newPath.parent_path());
      }
      catch (std::exception &)
      {
      }
    }

    if (boost::filesystem::exists(a_path))
    {
      try
      {
        boost::filesystem::remove_all(a_path.parent_path());
      }
      catch (std::exception &)
      {
      }
    }
  }

public:
  boost::filesystem::path a_newPath;
  boost::filesystem::path a_path;
  std::string             a_fileName;
  std::string             a_content;
};

TEST_F(TestFile, BasicMemoryFile)
{
  qi::File f;
  f.open(qi::Flag_Read | qi::Flag_Write | qi::Flag_Truncate);

  ASSERT_TRUE(!!(qi::Flag_Read & f.flags()));
  ASSERT_TRUE(qi::Flag_Write & f.flags());
  ASSERT_TRUE(!!(qi::Flag_Truncate & f.flags()));
  ASSERT_FALSE(qi::Flag_Create & f.flags());
  ASSERT_FALSE(qi::Flag_Invalid & f.flags());
}

TEST_F(TestFile, BasicDiskFile)
{
  qi::File f;
  f.open(a_newPath.string(qi::unicodeFacet()), qi::Flag_Read);
  f.save(a_path.string(qi::unicodeFacet()));

  FILE *file = qi::os::fopen(a_path.string(qi::unicodeFacet()).c_str(), "r");
  char *r = new char[a_content.size() + 1];
  ASSERT_TRUE(!!r) << "Cannot allocate read buffer.";
  fread(r, 1, a_content.size(), file);
  r[a_content.size()] = '\0';
  fclose(file);

  ASSERT_EQ(a_fileName, f.fileName());
  ASSERT_EQ(a_content, std::string(r));
  delete[] r;

  ASSERT_TRUE(!!(qi::Flag_Read & f.flags()));
  ASSERT_FALSE(qi::Flag_Write & f.flags());
  ASSERT_FALSE(qi::Flag_Truncate & f.flags());
  ASSERT_FALSE(qi::Flag_Create & f.flags());
  ASSERT_FALSE(qi::Flag_Invalid & f.flags());
}

TEST_F(TestFile, fileStream)
{
  qi::File f;
  f.open(a_newPath.string(qi::unicodeFacet()), qi::Flag_Read);
  f.save(a_path.string(qi::unicodeFacet()));

  qi::Buffer buf;
  qi::ODataStream d(buf);
  d << f;

  qi::IDataStream d2(buf);
  qi::File fres;
  d2 >> fres;

  FILE *file = qi::os::fopen(fres.path().c_str(), "r");
  char *r = new char[a_content.size() + 1];
  ASSERT_TRUE(!!r) << "Cannot allocate read buffer.";

  fread(r, 1, a_content.size(), file);
  r[a_content.size()] = '\0';
  fclose(file);

  ASSERT_EQ(a_fileName, fres.fileName());
  ASSERT_EQ(a_content, std::string(r));
  delete[] r;

  ASSERT_TRUE(!!(qi::Flag_Read & fres.flags()));
  ASSERT_FALSE(qi::Flag_Write & fres.flags());
  ASSERT_FALSE(qi::Flag_Truncate & fres.flags());
  ASSERT_FALSE(qi::Flag_Create & fres.flags());
  ASSERT_FALSE(qi::Flag_Invalid & f.flags());
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
