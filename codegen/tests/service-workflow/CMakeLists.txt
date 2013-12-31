
# Follow the service use case of the documentation



find_package(idl REQUIRED)


#user wrote cat.hpp interface, generate the idl files from it
qi_create_idl(idl_files
  SRC ${CMAKE_CURRENT_SOURCE_DIR}/cat.hpp
  CLASSES animals::Cat
)
message("Will generate IDL: ${idl_files}")


#generate the client support files
qi_create_client_lib(ANIMALS_CAT_LIB
  CLASSES animals::Cat
)

#generate an implementation skeleton
qi_create_skeleton(${CMAKE_CURRENT_BINARY_DIR}/catimpl-skel.cpp
  CLASS animals::Cat)

# fill it with stuff
qi_generate_src(${CMAKE_CURRENT_BINARY_DIR}/catimpl.cpp
  SRC ${CMAKE_CURRENT_BINARY_DIR}/catimpl-skel.cpp
  COMMAND ${CMAKE} -E copy ${CMAKE_CURRENT_BINARY_DIR}/catimpl-skel.cpp ${CMAKE_CURRENT_BINARY_DIR}/catimpl.cpp)

# compile the service
qi_create_lib(ANIMALS_CAT_IMPL_LIB
  MODULE
  SRC ${CMAKE_CURRENT_BINARY_DIR}/catimpl.cpp)
