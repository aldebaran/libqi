include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(_headers
  ../qipython/api.hpp
  ../qipython/pyinit.hpp
  ../qipython/pyexport.hpp
  ../qipython/pyapplication.hpp
  ../qipython/pysession.hpp
  ../qipython/pyfuture.hpp
  ../qipython/pyobject.hpp
  ../qipython/pyproperty.hpp
  ../qipython/pythreadsafeobject.hpp
  ../qipython/pysignal.hpp
  ../qipython/pyasync.hpp
  ../qipython/pyobjectfactory.hpp
  ../qipython/pylog.hpp
  ../qipython/pypath.hpp
  ../qipython/pytranslator.hpp
  ../qipython/error.hpp
  ../qipython/gil.hpp
  ../qipython/trace.hpp
)

set(_sources
  ../src/pyinit.cpp
  ../src/pyexport.cpp
  ../src/pyapplication.cpp
  ../src/pysession.cpp
  ../src/pyfuture.cpp
  ../src/pyobject.cpp
  ../src/pyobjectconverter.cpp
  ../src/pyproperty.cpp
  ../src/pysignal.cpp
  ../src/pyasync.cpp
  ../src/pyobjectfactory.cpp
  ../src/pylog.cpp
  ../src/pypath.cpp
  ../src/pytranslator.cpp
  ../src/error.cpp
  ../src/trace.cpp
)

qi_install_header(${_headers} SUBFOLDER qipython)

message(STATUS "Generating libs for python${pyver}")


qi_create_lib(qipython${pyver} SHARED
  ${_headers}
  ${_sources}
)

qi_use_lib(qipython${pyver} BOOST_PYTHON${pyver} PYTHON${pyver} QI QITYPE QIMESSAGING )

qi_stage_lib(qipython${pyver})


if("${pyver}" STREQUAL "")
  qi_create_python_ext(_qi SHARED ../src/boostpymain.cpp)
  qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/../qi KEEP_RELATIVE_PATHS)
  qi_use_lib(_qi qipython)
else()
  # Hack until qibuild fully supports python3
  add_library(_qi3 SHARED ../src/boostpymain.cpp)
  set_target_properties(_qi3
      PROPERTIES PREFIX ""
      OUTPUT_NAME "_qi"
      LIBRARY_OUTPUT_DIRECTORY
        "${QI_SDK_DIR}/${QI_SDK_LIB}/python3.4/site-packages"
  )
  qi_use_lib(_qi3 qipython3)
endif()
