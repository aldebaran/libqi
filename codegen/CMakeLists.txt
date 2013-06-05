qi_stage_cmake(src/idl-config.cmake)
#qi_install_program forces current_build_dir as prefix

qi_stage_script(src/idl.py)
qi_stage_script(src/shlib.py)
qi_install_program(src/idl.py)
qi_install_python(src/shlib.py)

option(WITH_CODEGEN_TESTS       "Run codegen tests"       OFF)
if (WITH_CODEGEN_TESTS)
  add_subdirectory("tests")
endif()

