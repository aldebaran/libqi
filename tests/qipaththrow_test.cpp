/*
** qipaththrow_test.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue May 10 11:24:23 2011 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
*/

#include <gtest/gtest.h>

#include <qi/path/sdklayout.hpp>
#include <qi/path.hpp>


//should not throw, it depends on qi::program()
#if 0
TEST(qiPathThrowTests, qiPathFuncThrow)
{
  ASSERT_THROW({qi::path::getSdkPrefix();}, qi::PathException);
  ASSERT_THROW({qi::path::getSdkPrefixes();}, qi::PathException);
  ASSERT_THROW({qi::path::addOptionalSdkPrefix("/build/sdk");}, qi::PathException);
  ASSERT_THROW({qi::path::clearOptionalSdkPrefix();}, qi::PathException);
  ASSERT_THROW({qi::path::findBinary("qipath_test");}, qi::PathException);
  ASSERT_THROW({qi::path::findLibrary("libqi.so");}, qi::PathException);
  ASSERT_THROW({qi::path::findConfiguration("naoqi", "autoinit");}, qi::PathException);
  ASSERT_THROW({qi::path::findData("naoqi", "");}, qi::PathException);
  ASSERT_THROW({qi::path::getConfigurationPaths();}, qi::PathException);
  ASSERT_THROW({qi::path::getDataPaths();}, qi::PathException);
  ASSERT_THROW({qi::path::getBinaryPaths();}, qi::PathException);
  ASSERT_THROW({qi::path::getLibraryPaths();}, qi::PathException);
  ASSERT_THROW({qi::path::getUserWritableDataPath("naoqi", "");}, qi::PathException);
  ASSERT_THROW({qi::path::getUserWritableConfigurationPath("naoqi", "");}, qi::PathException);
}

TEST(qiPathThrowTests, sdkLayoutFuncThrow)
{

  qi::SDKLayout* sdkl = new qi::SDKLayout();

  ASSERT_THROW({sdkl->getSdkPrefix();}, qi::PathException);
  ASSERT_THROW({sdkl->getSdkPrefixes();}, qi::PathException);
  ASSERT_THROW({sdkl->addOptionalSdkPrefix("/build/sdk");}, qi::PathException);
  ASSERT_THROW({sdkl->clearOptionalSdkPrefix();}, qi::PathException);
  ASSERT_THROW({sdkl->findBinary("qipath_test");}, qi::PathException);
  ASSERT_THROW({sdkl->findLibrary("libqi.so");}, qi::PathException);
  ASSERT_THROW({sdkl->findConfiguration("naoqi", "autoinit");}, qi::PathException);
  ASSERT_THROW({sdkl->findData("naoqi", "");}, qi::PathException);
  ASSERT_THROW({sdkl->getConfigurationPaths();}, qi::PathException);
  ASSERT_THROW({sdkl->getDataPaths();}, qi::PathException);
  ASSERT_THROW({sdkl->getBinaryPaths();}, qi::PathException);
  ASSERT_THROW({sdkl->getLibraryPaths();}, qi::PathException);
  ASSERT_THROW({sdkl->getUserWritableDataPath("naoqi", "");}, qi::PathException);
  ASSERT_THROW({sdkl->getUserWritableConfigurationPath("naoqi", "");}, qi::PathException);

  delete sdkl;
}
#endif
