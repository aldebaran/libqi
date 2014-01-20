# To be run by cmake -P

function(apply_replaces result_name)
  # Damm, we have no access to cmake_parse_arguments
  #cmake_parse_arguments(ARG "" "" "KEYS;DATA")
  set(in "${ARG_DATA}")
  FOREACH(key ${ARG_KEYS})
    string(REPLACE "// Implementation of ${key}"
      "${${key}}"
      out
      "${in}"
    )
    set(in "${out}")
    string(LENGTH "${in}" fclength)
    message("File replace: ${fclength}")
  ENDFOREACH(key)
  set(${result_name} "${in}" PARENT_SCOPE)
endfunction()

file(READ ${input} _fcontent)
string(LENGTH "${_fcontent}" fclength)
message("File read: ${fclength}")

set(setTarget "return true;")
set(selectTask "return qi::Object<animals::CatAction>();")
set(canPerform "return true;")
set(expectedResult "return std::vector<float>();")

set(ARG_KEYS "setTarget;selectTask;canPerform;expectedResult")
set(ARG_DATA ${_fcontent})

apply_replaces(outdata) #  KEYS "setTarget;selectTask;canPerform" DATA ${_fcontent})
set(outdata ${outdata}
  "\n#include<qitype/objectfactory.hpp>\n"
  "QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR_FOR(animals::Cat, Cat);\n"
  )
file(WRITE ${output} "${outdata}")
