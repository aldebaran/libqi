
qi_global_set(qi_create_service_file ${CMAKE_CURRENT_LIST_FILE})

#! \ Find idl.py or error
function(_qi_find_idl where)
  #First, locate idl.py
  qi_global_get(qi_create_service_file_local qi_create_service_file)
  get_filename_component(cdir "${qi_create_service_file_local}" PATH)
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
  qi_persistent_set(${where} ${IDLPY})
endfunction()


function(qi_create_proxy idl class_name output_dir _out)
  _qi_find_idl(IDL)
  string(TOLOWER ${class_name} _filename)
  set(_filename "${_filename}_proxy.hpp")
  message("tgt ${output_dir}/${_filename}")
  set(${_out} ${output_dir}/${_filename} PARENT_SCOPE)
  qi_generate_src(${output_dir}/${_filename}
    SRC ${idl}
    COMMAND ${_python_executable} ${IDL}
      ${idl}
      -c ${class_name}
      -o ${output_dir}/${_filename}
      -m proxyFuture)
endfunction()

#! Create an IDL file by parsing C++ header files.
# \group:SRC C++ source/headers file to parse
# \group:CLASSES name of the classes for which to generate idl
# \param:PREFIX path where to generate IDL files
# \param:files name of variable that is filled by generated file names
function(qi_create_idl files)
  cmake_parse_arguments(ARG
    ""
    "PREFIX"
    "SRC;CLASSES"
    ${ARGN})
  _qi_find_idl(IDL)
  set(names "")
  if (NOT ARG_PREFIX)
    set(ARG_PREFIX ".")
  endif()
  foreach(c ${ARG_CLASSES})
    set(target "${ARG_PREFIX}/${c}.xml")
    qi_generate_src(${target}
      SRC ${ARG_SRC}
      COMMAND  ${_python_executable} ${IDL}
      ${ARG_SRC}
      -c ${c}
      -o ${target}
      -m idl)
    list(APPEND names "${target}")
  endforeach(c)
  qi_persistent_set(${files} "${names}")
endfunction()

#! Create a naoqi service and companion files
# \group:CLASSES list of class names to be exposed to naoqi as objects
# \group:SERVICES list of class names to be exposed to naoqi as services
# \group:INCLUDE files to #include in generated files
# \group:INCLUDE_SERVICE files to #include in generated files
# \group:DOXYSRC sources to pass to doxygen, but not to compile
# \param:IDL the IDL file to use as input
# \param:PROXY generate specialized proxy header and install them
# \param:SPLIT_BIND generate binding code in a separate file per class
function(qi_create_service name)
  cmake_parse_arguments(ARG
    "NOBINDLL;NO_INSTALL;NO_FPIC;SHARED;STATIC;INTERNAL;PROXY;SPLIT_BIND"
    "SUBFOLDER;IDL"
    "SRC;DOXYSRC;SUBMODULE;DEPENDS;CLASSES;SERVICES;INCLUDE;INCLUDE_SERVICE" ${ARGN})
  message("parsing args: ${ARG_CLASSES}")
  #First, locate idl.py
  _qi_find_idl(IDLPY)
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
    set(kclasses "-k ${classes}")
  endif()
  if (classes)
    set(classes "-c ${classes}")
  endif()
  message("input classes: ${ARG_SERVICES} ${ARG_CLASSES}")
  message("classes: '${classes}'")

  # Generate comma separated includes for the various cases
  #  user includes
  foreach(inc ${ARG_INCLUDE})
    set(includes "${includes},${inc}")
  endforeach()
  #  user includes plus generated interface
  set(includesiface "${includes},${CMAKE_CURRENT_BINARY_DIR}/${name}_proxy.hpp")
  #  service includes
  foreach(inc ${ARG_INCLUDE_SERVICE})
    set(service_includes "${service_includes},${inc}")
  endforeach()
  if (NOT service_includes)
    set(service_includes "${includes}")
  endif()
  set(service_includes "${service_includes},${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp")


  # Generate proxy if asked to
  if (ARG_PROXY)
    qi_generate_src(
      ${CMAKE_CURRENT_BINARY_DIR}/${name}_proxy.hpp
      SRC ${ARG_SRC} ${ARG_DOXYSRC}
      COMMAND ${_python_executable} ${IDLPY}
        ${IDL}
        ${ARG_SRC} ${ARG_DOXYSRC}
        "${classes}"
        -m proxyFuture --interface
        ${kclasses}
        -I ${includes}
        -o ${CMAKE_CURRENT_BINARY_DIR}/${name}_proxy.hpp
    )
    list(APPEND ARG_SRC ${CMAKE_CURRENT_BINARY_DIR}/${name}_proxy.hpp)
  endif()
  # Generate binder
  # Build command argument from classes and modules
  # Put classes before services, most likely there is a dependency
  # in this order

  set(bindname ${name}_bind.cpp)
  set(bindtarget ${bindname})
  if (ARG_SPLIT_BIND)
    foreach(c ${ARG_SERVICES})
       string(TOLOWER ${c} cl)
       qi_generate_src(
       ${CMAKE_CURRENT_BINARY_DIR}/${cl}_bind.cpp
       SRC ${ARG_SRC} ${ARG_DOXYSRC}
       COMMAND ${_python_executable} ${IDLPY}
         ${IDL}
         ${ARG_SRC} ${ARG_DOXYSRC}
         ${kclasses}
         -m cxxservicebouncerregister --interface
         -c ${c}:cxxservicebouncerregister
         -o ${CMAKE_CURRENT_BINARY_DIR}/${cl}_bind.cpp
         -I "${service_includes}"
      )
    endforeach(c)
    foreach(c ${ARG_CLASSES})
      message("Generating bind for ${c}")
      string(TOLOWER ${c} cl)
      qi_generate_src(
      ${CMAKE_CURRENT_BINARY_DIR}/${cl}_bind.cpp
      SRC ${ARG_SRC} ${ARG_DOXYSRC}
      COMMAND ${_python_executable} ${IDLPY}
        ${IDL}
        ${ARG_SRC} ${ARG_DOXYSRC}
        ${kclasses}
        -m cxxservicebouncerregister --interface
        -c ${c}:cxxservicebouncer
        -o ${CMAKE_CURRENT_BINARY_DIR}/${cl}_bind.cpp
        -I "${service_includes}"
      )
    endforeach(c)
  else()
    message("Generating merged bind")
    set (carg '')
    foreach(class ${ARG_CLASSES})
      set(carg "${carg},${class}:cxxservicebouncer")
    endforeach()
    foreach(module ${ARG_SERVICES})
      set(carg "${carg},${module}:cxxservicebouncerregister")
    endforeach(module)
    if (carg)
      set(carg "-c ${carg}")
    endif()
    message("carg: '${carg}'")
    qi_generate_src(
       ${CMAKE_CURRENT_BINARY_DIR}/${name}_bind.cpp
       SRC ${ARG_SRC} ${ARG_DOXYSRC}
       COMMAND ${_python_executable} ${IDLPY}
         ${IDL}
         ${ARG_SRC} ${ARG_DOXYSRC}
         ${kclasses}
         -m cxxservicebouncerregister --interface
         "${carg}"
         -o ${CMAKE_CURRENT_BINARY_DIR}/${name}_bind.cpp
         -I "${service_includes}"
    )
    set(ARG_SRC ${CMAKE_CURRENT_BINARY_DIR}/${name}_bind.cpp ${ARG_SRC})
  endif()
  # Generate interface
  qi_generate_src(
     ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp
     SRC ${ARG_SRC} ${ARG_DOXYSRC}
     COMMAND ${_python_executable} ${IDLPY}
       ${IDL}
       ${ARG_SRC} ${ARG_DOXYSRC}
       ${classes}
       ${kclasses}
       -m interface --interface
       -o ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp
       -I "${includes}"
  )
  # Add interface (or it wont be generated) to sources
  set(ARG_SRC ${CMAKE_CURRENT_BINARY_DIR}/${name}_interface.hpp ${ARG_SRC})
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
