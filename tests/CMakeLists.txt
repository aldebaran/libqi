## Copyright (c) 2011, Aldebaran Robotics
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Aldebaran Robotics nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
## WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
## LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
## ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
qi_create_bin(testlaunch NO_INSTALL testlaunch.cpp)
qi_create_bin(check_env  NO_INSTALL check_env.cpp)

qi_use_lib(check_env QI)

set_target_properties(testlaunch PROPERTIES FOLDER tests)
set_target_properties(check_env  PROPERTIES FOLDER tests)

qi_create_gtest(test_qipath      SRC test_qipath.cpp      DEPENDS QI GTEST)
qi_create_gtest(test_qilocal     SRC test_locale.cpp      DEPENDS QI GTEST)
qi_create_gtest(test_qios        SRC test_qios.cpp        DEPENDS QI GTEST)
qi_create_gtest(test_thread      SRC test_thread.cpp      DEPENDS QI BOOST_THREAD)
qi_create_gtest(test_qilaunch    SRC test_qilaunch.cpp    DEPENDS QI GTEST)
qi_create_gtest(test_qilog_sync  SRC test_qilog_sync.cpp  DEPENDS QI GTEST)
qi_create_gtest(test_qilog_async SRC test_qilog_async.cpp DEPENDS QI GTEST)
