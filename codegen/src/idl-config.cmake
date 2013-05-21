
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
    SRC ${idl} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${idl}
      -c ${class_name}
      -o ${output_dir}/${_filename}
      -m proxyFuture)
endfunction()

function(qi_create_interface idl class_name output_dir _out)
  _qi_find_idl(IDL)
  string(TOLOWER ${class_name} _filename)
  set(_filename "${_filename}_interface.hpp")
  message("tgt ${output_dir}/${_filename}")
  set(${_out} ${output_dir}/${_filename} PARENT_SCOPE)
  qi_generate_src(${output_dir}/${_filename}
    SRC ${idl} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${idl}
      -c ${class_name}
      -o ${output_dir}/${_filename}
      -m interface)
endfunction()

#! Create type/factory registration file
# \group:INCLUDE files to include in generated file
# \param:DIR directory to output file to
# \param:IDL path to input IDL file
# \param:NAME name of the class in the IDL file
# \param:CLASS_NAME if set bind under this name instead of NAME
# \param:INTERFACE set if class inherits from inteface built with qi_create_interface
# \param:FACTORY if set, register a factory for this class as a service
# \param:SERVICE if set, register this class as a service
# \param:CPP create a source file (.cpp) instead of a header (.hpp)
function(qi_create_binder _out)
  cmake_parse_arguments(ARG
    "INTERFACE;FACTORY;SERVICE;CPP"
    "DIR;NAME;CLASS_NAME;IDL"
    "INCLUDE"
    ${ARGN})
  _qi_find_idl(IDL)
  if(ARG_CPP)
    set(_ext ".cpp")
  else()
    set(_ext ".hpp")
  endif()
  if(NOT ARG_CLASS_NAME)
    set(ARG_CLASS_NAME ${ARG_NAME})
  endif()
  string(TOLOWER ${ARG_CLASS_NAME} _filename)
  set(target "${ARG_DIR}/${_filename}_bind${_ext}")
  if(ARG_INTERFACE)
    set(interface "--interface")
  endif()
  if(ARG_FACTORY)
    set(mode "cxxtyperegisterfactory")
  elseif(ARG_SERVICE)
    set(mode "cxxtyperegisterservice")
  else()
    set(mode "cxxtype")
  endif()
  if(ARG_INCLUDE)
    set(include "--include")
    FOREACH(i ${ARG_INCLUDE})
      set(includes "${includes},${i}")
    ENDFOREACH()
  endif()
  set(${_out} ${target} PARENT_SCOPE)
  qi_generate_src(${target}
    SRC ${ARG_IDL} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${ARG_IDL}
      -c ${ARG_NAME}:${mode}:${ARG_CLASS_NAME}
      -o ${target}
      -m many
      ${include} ${includes}
      ${interface}
   )
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

