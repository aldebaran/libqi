
find_package(clang)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

qi_create_bin(qiclang SRC src/qiclang.cpp DEPENDS clang QI)
qi_stage_bin(qiclang)

qi_stage_cmake(src/idl-config.cmake)
#qi_install_program forces current_build_dir as prefix

qi_stage_script(src/idl.py DEPENDS QITYPE TRAMPOLINE PYTHON)
qi_stage_script(src/shlib.py)
qi_install_program(src/idl.py)
if (NOT ANDROID)
  qi_install_python(src/shlib.py)
endif()
