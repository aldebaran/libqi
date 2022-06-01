## Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 3.16) # Ubuntu 20.04 comes with CMake 3.16.3
project(LibQi)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

qi_install_header(qimessaging/applicationsession.hpp
                  qimessaging/autoservice.hpp
                  qimessaging/serviceinfo.hpp
                  qimessaging/session.hpp
                  qimessaging/url.hpp)
qi_stage_header_only_lib(qimessaging DEPENDS QI QITYPE)
