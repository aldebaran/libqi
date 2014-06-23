##
## Copyright (C) 2009, 2010, 2011, 2012, 2013 Aldebaran Robotics
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
                   qi/anyfunction.hpp
                   qi/anyobject.hpp
                   qi/signature.hpp
                   qi/property.hpp
                   qi/signal.hpp
                   qi/anyvalue.hpp

                   qi/type/detail/signal.hxx
                   qi/type/detail/property.hxx
                   qi/type/detail/accessor.hxx
                   qi/type/detail/anyreference.hpp
                   qi/type/detail/anyreference.hxx
                   qi/type/detail/anyvalue.hpp
                   qi/type/detail/anyvalue.hxx
                   qi/type/detail/anyfunction.hxx
                   qi/type/detail/anyfunctionfactory.hxx
                   qi/type/detail/anyiterator.hpp
                   qi/type/detail/anyiterator.hxx
                   qi/type/detail/bindtype.hxx
                   qi/type/detail/functionsignature.hxx
                   qi/type/detail/dynamicobjectbuilder.hxx
                   qi/type/detail/genericobject.hpp
                   qi/type/detail/object.hxx
                   qi/type/detail/async.hxx
                   qi/type/detail/proxyregister.hpp
                   qi/type/detail/hasless.hxx
                   qi/type/detail/objecttypebuilder.hxx
                   qi/type/detail/type.hxx
                   qi/type/detail/buffertypeinterface.hxx
                   qi/type/detail/typedispatcher.hxx
                   qi/type/detail/dynamictypeinterface.hxx
                   qi/type/detail/typeimpl.hxx
                   qi/type/detail/typeinterface.hpp
                   qi/type/detail/inttypeinterface.hxx
                   qi/type/detail/listtypeinterface.hxx
                   qi/type/detail/maptypeinterface.hxx
                   qi/type/detail/pointertypeinterface.hxx
                   qi/type/detail/stringtypeinterface.hxx
                   qi/type/detail/structtypeinterface.hxx
                   qi/type/detail/type.hpp
                   qi/type/detail/manageable.hpp
                   qi/type/detail/traceanalyzer.hpp

                   qi/type/api.hpp
                   qi/binarycodec.hpp
                   qi/type/dynamicobject.hpp
                   qi/type/dynamicobjectbuilder.hpp
                   qi/type/fwd.hpp
                   qi/jsoncodec.hpp
                   qi/type/metamethod.hpp
                   qi/type/metaobject.hpp
                   qi/type/metaproperty.hpp
                   qi/type/metasignal.hpp
                   qi/type/objectfactory.hpp
                   qi/type/objecttypebuilder.hpp
                   qi/type/proxyproperty.hpp
                   qi/type/proxysignal.hpp
                   qi/type/typeinterface.hpp
                   qi/type/typeobject.hpp
                   qi/type/typedispatcher.hpp)


set(qitype_STATIC_BUILD OFF)

qi_create_config_h(_confighpp qi/type/config.hpp.in qi/type/config.hpp)

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
            src/genericobject.cpp
            src/jsoncodec_p.hpp
            src/jsondecoder.cpp
            src/jsonencoder.cpp
            src/manageable.cpp
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
            src/structtypeinterface.cpp
            src/type.cpp
            src/signature.cpp
            src/traceanalyzer.cpp
            src/registration.cpp
            )

list(APPEND SOURCES ${_confighpp})

qi_create_lib(qitype ${SOURCES} SHARED DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD BOOST_LOCALE)
qi_stage_lib(qitype INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/compat")

option(DISABLE_CODEGEN "disable the code generation (broken)" ON)

add_subdirectory("compat")
add_subdirectory("tests")
if(NOT ANDROID AND NOT DISABLE_CODEGEN)
  add_subdirectory("codegen")
endif()
