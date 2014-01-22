
# Follow the service use case of the documentation



find_package(idl REQUIRED)

# first thing, check that the user interface file compiles
# Not in a workflow, just for early-detection of problems
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
qi_create_lib(cattestcompile SRC cat-test-compiles.cpp DEPENDS QITYPE)

#user wrote cat.hpp interface, generate the idl files from it
qi_create_idl(idl_files
  SRC ${CMAKE_CURRENT_SOURCE_DIR}/cat.hpp
  CLASSES animals::Cat
)
message("Will generate IDL: ${idl_files}")


#generate the client support files
qi_create_client_lib(animals_cat
  CLASSES animals::Cat
)
qi_stage_lib(animals_cat)

#generate an implementation skeleton
qi_create_skeleton(${CMAKE_CURRENT_BINARY_DIR}/catimpl-skel.cpp
  CLASS animals::Cat)

# fill it with stuff
qi_generate_src(${CMAKE_CURRENT_BINARY_DIR}/catimpl.cpp
  SRC ${CMAKE_CURRENT_BINARY_DIR}/catimpl-skel.cpp ${CMAKE_CURRENT_SOURCE_DIR}/cat-patch.cmake
  COMMAND ${CMAKE_COMMAND}
    -Dinput=${CMAKE_CURRENT_BINARY_DIR}/catimpl-skel.cpp
    -Doutput=${CMAKE_CURRENT_BINARY_DIR}/catimpl.cpp
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cat-patch.cmake)

# compile the service
qi_create_lib(cat
  MODULE
  SRC ${CMAKE_CURRENT_BINARY_DIR}/catimpl.cpp DEPENDS animals_cat)

# compile a second hand-written service
qi_create_lib(lynx
  MODULE
  SRC lynx.cpp DEPENDS animals_cat)

# compile a test client
qi_create_test(test_service_workflow
  SRC cattest.cpp
  DEPENDS animals_cat QITYPE GTEST)
#workaround for ubuntu/debian who decided it was a good idea to
# break the default link semantic by passing as-needed.
if(NOT WIN32)
set_target_properties(test_service_workflow PROPERTIES
  LINK_FLAGS "-Wl,-no-as-needed")
endif()
