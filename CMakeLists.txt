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
                   qi/anyreference.hpp
                   qi/signal.hpp
                   qi/anyvalue.hpp

                   qitype/fwd.hpp
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

                   qi/type/details/signal.hxx
                   qi/type/details/property.hxx
                   qi/type/details/accessor.hxx
                   qi/type/details/anyreference.hpp
                   qi/type/details/anyreference.hxx
                   qi/type/details/anyvalue.hpp
                   qi/type/details/anyvalue.hxx
                   qi/type/details/anyfunction.hxx
                   qi/type/details/anyfunctionfactory.hxx
                   qi/type/details/anyiterator.hpp
                   qi/type/details/anyiterator.hxx
                   qi/type/details/bindtype.hxx
                   qi/type/details/functionsignature.hxx
                   qi/type/details/dynamicobjectbuilder.hxx
                   qi/type/details/anyobject.hxx
                   qi/type/details/hasless.hxx
                   qi/type/details/objecttypebuilder.hxx
                   qi/type/details/type.hxx
                   qi/type/details/buffertypeinterface.hxx
                   qi/type/details/typedispatcher.hxx
                   qi/type/details/dynamictypeinterface.hxx
                   qi/type/details/typeimpl.hxx
                   qi/type/details/typeinterface.hpp
                   qi/type/details/inttypeinterface.hxx
                   qi/type/details/listtypeinterface.hxx
                   qi/type/details/maptypeinterface.hxx
                   qi/type/details/pointertypeinterface.hxx
                   qi/type/details/stringtypeinterface.hxx
                   qi/type/details/structtypeinterface.hxx
                   qi/type/details/type.hpp
                   qi/type/details/manageable.hpp
                   qi/type/details/traceanalyzer.hpp

                   qi/type/api.hpp
                   qi/type/binarycodec.hpp
                   qi/type/dynamicobject.hpp
                   qi/type/dynamicobjectbuilder.hpp
                   qi/type/fwd.hpp
                   qi/type/jsoncodec.hpp
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
            src/structtypeinterface.cpp
            src/type.cpp
            src/signature.cpp
            src/traceanalyzer.cpp
            src/registration.cpp
            )

list(APPEND SOURCES ${_confighpp})

qi_create_lib(qitype ${SOURCES} SHARED DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD)
qi_stage_lib(qitype)

option(DISABLE_CODEGEN "disable the code generation (broken)" ON)

add_subdirectory("tests")
if(NOT ANDROID AND NOT DISABLE_CODEGEN)
  add_subdirectory("codegen")
endif()
