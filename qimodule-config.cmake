##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran.com>
##
## Copyright (C) 2014 Aldebaran Robotics
##

#!
# create a qi module
#
# it's the normal qi_create_lib except that:
# - if the name is separated by point, it automatically add SUBFOLDERs to qi_create_lib
#     naoqi.foo.bar -> SUBFOLDER naoqi/foo/
# - a file is created to state that the module is a c++ module
#     sdk/share/qi/naoqi.foo.bar.mod
# - the created library is always SHARED
# - a depends on libqi is added
#
function(qi_create_module name)
  set(_namelist)
  string(FIND "${name}" "." _idx REVERSE)

  string(SUBSTRING "${name}" "0" "${_idx}" _pkgname)
  #only libname
  if (_idx EQUAL -1)
    set(_pkgname "")
    set(_libname "${name}")
  else()  #pkg + lib name
    math(EXPR _idxlib "${_idx} + 1")
    string(SUBSTRING "${name}" "${_idxlib}" "-1"      _libname)
  endif()

  string(REPLACE "." "/" _pkgpathname "${_pkgname}")

  qi_info("Module: ${name}")

  #TODO: check for SUBFOLDER spec already provided... FAIL in that case..
  qi_create_lib("${name}" SHARED ${ARGN} SUBFOLDER "${_pkgpathname}" NO_LOG)
  qi_use_lib("${name}" qi)
  set_target_properties("${name}" PROPERTIES OUTPUT_NAME "${_libname}")

  set(_modfilename "${name}.mod")
  file(WRITE "${QI_SDK_DIR}/${QI_SDK_SHARE}/qi/module/${_modfilename}" "cpp\n")
  qi_install_data("${QI_SDK_DIR}/${QI_SDK_SHARE}/qi/module/${_modfilename}" SUBFOLDER "qi/module")
endfunction()
