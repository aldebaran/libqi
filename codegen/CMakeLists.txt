qi_stage_cmake(src/idl-config.cmake)
#qi_install_program forces current_build_dir as prefix
qi_install_program(src/idl.py)
add_subdirectory("tests")

