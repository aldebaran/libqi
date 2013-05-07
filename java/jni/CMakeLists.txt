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
   jni/genericobject_jni.hpp
   jni/servicedirectory_jni.hpp
   jni/objectbuilder_jni.hpp
   jni/jnitools.hpp
   jni/jobjectconverter.hpp
   jni/map_jni.hpp
   jni/enumeration_jni.hpp
   jni/list_jni.hpp
   jni/future_jni.hpp
   jni/tuple_jni.hpp
   jni/futurehandler.hpp

   src/session_jni.cpp
   src/application_jni.cpp
   src/genericobject_jni.cpp
   src/servicedirectory_jni.cpp
   src/objectbuilder_jni.cpp
   src/jnitools.cpp
   src/jobjectconverter.cpp
   src/map_jni.cpp
   src/enumeration_jni.cpp
   src/list_jni.cpp
   src/future_jni.cpp
   src/tuple_jni.cpp
   src/futurehandler.cpp
   )

# Compile qimessaging java compatibility layer using jni
qi_create_lib(qimessagingjni ${SOURCE} SHARED DEPENDS QIMESSAGING)
