qi_stage_cmake(src/idl-config.cmake)
#qi_install_program forces current_build_dir as prefix

qi_stage_script(src/idl.py DEPENDS QITYPE TRAMPOLINE PYTHON)
qi_stage_script(src/shlib.py)
qi_install_program(src/idl.py)
if (NOT ANDROID)
  qi_install_python(src/shlib.py)
endif()

if(NOT APPLE AND NOT I_AM_A_ROBOT AND NOT ANDROID)
  add_subdirectory("tests")
endif()
