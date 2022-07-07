find_package(idl REQUIRED)

# PART 1: in service project

qi_create_interface(_header Adder)

# Generate the proxies
qi_create_client_lib(adder_fromservice_support CLASSES Adder SRC ${_header})
qi_stage_lib(adder_fromservice_support)

# Build the service
qi_create_lib(adder_fromservice
  MODULE
  SRC adder.cpp
  DEPENDS qi adder_fromservice_support
)

# PART 2: in client project
# Generate the client which uses the proxy.

include_directories(${CMAKE_CURRENT_BINARY_DIR})
qi_create_test(test_adder_fromservice SRC test_adder.cpp DEPENDS adder_fromservice_support qi TIMEOUT 10)
if(NOT WIN32)
set_target_properties(test_adder_fromservice PROPERTIES
  LINK_FLAGS "-Wl,-no-as-needed")
endif()
