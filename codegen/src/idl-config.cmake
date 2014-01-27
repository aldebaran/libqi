
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
    set(${where} ${genname} PARENT_SCOPE)
  else()
    # Proper reset since find_path writes to cache
    set(_idlfile_find_path "_idlfile_find_path-NOTFOUND" CACHE FILEPATH   "Cleared." FORCE)
    find_path(_idlfile_find_path "${idlname}.xml"
      PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/share/idl
      ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/share/idl
      $ENV{IDL_PATH}
    )
    if(_idlfile_find_path)
      message("found ${idlname}.xml at ${idlfile} from ${CMAKE_CURRENT_BINARY_DIR}")
      set(${where} "${_idlfile_find_path}/${idlname}.xml" PARENT_SCOPE)
    else()
      qi_error("Could not find IDL file ${idlname}.xml for ${classname} (searched ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} $ENV{IDL_PATH} )")
    endif()
  endif()

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
  if (NOT ARG_OUTPUT_DIR)
    set(ARG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  set(tgt "${ARG_OUTPUT_DIR}/${ARG_OUTPUT_FILE}")
  set(${_out_filename} ${tgt} PARENT_SCOPE)
  message("-- Interface: ${class_name} ${tgt} from ${idlfile}")
  qi_generate_src(${tgt}
    SRC ${idlfile} ${IDL}
    COMMAND ${_python_executable} ${IDL}
      ${idlfile}
      --search-path "${ARG_SEARCH_PATH}:${CMAKE_CURRENT_SOURCE_DIR}"
      -o ${tgt}
      -m interface)
endfunction()

#! Create client support library for given classes
function(qi_create_client_lib libname)
  cmake_parse_arguments(ARG "" "PREFIX" "CLASSES;INCLUDE;SRC;DEPENDS" ${ARGN})
  _qi_find_idl(IDL)
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
  qi_create_lib("${libname}" SHARED SRC ${sources} ${ARG_SRC} DEPENDS ${ARG_DEPENDS})
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
  set(${files} "${names}" PARENT_SCOPE)
endfunction()
