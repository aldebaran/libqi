
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
    PATHS ${cdir} ${cdir}/../../bin ${cdir}/../bin
    )
  if (NOT IDLPY)
    qi_error("idl.py not found")
  endif()
  qi_persistent_set(${where} ${IDLPY})
endfunction()

#! Find IDL file for given class name, in filesystem or in targets
function(qi_find_idl_for classname where)
  string(REPLACE "::" "/" idlname ${classname})
  set(genname "${CMAKE_CURRENT_BINARY_DIR}/share/idl/${idlname}.xml")
  get_source_file_property(isgenerated ${genname} GENERATED)
  if (TARGET ${genname} OR isgenerated)
    qi_persistent_set(${where} ${genname})
  else()
    find_path(idlfile "${idlname}.xml"
      PATHS ${CMAKE_CURRENT_BINARY_DIR} $ENV{IDL_PATH}
    )
    if(idlfile)
      qi_persistent_set(${where} idlfile)
    else()
      qi_error("Could not find IDL file for ${classname}")
    endif()
  endif()

endfunction()


function(qi_create_proxy idl class_name output_dir _out)
cmake_parse_arguments(ARG
    "NO_FUTURE"
    ""
    ""
    ${ARGN})
  _qi_find_idl(IDL)
  string(REPLACE "::" ";" split_class ${class_name})
  list(REVERSE split_class)
  list(GET split_class 0 class)
  string(TOLOWER ${class} _filename)
  set(_filename "${_filename}_proxy.hpp")
  set(${_out} ${output_dir}/${_filename} PARENT_SCOPE)
  if(ARG_NO_FUTURE OR ARG_INTERFACE)
    set(_mode proxy)
  else()
    set(_mode proxyFuture)
  endif()
  qi_generate_src(${output_dir}/${_filename}
    SRC ${idl} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${idl}
      -c ${class_name}
      -o ${output_dir}/${_filename}
      -m ${_mode}
      ${_interface}
      )
endfunction()

#! Create interface header for given class
#\group:SEARCH_PATH path where to search IDL files
#\param:IDL name of IDL xml file (will be searched for if not specified)
#\param:_out_filename variable that will be set with full path and name of generated file
#\param:class_name Name of the class for which to generate interface
#\param:OUTPUT_DIR Directory in which to put the result
#\param:OUTPUT_FILE File name (default: <classname>.hpp)
function(qi_create_interface _out_filename class_name)
  cmake_parse_arguments(ARG "" "IDL;OUTPUT_DIR;OUTPUT_FILE" "SEARCH_PATH" ${ARGN})
  _qi_find_idl(IDL)
  qi_find_idl_for(${class_name} idlfile "${ARG_SEARCH_PATH}")
  # Get bare class name without namespaces
  if (NOT ARG_OUTPUT_FILE)
    string(REPLACE "::" ";" split_class ${class_name})
    list(REVERSE split_class)
    list(GET split_class 0 class)
    string(TOLOWER ${class} _filename)
    set(ARG_OUTPUT_FILE "${_filename}.hpp")
  endif()
  message("COIN ${ARG_OUTPUT_DIR}")
  if (NOT ARG_OUTPUT_DIR)
    set(ARG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  set(tgt "${ARG_OUTPUT_DIR}/${ARG_OUTPUT_FILE}")
  set(${_out_filename} ${tgt} PARENT_SCOPE)
  qi_generate_src(${tgt}
    SRC ${idlfile} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${idlfile}
      -o ${tgt}
      -m interface)
endfunction()

#! Create client support library for given classes
function(qi_create_client_lib libname)
  cmake_parse_arguments(ARG "" "PREFIX" "CLASSES;INCLUDE" ${ARGN})
  if (NOT ARG_PREFIX)
    set(ARG_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  set(sources "")
  set(search_path  "${ARG_PREFIX}:${CMAKE_CURRENT_SOURCE_DIR}")
  foreach(c ${ARG_CLASSES})
    string(REPLACE "::" "-" class ${c})
    string(REPLACE "::" "/" idlname ${c})
    set(target "${ARG_PREFIX}/client-${class}.cc")
    # we need to find the IDL file for proper dependency tracking
    qi_find_idl_for(${c} idlfile)
    qi_generate_src(${target}
      SRC ${idlfile}
      COMMAND  ${_python_executable} ${IDL}
      --prefix ${ARG_PREFIX} # IDLS might be around here
      --search-path "${search_path}"
      -c ${c}
      -o "${target}"
      --cxx-signature-mapping "'${_type_map}'"
      -m client)
    list(APPEND sources "${target}")
  endforeach()
  qi_create_lib("${libname}" SHARED SRC ${sources})
endfunction()

#! Create a skeleton implementation of given class
function(qi_create_skeleton target)
  cmake_parse_arguments(ARG "" "CLASS" "INCLUDE;SEARCHPATH" ${ARGN})
  # we need to find the IDL file for proper dependency tracking
  string(REPLACE "::" "/" idlname ${ARG_CLASS})
  find_path(idlfile ${idlname}
    PATHS ${CMAKE_CURRENT_BINARY_DIR}
  )
  set(search_path  "${ARG_SEARCHPATH}:${CMAKE_CURRENT_SOURCE_DIR}")
  qi_generate_src(${target}
    SRC ${idlfile}
    COMMAND ${_python_executable} ${IDL}
     -m cxxskel
     -o ${target}
     -c ${ARG_CLASS}
     --include-file "'${ARG_INCLUDE}'"
     --search-path "${search_path}"
     )
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
    "FACTORY;SERVICE;CPP"
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

  string(REPLACE "::" ";" split_class ${ARG_CLASS_NAME})
  list(REVERSE split_class)
  list(GET split_class 0 class)
  string(TOLOWER ${class} _filename)
  set(target "${ARG_DIR}/${_filename}_bind${_ext}")
  if(ARG_FACTORY)
    set(mode "cxxtyperegisterfactory")
  elseif(ARG_SERVICE)
    set(mode "cxxtyperegisterservice")
  else()
    set(mode "cxxtype")
  endif()
  if(ARG_INCLUDE)
    set(include "--include-file")
    FOREACH(i ${ARG_INCLUDE})
      set(includes "${includes},${i}")
    ENDFOREACH()
  endif()
  set(${_out} ${target} PARENT_SCOPE)
  qi_generate_src(${target}
    SRC ${ARG_IDL} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${ARG_IDL}
      -c ${ARG_NAME}:${mode}
      -o ${target}
      -n ${ARG_CLASS_NAME}
      -m many
      ${include} ${includes}
      ${interface}
   )
endfunction()

#! Create IDL files by parsing C++ header files.
# \group:SRC C++ source/headers file to parse
#                those will also be used as buid dependencies, so
#                you should list all relevant headers here
# \group:CLASSES fully qualified name of the classes for which to generate idl
#                IDL files will be generated for this classes and all their
#                dependencies
# \group:TYPE_MAP Extra c++ types to signature mapping (cxxtype=signature)
# \param:PREFIX path where to generate IDL files
# \param:files name of variable that is filled by generated file names
#              note that other files might be also generated
function(qi_create_idl files)
  cmake_parse_arguments(ARG
    ""
    "PREFIX"
    "SRC;CLASSES;TYPE_MAP"
    ${ARGN})
  _qi_find_idl(IDL)
  set(names "")
  # If we are building qiclang, pass it along
  if (TARGET qiclang)
    get_target_property(qiclangname qiclang OUTPUT_NAME)
    if (NOT qiclangname)
      set(qiclangname "qiclang")
    endif()
    get_target_property(qiclangdir qiclang RUNTIME_OUTPUT_DIRECTORY)
    set(qiclangopt "--qiclang;${qiclangdir}/${qiclangname}")
  endif()
  if (NOT ARG_PREFIX)
    set(ARG_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/share/idl)
  endif()
  string(REPLACE ";" "," _type_map "${ARG_TYPE_MAP}")
  foreach(c ${ARG_CLASSES})
    string(REPLACE "::" "/" class ${c})
    set(target "${ARG_PREFIX}/${class}.xml")
    qi_generate_src(${target}
      SRC ${ARG_SRC}
      COMMAND  ${_python_executable} ${IDL}
      ${ARG_SRC}
      -c ${c}
      -p "${ARG_PREFIX}"
      --cxx-signature-mapping "'${_type_map}'"
      ${qiclangopt}
      -m idl)
    list(APPEND names "${target}")
  endforeach(c)
  qi_persistent_set(${files} "${names}")
endfunction()
