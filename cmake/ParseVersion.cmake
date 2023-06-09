# Parses a version number.
#
# The input version number must match the format
# `MAJOR(.MINOR(.PATCH(.TWEAK)?)?)?(SUFFIXES)` where:
# - MAJOR, MINOR, PATCH, TWEAK = [0-9]+
# - SUFFIXES = .*
#
# It defines a number of variables in the parent scope:
#
# <prefix>_VERSION_FULL
# <prefix>_VERSION
# <prefix>_VERSION_MAJOR
# <prefix>_VERSION_MINOR
# <prefix>_VERSION_PATCH
# <prefix>_VERSION_TWEAK
# <prefix>_VERSION_SUFFIXES
function(parse_version prefix version)
  string(
    REGEX MATCH
    [[^(([0-9]+)(\.([0-9]+)(\.([0-9]+)(\.([0-9]+))?)?)?)(.*)]]
    _unused
    "${version}"
  )
  set("${prefix}_VERSION_FULL"     "${version}" PARENT_SCOPE)
  set("${prefix}_VERSION"          "${CMAKE_MATCH_1}" PARENT_SCOPE)
  set("${prefix}_VERSION_MAJOR"    "${CMAKE_MATCH_2}" PARENT_SCOPE)
  set("${prefix}_VERSION_MINOR"    "${CMAKE_MATCH_4}" PARENT_SCOPE)
  set("${prefix}_VERSION_PATCH"    "${CMAKE_MATCH_6}" PARENT_SCOPE)
  set("${prefix}_VERSION_TWEAK"    "${CMAKE_MATCH_8}" PARENT_SCOPE)
  set("${prefix}_VERSION_SUFFIXES" "${CMAKE_MATCH_9}" PARENT_SCOPE)
endfunction()
