
qi_global_set(qi_create_module_file ${CMAKE_CURRENT_LIST_FILE})

#! Create a naoqi module
# \group:CLASSES list of class names to be exposed to naoqi as objects
# \group:SERVICES list of class names to be exposed to naoqi as services
# \group:HEADERS header files to use, will use SRC if not set
# \group:DOXYSRC sources to pass to doxygen, but not to compile
# \param:IDL the IDL file to use as input
# \param:PROXY generate specialized proxy header and install them
function(qi_create_module name)
  message("qi_create_module: ${ARGN}")
  cmake_parse_arguments(ARG
    "NOBINDLL;NO_INSTALL;NO_FPIC;SHARED;STATIC;INTERNAL;PROXY"
    "SUBFOLDER;IDL"
    "SRC;DOXYSRC;SUBMODULE;DEPENDS;CLASSES;SERVICES;HEADERS;INCLUDE" ${ARGN})
  message("parsing args: ${ARG_CLASSES}")
  #First, locate idl.py
  qi_global_get(qi_create_module_file_local qi_create_module_file)
  get_filename_component(cdir "${qi_create_module_file_local}" PATH)
  find_program(_python_executable
    NAMES python2 python python.exe
    NO_CMAKE_FIND_ROOT_PATH)
  if (NOT _python_executable)
    qi_error("needs python executable in PATH")
  endif()
  find_program(IDLPY
    name idl.py
    PATHS ${cdir}/../../../codegen/src ${cdir}/../../../bin
    )
  if (NOT IDLPY)
    qi_error("idl.py not found")
  endif()
  #ARGN are sources too
  set(ARG_SRC ${ARG_UNPARSED_ARGUMENTS} ${ARG_SRC})
  #Reformat arguments to be able to bounce them
  if (ARG_NO_INSTALL)
    set(ARGOPT_NO_INSTALL NO_INSTALL)
  endif()
  if (ARG_NO_FPIC)
    set(ARGOPT_NO_FPIC NO_FPIC)
  endif()
  if (ARG_SHARED)
    set(ARGOPT_SHARED SHARED)
  endif()
  if (ARG_STATIC)
    set(ARGOPT_STATIC STATIC)
  endif()
  if (ARG_INTERNAL)
    set(ARGOPT_INTERNAL INTERNAL)
  endif()
  if ("${ARG_HEADERS}" STREQUAL "")
    set(ARG_HEADERS ${ARG_SRC})
  endif()
  if ("${ARG_IDL}" STREQUAL "")
    # no IDL specified, generate it from sources and classes
    # or do nothing, will be generated at each call
  else()
    set(IDL ${ARG_IDL})
  endif()
  set(classes '')
  foreach(c ${ARG_SERVICES})
    set(classes "${classes},${c}")
  endforeach()
  foreach(c ${ARG_CLASSES})
    set(classes "${classes},${c}")
  endforeach()
  if (classes)
    set(classes "-c ${classes}")
  endif()
  message("input classes: ${ARG_SERVICES} ${ARG_CLASSES}")
  message("classes: '${classes}'")
  # Generate proxy if asked to
  if (ARG_PROXY)
    qi_generate_src(
      ${CMAKE_CURRENT_BINARY_DIR}/${name}_proxy.hpp
      SRC ${ARG_SRC}
      COMMAND ${_python_executable} ${IDLPY}
        ${IDL}
        "${classes}"
        -m proxy --interface
        -I ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp
        -o ${CMAKE_CURRENT_BINARY_DIR}/${name}_proxy.hpp
    )
  endif()
  # Generate binder
  # Build command argument from classes and modules
  set (carg '')
  foreach(module ${ARG_SERVICES})
    set(carg "${carg},${module}:cxxservicebouncerregister")
  endforeach(module)
  foreach(class ${ARG_CLASSES})
    set(carg "${carg},${class}:cxxservicebouncer")
  endforeach()
  if (carg)
    set(carg "-c ${carg}")
  endif()
  message("carg: '${carg}'")
  set(includes ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp)
  foreach(inc ${ARG_INCLUDE})
    set(includes "${includes},${inc}")
  endforeach()
  qi_generate_src(
     ${CMAKE_CURRENT_BINARY_DIR}/${name}_bind.cpp
     COMMAND ${_python_executable} ${IDLPY}
       ${IDL}
       ${ARG_SRC} ${ARG_DOXYSRC}
       -m cxxservicebouncerregister --interface
       "${carg}"
       -o ${CMAKE_CURRENT_BINARY_DIR}/${name}_bind.cpp
       -I "${includes}"
  )
  # Generate interface
  qi_generate_src(
     ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp
     COMMAND ${_python_executable} ${IDLPY}
       ${IDL}
       ${ARG_SRC} ${ARG_DOXYSRC}
       ${classes}
       -m interface --interface
       -o ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp
  )
  # Add binder and interface (or it wont be generated) to sources
  set(ARG_SRC ${CMAKE_CURRENT_BINARY_DIR}/${name}_bind.cpp ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp ${ARG_SRC})
  #invoke qi_create_lib
  qi_create_lib(${name}
    "${ARGVAL_SUBFOLDER}"
    "${ARGOPT_NO_BINDLL}"
    "${ARGOPT_NO_INSTALL}"
    "${ARGOPT_NO_FPIC}"
    "${ARGOPT_SHARED}"
    "${ARGOPT_STATIC}"
    "${ARGOPT_INTERNAL}"
    SRC ${ARG_SRC}
    SUBMODULE ${ARG_SUBMODULE}
    DEPENDS ${ARG_DEPENDS}
  )
endfunction()
