qi_stage_cmake(src/idl-config.cmake)
#qi_install_program forces current_build_dir as prefix
qi_install_program(src/idl.py)

option(WITH_CODEGEN_TESTS       "Run codegen tests"       OFF)
if (WITH_CODEGEN_TESTS)
  add_subdirectory("tests")
endif()

