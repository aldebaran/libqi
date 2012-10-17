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
                   qitype/api.hpp
                   qitype/details/functiontype.hxx
                   qitype/details/genericobjectbuilder.hxx
                   qitype/details/genericobject.hxx
                   qitype/details/genericvalue.hxx
                   qitype/details/genericvaluespecialized.hxx
                   qitype/details/methodtype.hxx
                   qitype/details/objecttypebuilder.hxx
                   qitype/details/signal.hxx
                   qitype/details/type.hxx
                   qitype/details/typedispatcher.hxx
                   qitype/details/typeimpl.hxx
                   qitype/details/typelist.hxx
                   qitype/details/typemap.hxx
                   qitype/details/typepointer.hxx
                   qitype/details/typestring.hxx
                   qitype/details/typetuple.hxx
                   qitype/dynamicobject.hpp
                   qitype/functiontype.hpp
                   qitype/genericobjectbuilder.hpp
                   qitype/genericobject.hpp
                   qitype/genericvalue.hpp
                   qitype/metamethod.hpp
                   qitype/metaobject.hpp
                   qitype/metasignal.hpp
                   qitype/methodtype.hpp
                   qitype/objectfactory.hpp
                   qitype/objecttypebuilder.hpp
                   qitype/signal.hpp
                   qitype/signature.hpp
                   qitype/type.hpp
                   qitype/typeobject.hpp
                   qitype/typedispatcher.hpp)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)

set(SOURCES ${PUBLIC_HEADERS}
            src/dynamicobject.cpp
            src/functiontype.cpp
            src/genericobjectbuilder.cpp
            src/genericvalue.cpp
            src/metamethod.cpp
            src/metasignal.cpp
            src/metasignal_p.cpp
            src/metasignal_p.hpp
            src/metaobject.cpp
            src/metaobject_p.hpp
            src/object_p.hpp
            src/object.cpp
            src/objectfactory.cpp
            src/objecttypebuilder.cpp
            src/signal.cpp
            src/signal_p.hpp
            src/signatureconvertor.cpp
            src/signatureconvertor.hpp
            src/signatureiterator.cpp
            src/staticobjecttype.cpp
            src/type.cpp
            src/typeint.cpp
            src/typetuple.cpp
            src/signature.cpp
            )

qi_create_lib(qitype ${SOURCES} SHARED DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD)
qi_stage_lib(qitype)

add_subdirectory("tests")
