##
## Copyright (C) 2009, 2010, 2011, 2012 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.8.0)
project(libqitype)

find_package(qibuild)

enable_testing()

qi_sanitize_compile_flags(HIDDEN_SYMBOLS)


qi_add_optional_package(PYTHON)
set(WITH_PYTHON ON)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PERF     "Performances test" ON)

add_definitions(" -DBOOST_FILESYSTEM_VERSION=3 ")

if (WIN32)
  add_definitions(" -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN")
endif()

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PUBLIC_HEADERS
                   qitype/details/accessor.hxx
                   qitype/details/anyreference.hpp
                   qitype/details/anyreference.hxx
                   qitype/details/anyvalue.hpp
                   qitype/details/anyvalue.hxx
                   qitype/details/anyfunction.hxx
                   qitype/details/anyfunctionfactory.hxx
                   qitype/details/anyiterator.hxx
                   qitype/details/bindtype.hxx
                   qitype/details/functionsignature.hxx
                   qitype/details/dynamicobjectbuilder.hxx
                   qitype/details/anyobject.hxx
                   qitype/details/hasless.hxx
                   qitype/details/objecttypebuilder.hxx
                   qitype/details/property.hxx
                   qitype/details/signal.hxx
                   qitype/details/type.hxx
                   qitype/details/buffertypeinterface.hxx
                   qitype/details/typedispatcher.hxx
                   qitype/details/dynamictypeinterface.hxx
                   qitype/details/typeimpl.hxx
                   qitype/details/typeinterface.hpp
                   qitype/details/inttypeinterface.hxx
                   qitype/details/listtypeinterface.hxx
                   qitype/details/maptypeinterface.hxx
                   qitype/details/pointertypeinterface.hxx
                   qitype/details/stringtypeinterface.hxx
                   qitype/details/structtypeinterface.hxx
                   qitype/api.hpp
                   qitype/anyfunction.hpp
                   qitype/anyobject.hpp
                   qitype/anyreference.hpp
                   qitype/anyvalue.hpp
                   qitype/binarycodec.hpp
                   qitype/dynamicobject.hpp
                   qitype/dynamicobjectbuilder.hpp
                   qitype/fwd.hpp
                   qitype/jsoncodec.hpp
                   qitype/manageable.hpp
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
                   qitype/typedispatcher.hpp)


set(qitype_STATIC_BUILD OFF)

qi_create_config_h(_confighpp qitype/config.hpp.in qitype/config.hpp)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)

set(SOURCES ${PUBLIC_HEADERS}
            src/binarycodec.cpp
            src/binarycodec_p.hpp
            src/dynamicobject.cpp
            src/dynamicobjectbuilder.cpp
            src/anyfunction.cpp
            src/anyreference.cpp
            src/anyvalue.cpp
            src/anyobject_p.hpp
            src/anyobject.cpp
            src/jsoncodec_p.hpp
            src/jsondecoder.cpp
            src/jsonencoder.cpp
            src/metamethod.cpp
            src/metaproperty.cpp
            src/metasignal.cpp
            src/metasignal_p.cpp
            src/metasignal_p.hpp
            src/metaobject.cpp
            src/metaobject_p.hpp
            src/objectfactory.cpp
            src/objecttypebuilder.cpp
            src/signal.cpp
            src/signal_p.hpp
            src/signatureconvertor.cpp
            src/signatureconvertor.hpp
            src/staticobjecttype.cpp
            src/typeinterface.cpp
            src/inttypeinterface.cpp
            src/structtypeinterface.cpp
            src/signature.cpp
            )

list(APPEND SOURCES ${_confighpp})

qi_create_lib(qitype ${SOURCES} SHARED DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD)
qi_stage_lib(qitype)

if(NOT ANDROID)
  add_subdirectory("tests")
  add_subdirectory("codegen")
endif()

