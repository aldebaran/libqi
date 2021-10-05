## Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 3.16) # Ubuntu 20.04 comes with CMake 3.16.3
project(LibQi)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

qi_install_header(qitype/fwd.hpp
                  qitype/anyfunction.hpp
                  qitype/anyobject.hpp
                  qitype/anyreference.hpp
                  qitype/anyvalue.hpp
                  qitype/binarycodec.hpp
                  qitype/dynamicobject.hpp
                  qitype/dynamicobjectbuilder.hpp
                  qitype/jsoncodec.hpp
                  qitype/metamethod.hpp
                  qitype/metaobject.hpp
                  qitype/metaproperty.hpp
                  qitype/metasignal.hpp
                  qitype/objectfactory.hpp
                  qitype/objecttypebuilder.hpp
                  qitype/property.hpp
                  qitype/proxyproperty.hpp
                  qitype/proxysignal.hpp
                  qitype/signal.hpp
                  qitype/signature.hpp
                  qitype/typeinterface.hpp
                  qitype/typeobject.hpp
                  qitype/typedispatcher.hpp
                  KEEP_RELATIVE_PATHS)
qi_stage_header_only_lib(qitype DEPENDS QI)
