qi_stage_cmake(src/idl-config.cmake)
#qi_install_program forces current_build_dir as prefix

qi_stage_script(src/idl.py)
qi_stage_script(src/shlib.py)
qi_install_program(src/idl.py)
qi_install_python(src/shlib.py)

if(NOT APPLE AND NOT I_AM_A_ROBOT)
  add_subdirectory("tests")
endif()
