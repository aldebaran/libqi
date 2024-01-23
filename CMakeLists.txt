## Copyright (c) 2012, 2013, 2014 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.
##
## qi CMake project
## ================
##
## Parameters:
## -----------
## QI_VERSION
## ~~~~~~~~~~
## A version number that will be used for this project. If this variable is
## undefined, CMake will read the project version number from the
## `project.json` file in the source directory.
##
## BUILD_TESTING
## ~~~~~~~~~~~~~
## If set to true, enables building the tests. See the documentation of the
## `CTest` module for more details on this variable.
##
## WITH_SYSTEMD
## ~~~~~~~~~~~~
## If set to true, enables building libqi with usage of systemd libraries.
## This variable is ignored if pkg-config is not found on the system or if
## the systemd library is not found using pkg-config.
##
## WITH_BOOST_LOCALE
## ~~~~~~~~~~~~~~~~~
## If set to true, enables building libqi with usage of Boost locale library.
## This variable is ignored if the library was not found in Boost.
##
## Targets
## -------
## The targets defined by the project are:
## - `qi::ka`: header-only library.
## - `qi::qi`: shared library.
## - `qi::testssl`: static library, links with `qi::qi`.
## - `qi::testsession`: static library, links with `qi::qi`.
##
## All targets are defined for:
## - libqi as a CMake subproject (through `add_subdirectory`).
## - a build directory of the project.
## - an installation of the project.
##
## The last two cases are implemented by generating package configuration
## files (`X-config.cmake`) in the build directory and in the installation
## directory.

cmake_minimum_required(VERSION 3.23)

# - Parse project description file
file(READ "${CMAKE_CURRENT_LIST_DIR}/project.json" project_json)

# - Define the project version number.
if(NOT QI_VERSION)
  string(JSON QI_VERSION GET "${project_json}" version)
endif()

include(cmake/ParseVersion.cmake)
parse_version(QI "${QI_VERSION}")
message(STATUS "Project version is ${QI_VERSION_FULL} (interpreted as ${QI_VERSION} for CMake)")

project(qi VERSION "${QI_VERSION}")

include(CMakeDependentOption)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(FeatureSummary)
include(cmake/BuildType.cmake)
include(cmake/MakePackageConfigFile.cmake)

# Enable testing with CTest. This defines the BUILD_TESTING option.
# Disable tests by default when cross compiling.
set(build_testing_default TRUE)
if(CMAKE_CROSSCOMPILING)
  set(build_testing_default FALSE)
endif()
option(BUILD_TESTING "Build the testing tree." "${build_testing_default}")
include(CTest)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  set(IS_GCC_OR_CLANG TRUE)
else()
  set(IS_GCC_OR_CLANG FALSE)
endif()

# Enable more warnings. This is set globally for every targets of this project,
# but is not reflected on the interface of this project libraries. Consumers
# of these libraries are free to add (or not) these flags themselves.
if(IS_GCC_OR_CLANG)
  add_compile_options(-Wall -Wextra)
elseif(MSVC)
  add_compile_options(/W3)
endif()

# Add QI_WITH_TESTS definitions to all targets in the build, to enable export
# of some internal symbols needed by tests.
if(BUILD_TESTING)
  add_compile_definitions(QI_WITH_TESTS)
endif()

# Output everything directly in predefined directories of the build tree.
# This is required by the SDK layout implementation.
# Also write a "path.conf" file, which is required for executing some tests.
set(sdk_dir "${CMAKE_BINARY_DIR}/sdk")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${sdk_dir}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${sdk_dir}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${sdk_dir}/lib")
set(path_conf_dir "${sdk_dir}/share/qi")
set(path_conf_file_path "${path_conf_dir}/path.conf")
if(NOT EXISTS "${path_conf_file_path}")
  file(MAKE_DIRECTORY "${path_conf_dir}")
  file(TOUCH "${path_conf_file_path}")
endif()

##############################################################################
# External Packages
##############################################################################
# Boost
set(
  boost_required_components
    atomic
    chrono
    date_time
    filesystem
    program_options
    random
    regex
    thread
)
find_package(
  Boost REQUIRED
  COMPONENTS ${boost_required_components}
  OPTIONAL_COMPONENTS locale
)
# Used by "qi-config.cmake".
set(BOOST_VERSION "${Boost_VERSION}")

set(with_boost_locale_descr
  "Enable building libqi with usage of Boost Locale libraries."
)
# We do not rely on the "Boost_LOCALE_FOUND" variable normally set by the
# FindBoost CMake module because Conan generated files for Boost fail to
# declare which components are found or not.
set(has_boost_locale FALSE)
if(TARGET Boost::locale)
  set(has_boost_locale TRUE)
endif()
cmake_dependent_option(
  WITH_BOOST_LOCALE
  "${with_boost_locale_descr}"
  ON has_boost_locale OFF
)
add_feature_info(
    WITH_BOOST_LOCALE
    WITH_BOOST_LOCALE
    "${with_boost_locale_descr}"
)

set(BOOST_COMPONENTS ${boost_required_components})
if(WITH_BOOST_LOCALE)
  list(APPEND BOOST_COMPONENTS locale)
endif()

# OpenSSL
find_package(OpenSSL REQUIRED)

# Parse the exact version number to remove the letter suffix so we may
# reuse it in find_package.
parse_version(OPENSSL "${OPENSSL_VERSION}")

# Threads
if(UNIX)
  find_package(Threads REQUIRED)
endif()

# systemd
set(has_systemd FALSE)
find_package(PkgConfig MODULE)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(SYSTEMD systemd)
  set(has_systemd SYSTEMD_FOUND)
endif()

set(with_systemd_descr
  "Enable building libqi with usage of systemd libraries."
)
cmake_dependent_option(
  WITH_SYSTEMD
  "${with_systemd_descr}"
  OFF # Disabled by default.
  has_systemd OFF
)
add_feature_info(
  WITH_SYSTEMD
  WITH_SYSTEMD
  "${with_systemd_descr}"
)

##############################################################################
# Convenience library: cxx_standard
##############################################################################
add_library(cxx_standard INTERFACE)
add_library(qi::internal::cxx_standard ALIAS cxx_standard)

# The project requires at least C++17.
target_compile_features(
  cxx_standard
  INTERFACE
    cxx_std_17
)

set_target_properties(
  cxx_standard
  PROPERTIES
    EXPORT_NAME internal::cxx_standard
)

##############################################################################
# Library: ka
##############################################################################
add_library(ka INTERFACE)
add_library(qi::ka ALIAS ka)

target_sources(
  ka

  PUBLIC
    FILE_SET source_headers
      TYPE HEADERS
      FILES
        ka/algorithm.hpp
        ka/ark/inputiter.hpp
        ka/ark/mutable.hpp
        ka/base64.hpp
        ka/concept.hpp
        ka/conceptpredicate.hpp
        ka/empty.hpp
        ka/errorhandling.hpp
        ka/flatten.hpp
        ka/functional.hpp
        ka/functor.hpp
        ka/functorcontainer.hpp
        ka/indexed.hpp
        ka/integersequence.hpp
        ka/iterator.hpp
        ka/macro.hpp
        ka/macroregular.hpp
        ka/memory.hpp
        ka/moveoncopy.hpp
        ka/mutablestore.hpp
        ka/mutex.hpp
        ka/opt.hpp
        ka/parse.hpp
        ka/productiter.hpp
        ka/range.hpp
        ka/relationpredicate.hpp
        ka/scoped.hpp
        ka/sha1.hpp
        ka/src.hpp
        ka/testutils.hpp
        ka/typetraits.hpp
        ka/unit.hpp
        ka/uri.hpp
        ka/uri/io.hpp
        ka/uri/parsing.hpp
        ka/uri/uri.hpp
        ka/uri/uri_fwd.hpp
        ka/utility.hpp
        ka/zero.hpp
)

target_include_directories(
  ka
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(
  ka
  INTERFACE
    qi::internal::cxx_standard
    Boost::headers
    Boost::date_time
    Boost::thread
)

##############################################################################
# Library: qi_objects
##############################################################################
add_library(qi_objects OBJECT)
add_library(qi::internal::qi_objects ALIAS qi_objects)

target_sources(
  qi_objects

  PUBLIC
    FILE_SET source_headers
      TYPE HEADERS
      FILES
        qi/actor.hpp
        qi/anyfunction.hpp
        qi/anymodule.hpp
        qi/anyobject.hpp
        qi/anyvalue.hpp
        qi/api.hpp
        qi/application.hpp
        qi/applicationsession.hpp
        qi/assert.hpp
        qi/async.hpp
        qi/atomic.hpp
        qi/binarycodec.hpp
        qi/buffer.hpp
        qi/clock.hpp
        qi/detail/async.hxx
        qi/detail/clock.hxx
        qi/detail/eventloop.hxx
        qi/detail/executioncontext.hpp
        qi/detail/future.hxx
        qi/detail/future_fwd.hpp
        qi/detail/futurebarrier.hpp
        qi/detail/futureunwrap.hpp
        qi/detail/log.hxx
        qi/detail/mpl.hpp
        qi/detail/print.hpp
        qi/detail/trackable.hxx
        qi/either.hpp
        qi/eventloop.hpp
        qi/flags.hpp
        qi/future.hpp
        qi/futuregroup.hpp
        qi/getenv.hpp
        qi/iocolor.hpp
        qi/jsoncodec.hpp
        qi/log.hpp
        qi/log/consoleloghandler.hpp
        qi/log/csvloghandler.hpp
        qi/log/fileloghandler.hpp
        qi/log/headfileloghandler.hpp
        qi/log/tailfileloghandler.hpp
        qi/macro.hpp
        qi/messaging/authprovider.hpp
        qi/messaging/authproviderfactory.hpp
        qi/messaging/autoservice.hpp
        qi/messaging/clientauthenticator.hpp
        qi/messaging/clientauthenticatorfactory.hpp
        qi/messaging/detail/autoservice.hxx
        qi/messaging/gateway.hpp
        qi/messaging/messagesocket_fwd.hpp
        qi/messaging/servicedirectoryproxy.hpp
        qi/messaging/serviceinfo.hpp
        qi/messaging/ssl/detail/ssl.hxx
        qi/messaging/ssl/ssl.hpp
        qi/messaging/tcpscheme.hpp
        qi/numeric.hpp
        qi/objectuid.hpp
        qi/os.hpp
        qi/path.hpp
        qi/path_conf.hpp
        qi/perf/dataperf.hpp
        qi/perf/dataperfsuite.hpp
        qi/perf/detail/dataperfsuite.hxx
        qi/perf/measure.hpp
        qi/periodictask.hpp
        qi/preproc.hpp
        qi/property.hpp
        qi/ptruid.hpp
        qi/qi.hpp
        qi/session.hpp
        qi/signal.hpp
        qi/signalspy.hpp
        qi/signature.hpp
        qi/stats.hpp
        qi/strand.hpp
        qi/tag.hpp
        qi/trackable.hpp
        qi/translator.hpp
        qi/type/detail/accessor.hxx
        qi/type/detail/anyfunction.hxx
        qi/type/detail/anyfunctionfactory.hxx
        qi/type/detail/anyiterator.hpp
        qi/type/detail/anyiterator.hxx
        qi/type/detail/anyreference.hpp
        qi/type/detail/anyreference.hxx
        qi/type/detail/anyvalue.hpp
        qi/type/detail/anyvalue.hxx
        qi/type/detail/bindtype.hxx
        qi/type/detail/buffertypeinterface.hxx
        qi/type/detail/dynamicobjectbuilder.hxx
        qi/type/detail/dynamictypeinterface.hxx
        qi/type/detail/functionsignature.hxx
        qi/type/detail/futureadapter.hpp
        qi/type/detail/futureadapter.hxx
        qi/type/detail/genericobject.hpp
        qi/type/detail/hasless.hxx
        qi/type/detail/inttypeinterface.hxx
        qi/type/detail/listtypeinterface.hxx
        qi/type/detail/manageable.hpp
        qi/type/detail/maptypeinterface.hxx
        qi/type/detail/object.hxx
        qi/type/detail/objecttypebuilder.hxx
        qi/type/detail/optionaltypeinterface.hxx
        qi/type/detail/pointertypeinterface.hxx
        qi/type/detail/property.hxx
        qi/type/detail/proxyregister.hpp
        qi/type/detail/signal.hxx
        qi/type/detail/staticobjecttype.hpp
        qi/type/detail/stringtypeinterface.hxx
        qi/type/detail/structtypeinterface.hxx
        qi/type/detail/traceanalyzer.hpp
        qi/type/detail/type.hpp
        qi/type/detail/type.hxx
        qi/type/detail/typedispatcher.hxx
        qi/type/detail/typeimpl.hxx
        qi/type/detail/typeinterface.hpp
        qi/type/dynamicobject.hpp
        qi/type/dynamicobjectbuilder.hpp
        qi/type/fwd.hpp
        qi/type/metamethod.hpp
        qi/type/metaobject.hpp
        qi/type/metaproperty.hpp
        qi/type/metasignal.hpp
        qi/type/objecttypebuilder.hpp
        qi/type/proxyproperty.hpp
        qi/type/proxysignal.hpp
        qi/type/typedispatcher.hpp
        qi/type/typeinterface.hpp
        qi/type/typeobject.hpp
        qi/types.hpp
        qi/uri.hpp
        qi/url.hpp
        qi/uuid.hpp
        qi/version.hpp

  PRIVATE
    src/application.cpp
    src/application_p.hpp
    src/buffer.cpp
    src/buffer_p.hpp
    src/bufferreader.cpp
    src/clock.cpp
    src/consoleloghandler.cpp
    src/csvloghandler.cpp
    src/dlfcn.cpp
    src/eventloop.cpp
    src/eventloop_p.hpp
    src/fileloghandler.cpp
    src/future.cpp
    src/headfileloghandler.cpp
    src/iocolor.cpp
    src/locale-light.cpp
    src/log.cpp
    src/log_p.hpp
    src/messaging/applicationsession.cpp
    src/messaging/applicationsession_internal.cpp
    src/messaging/applicationsession_internal.hpp
    src/messaging/authprovider.cpp
    src/messaging/authprovider_p.hpp
    src/messaging/boundobject.cpp
    src/messaging/boundobject.hpp
    src/messaging/clientauthenticator.cpp
    src/messaging/clientauthenticator_p.hpp
    src/messaging/message.cpp
    src/messaging/message.hpp
    src/messaging/messagedispatcher.cpp
    src/messaging/messagedispatcher.hpp
    src/messaging/messagesocket.cpp
    src/messaging/messagesocket.hpp
    src/messaging/objecthost.cpp
    src/messaging/objecthost.hpp
    src/messaging/objectregistrar.cpp
    src/messaging/objectregistrar.hpp
    src/messaging/remoteobject.cpp
    src/messaging/remoteobject_p.hpp
    src/messaging/server.cpp
    src/messaging/server.hpp
    src/messaging/servicedirectory.cpp
    src/messaging/servicedirectory.hpp
    src/messaging/servicedirectory_p.hpp
    src/messaging/servicedirectoryclient.cpp
    src/messaging/servicedirectoryclient.hpp
    src/messaging/servicedirectoryproxy.cpp
    src/messaging/serviceinfo.cpp
    src/messaging/serviceinfo_p.hpp
    src/messaging/session.cpp
    src/messaging/session_p.hpp
    src/messaging/sessionservice.cpp
    src/messaging/sessionservice.hpp
    src/messaging/sessionservices.cpp
    src/messaging/sessionservices.hpp
    src/messaging/sock/accept.hpp
    src/messaging/sock/common.hpp
    src/messaging/sock/concept.hpp
    src/messaging/sock/connect.hpp
    src/messaging/sock/connectedstate.hpp
    src/messaging/sock/connectingstate.hpp
    src/messaging/sock/disconnectedstate.hpp
    src/messaging/sock/disconnectingstate.hpp
    src/messaging/sock/error.hpp
    src/messaging/sock/macrolog.hpp
    src/messaging/sock/networkasio.cpp
    src/messaging/sock/networkasio.hpp
    src/messaging/sock/option.hpp
    src/messaging/sock/receive.hpp
    src/messaging/sock/resolve.hpp
    src/messaging/sock/send.hpp
    src/messaging/sock/socketptr.hpp
    src/messaging/sock/socketwithcontext.hpp
    src/messaging/sock/sslcontextptr.hpp
    src/messaging/sock/traits.hpp
    src/messaging/ssl/ssl.cpp
    src/messaging/streamcontext.cpp
    src/messaging/streamcontext.hpp
    src/messaging/tcpmessagesocket.cpp
    src/messaging/tcpmessagesocket.hpp
    src/messaging/tcpscheme.cpp
    src/messaging/transportserver.cpp
    src/messaging/transportserver.hpp
    src/messaging/transportserverasio_p.cpp
    src/messaging/transportserverasio_p.hpp
    src/messaging/transportsocketcache.cpp
    src/messaging/transportsocketcache.hpp
    src/messaging/uri.cpp
    src/messaging/url.cpp
    src/os.cpp
    src/path.cpp
    src/path_conf.cpp
    src/perf/dataperf.cpp
    src/perf/dataperf_p.hpp
    src/perf/dataperfsuite.cpp
    src/perf/dataperfsuite_p.hpp
    src/perf/measure.cpp
    src/periodictask.cpp
    src/print.cpp
    src/ptruid.cpp
    src/registration.cpp
    src/sdklayout-boost.cpp
    src/sdklayout.hpp
    src/strand.cpp
    src/tailfileloghandler.cpp
    src/type/anyfunction.cpp
    src/type/anymodule.cpp
    src/type/anyobject.cpp
    src/type/anyreference.cpp
    src/type/anyvalue.cpp
    src/type/binarycodec.cpp
    src/type/binarycodec_p.hpp
    src/type/dynamicobject.cpp
    src/type/dynamicobjectbuilder.cpp
    src/type/genericobject.cpp
    src/type/jsoncodec_p.hpp
    src/type/jsondecoder.cpp
    src/type/jsonencoder.cpp
    src/type/manageable.cpp
    src/type/metamethod.cpp
    src/type/metamethod_p.hpp
    src/type/metaobject.cpp
    src/type/metaobject_p.hpp
    src/type/metaproperty.cpp
    src/type/metasignal.cpp
    src/type/metasignal_p.cpp
    src/type/metasignal_p.hpp
    src/type/objecttypebuilder.cpp
    src/type/signal.cpp
    src/type/signal_p.hpp
    src/type/signalspy.cpp
    src/type/signature.cpp
    src/type/signatureconvertor.cpp
    src/type/signatureconvertor.hpp
    src/type/staticobjecttype.cpp
    src/type/structtypeinterface.cpp
    src/type/traceanalyzer.cpp
    src/type/type.cpp
    src/type/typeinterface.cpp
    src/utils.cpp
    src/utils.hpp
    src/version.cpp
)

if(UNIX)
  check_symbol_exists(_SC_HOST_NAME_MAX "unistd.h" HAVE_SC_HOST_NAME_MAX)
  if(HAVE_SC_HOST_NAME_MAX)
    set_source_files_properties(
      src/os_posix.cpp
      PROPERTIES
        COMPILE_DEFINITIONS HAVE_SC_HOST_NAME_MAX
    )
  endif()
  target_sources(
    qi_objects
    PRIVATE
      src/os_posix.cpp
      src/os_debugger_posix.cpp
  )
  if(ANDROID)
    target_sources(
      qi_objects
      PUBLIC
        FILE_SET source_headers
          TYPE HEADERS
          FILES
            qi/log/androidloghandler.hpp
      PRIVATE
        src/androidloghandler.cpp
        src/os_launch_android.cpp
    )
  else()
    target_sources(
      qi_objects
      PRIVATE src/os_launch_posix.cpp
    )
  endif()
endif()

target_link_libraries(
  qi_objects
  PUBLIC
    qi::internal::cxx_standard
    qi::ka
)

set_target_properties(
  qi_objects
  PROPERTIES
    CXX_EXTENSIONS TRUE
    CXX_VISIBILITY_PRESET hidden
    POSITION_INDEPENDENT_CODE TRUE
    EXPORT_NAME internal::qi_objects
)

if(IS_GCC_OR_CLANG)
  target_compile_options(
    qi_objects
    PRIVATE
      -Wno-unused-parameter
      -Werror=return-type
      -fno-strict-aliasing
  )
endif()

if(WIN32)
  target_sources(
    qi_objects
    PRIVATE
      src/os_launch_win32.cpp
      src/os_win32.cpp
      src/os_win32.hpp
      src/os_debugger_win32.cpp
  )

  target_compile_definitions(
    qi_objects
    PUBLIC
      _SCL_SECURE_NO_WARNINGS
      _CRT_SECURE_NO_WARNINGS
      WIN32_LEAN_AND_MEAN
      NOMINMAX
  )
endif()

# Configure "config.hpp" file.
#
# Refer to the "config.hpp.in" file to see what parameters the file defines.
set(config_hpp_file_path "${CMAKE_CURRENT_BINARY_DIR}/qi/config.hpp")
configure_file(qi/config.hpp.in "${config_hpp_file_path}" @ONLY)
target_sources(
  qi_objects
  PUBLIC
    FILE_SET generated_headers
    TYPE HEADERS
      BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
      FILES "${config_hpp_file_path}"
)

target_include_directories(
  qi_objects
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(
  qi_objects
  PUBLIC
    Boost::headers
    Boost::atomic
    Boost::chrono
    Boost::date_time
    Boost::filesystem
    Boost::program_options
    Boost::random
    Boost::regex
    Boost::thread
)

target_compile_definitions(
  qi_objects
  PUBLIC
    BOOST_FILESYSTEM_VERSION=3
  PRIVATE
    BOOST_ASIO_DISABLE_STD_CHRONO
)

if(WITH_BOOST_LOCALE)
  target_compile_definitions(
    qi_objects
    PRIVATE
      WITH_BOOST_LOCALE
  )
  target_sources(
    qi_objects
    PUBLIC
      FILE_SET source_headers
        TYPE HEADERS
        FILES
          qi/translator.hpp
    PRIVATE
      src/translator.cpp
  )
  target_link_libraries(
    qi_objects
    PRIVATE
      Boost::locale
  )
endif()

target_link_libraries(
  qi_objects
  PUBLIC
    OpenSSL::Crypto
    OpenSSL::SSL
)

if(UNIX)
  target_link_libraries(
    qi_objects
    PUBLIC
      Threads::Threads
  )
endif()

if(WITH_SYSTEMD)
  target_compile_definitions(
    qi_objects
    PUBLIC
      WITH_SYSTEMD
  )
  target_link_libraries(
    qi_objects
    PUBLIC
      systemd
  )
  target_sources(
    qi_objects
    PUBLIC
      FILE_SET source_headers
        TYPE HEADERS
        FILES
          qi/log/journaldloghandler.hpp
    PRIVATE
      src/journaldloghandler.cpp
  )
endif()

if(ANDROID)
  target_link_libraries(qi_objects PUBLIC log)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
  target_link_libraries(qi_objects PUBLIC dl rt)
endif()

if(WIN32)
  target_link_libraries(qi_objects PUBLIC wsock32 ws2_32 iphlpapi psapi)
endif()

##############################################################################
# Library: qi
##############################################################################
add_library(qi SHARED)
add_library(qi::qi ALIAS qi)

target_link_libraries(
  qi
  PUBLIC
    qi::ka
    qi::internal::cxx_standard
    qi::internal::qi_objects
)

set_target_properties(
  qi
  PROPERTIES
    VERSION "${qi_VERSION}"
    SOVERSION "${qi_VERSION_MAJOR}"
)

##############################################################################
# Library: testssl
##############################################################################
add_library(testssl STATIC)
add_library(qi::testssl ALIAS testssl)

# TODO: Move sources in current directory.
set(
  libtestssl_base_dir
  "${CMAKE_CURRENT_LIST_DIR}/tests/messaging/libtestssl"
)

target_sources(
  testssl
  PUBLIC
    FILE_SET source_headers
      TYPE HEADERS
      FILES
        "${libtestssl_base_dir}/testssl/testssl.hpp"
      BASE_DIRS "${libtestssl_base_dir}"
  PRIVATE
    "${libtestssl_base_dir}/src/testssl.cpp"
)

target_include_directories(
  testssl
  PUBLIC
    $<BUILD_INTERFACE:${libtestssl_base_dir}>
)

target_link_libraries(
  testssl
  PUBLIC
    qi::internal::cxx_standard
    qi::qi
)

set_target_properties(
  testssl
  PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

##############################################################################
# Library: testsession
#
# A helper library to test two sessions communicating in various modes
##############################################################################
add_library(testsession STATIC)
add_library(qi::testsession ALIAS testsession)

# TODO: Move sources in current directory.
set(
  libtestsession_base_dir
  "${CMAKE_CURRENT_LIST_DIR}/tests/messaging/libtestsession"
)

target_sources(
  testsession
  PUBLIC
    FILE_SET source_headers
      TYPE HEADERS
      FILES
        "${libtestsession_base_dir}/testsession/testsession.hpp"
        "${libtestsession_base_dir}/testsession/testsessionpair.hpp"
      BASE_DIRS "${libtestsession_base_dir}"
  PRIVATE
    "${libtestsession_base_dir}/src/testmode.cpp"
    "${libtestsession_base_dir}/src/testsessionpair.cpp"
    "${libtestsession_base_dir}/src/testsession.cpp"
    "${libtestsession_base_dir}/src/testsession_p.hpp"
    "${libtestsession_base_dir}/src/testsession_p.cpp"
    "${libtestsession_base_dir}/src/testsessionerror.cpp"
    "${libtestsession_base_dir}/src/trafficgenerator.hpp"
    "${libtestsession_base_dir}/src/trafficgenerator.cpp"
    "${libtestsession_base_dir}/src/populationgenerator.cpp"
    "${libtestsession_base_dir}/src/populationgenerator.cpp"
    "${libtestsession_base_dir}/src/sessioninitializer.hpp"
    "${libtestsession_base_dir}/src/sessioninitializer.cpp"
    "${libtestsession_base_dir}/src/defaultservice.hpp"
    "${libtestsession_base_dir}/src/defaultservice.cpp"
)

target_include_directories(
  testsession
  PUBLIC
    $<BUILD_INTERFACE:${libtestsession_base_dir}>
)

target_link_libraries(
  testsession
  PUBLIC
    qi::internal::cxx_standard
    qi::qi
    qi::testssl
)

set_target_properties(
  testsession
  PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

##############################################################################
# Installation & export of targets
#
# We create 2 package configuration files:
# - qi
# - testsession
#
# The goal is to support client projects using a previous representation of the
# package, where a `testsession` package was also available.
#
# The export set is the same: `qi-targets`. It is exported twice with different
# namespaces and filenames.
##############################################################################
install(
  TARGETS ka qi testssl testsession cxx_standard qi_objects
  EXPORT qi-targets
  RUNTIME COMPONENT runtime
  LIBRARY COMPONENT runtime
  ARCHIVE COMPONENT devel
  FILE_SET source_headers COMPONENT devel
  FILE_SET generated_headers COMPONENT devel
  PUBLIC_HEADER
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

make_package_config_file(qi-targets qi COMPONENT devel)
make_package_config_file(qi-targets testsession COMPONENT devel)

##############################################################################
# qimodule
##############################################################################
find_package(
  qimodule REQUIRED
  PATHS cmake
  # Disable CMAKE_FIND_ROOT_PATH modes, when cross-compiling.
  NO_CMAKE_FIND_ROOT_PATH
)

set(
  qimodule_INSTALL_CMAKEDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/qimodule"
  CACHE STRING
  "Path where qimodule CMake files are to be installed."
)

set(
  qimodule_config_file_path
  "${CMAKE_CURRENT_BINARY_DIR}/qimodule-config.cmake"
)

configure_file(
  cmake/qimodule-config.cmake
  "${qimodule_config_file_path}"
  COPYONLY
)

install(
  FILES
    "${qimodule_config_file_path}"
  DESTINATION "${qimodule_INSTALL_CMAKEDIR}"
  COMPONENT devel
)

##############################################################################
# Examples
##############################################################################
add_custom_target(examples)
add_subdirectory(examples)

##############################################################################
# Tests
##############################################################################
add_feature_info(
  BUILD_TESTING
  BUILD_TESTING
  "Enable building project tests."
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

##############################################################################
feature_summary(WHAT ALL)
