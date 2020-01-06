include_directories(".")
qi_stage_dir(".")

set(TESTSSL_HEADERS
  "testssl/testssl.hpp"
)

set(TESTSSL_SOURCES
  "src/testssl.cpp"
)

qi_create_lib(testssl
  STATIC
  SRC
    ${TESTSSL_HEADERS}
    ${TESTSSL_SOURCES}
  DEPENDS
    qi
)

qi_install_header(${TESTSSL_HEADERS} KEEP_RELATIVE_PATHS)
qi_stage_lib(testssl)

