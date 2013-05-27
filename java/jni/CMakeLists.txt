cmake_minimum_required(VERSION 2.8)
project(qimessaging-jni)

find_package(qibuild)
if (NOT ANDROID)
  find_package(JNI REQUIRED)
  include_directories(${JNI_INCLUDE_DIRS})
endif()

include_directories("jni")

set(SOURCE
   jni/session_jni.hpp
   jni/application_jni.hpp
   jni/callbridge.hpp
   jni/servicedirectory_jni.hpp
   jni/objectbuilder.hpp
   jni/jnitools.hpp
   jni/jobjectconverter.hpp
   jni/map_jni.hpp
   jni/enumeration_jni.hpp
   jni/list_jni.hpp
   jni/future_jni.hpp
   jni/tuple_jni.hpp
   jni/futurehandler.hpp
   jni/object_jni.hpp
   jni/object.hpp

   src/session_jni.cpp
   src/application_jni.cpp
   src/callbridge.cpp
   src/servicedirectory_jni.cpp
   src/objectbuilder.cpp
   src/jnitools.cpp
   src/jobjectconverter.cpp
   src/map_jni.cpp
   src/enumeration_jni.cpp
   src/list_jni.cpp
   src/future_jni.cpp
   src/tuple_jni.cpp
   src/futurehandler.cpp
   src/object_jni.cpp
   src/object.cpp
   )

# Compile qimessaging java compatibility layer using jni
qi_create_lib(qimessagingjni ${SOURCE} SHARED DEPENDS QIMESSAGING)
