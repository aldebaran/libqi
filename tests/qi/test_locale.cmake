execute_process(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE DATE)
if (${DATE} GREATER 20160120)
  # LOL
  add_definitions("-DQI_API_TEXPORT=__attribute__((deprecated))")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()
