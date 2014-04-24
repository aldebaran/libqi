## Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 2.8)
project(translate)
find_package(qibuild)

qi_create_bin(test_translate main.cpp NO_INSTALL)
qi_use_lib(test_translate qi)

qi_create_trad(test_translate "po")
