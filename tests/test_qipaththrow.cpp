/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <gtest/gtest.h>

#include "../src/sdklayout.hpp"
#include <qi/path.hpp>
#include <qi/error.hpp>


//should not throw, it depends on qi::program()
#if 0
TEST(qiPathThrowTests, qiPathFuncThrow)
{
  ASSERT_THROW({qi::path::sdkPrefix();}, qi::os::QiException);
  ASSERT_THROW({qi::path::getSdkPrefixes();}, qi::os::QiException);
  ASSERT_THROW({qi::path::addOptionalSdkPrefix("/build/sdk");}, qi::os::QiException);
  ASSERT_THROW({qi::path::clearOptionalSdkPrefix();}, qi::os::QiException);
  ASSERT_THROW({qi::path::findBin("qipath_test");}, qi::os::QiException);
  ASSERT_THROW({qi::path::findLib("libqi.so");}, qi::os::QiException);
  ASSERT_THROW({qi::path::findConf("naoqi", "autoinit");}, qi::os::QiException);
  ASSERT_THROW({qi::path::findData("naoqi", "");}, qi::os::QiException);
  ASSERT_THROW({qi::path::confPaths();}, qi::os::QiException);
  ASSERT_THROW({qi::path::dataPaths();}, qi::os::QiException);
  ASSERT_THROW({qi::path::binPaths();}, qi::os::QiException);
  ASSERT_THROW({qi::path::libPaths();}, qi::os::QiException);
  ASSERT_THROW({qi::path::userWritableDataPath("naoqi", "");}, qi::os::QiException);
  ASSERT_THROW({qi::path::userWritableConfPath("naoqi", "");}, qi::os::QiException);
}

TEST(qiPathThrowTests, sdkLayoutFuncThrow)
{

  qi::SDKLayout* sdkl = new qi::SDKLayout();

  ASSERT_THROW({sdkl->sdkPrefix();}, qi::os::QiException);
  ASSERT_THROW({sdkl->getSdkPrefixes();}, qi::os::QiException);
  ASSERT_THROW({sdkl->addOptionalSdkPrefix("/build/sdk");}, qi::os::QiException);
  ASSERT_THROW({sdkl->clearOptionalSdkPrefix();}, qi::os::QiException);
  ASSERT_THROW({sdkl->findBin("qipath_test");}, qi::os::QiException);
  ASSERT_THROW({sdkl->findLib("libqi.so");}, qi::os::QiException);
  ASSERT_THROW({sdkl->findConf("naoqi", "autoinit");}, qi::os::QiException);
  ASSERT_THROW({sdkl->findData("naoqi", "");}, qi::os::QiException);
  ASSERT_THROW({sdkl->confPaths();}, qi::os::QiException);
  ASSERT_THROW({sdkl->dataPaths();}, qi::os::QiException);
  ASSERT_THROW({sdkl->binPaths();}, qi::os::QiException);
  ASSERT_THROW({sdkl->libPaths();}, qi::os::QiException);
  ASSERT_THROW({sdkl->userWritableDataPath("naoqi", "");}, qi::os::QiException);
  ASSERT_THROW({sdkl->userWritableConfPath("naoqi", "");}, qi::os::QiException);

  delete sdkl;
}
#endif
