
set(headers
  serialization.h
  call_definition.hpp
  call_definition.hxx
  result_definition.hpp
  result_definition.hxx
  definition_type.hpp
)

install_header(LIBIPPC SUBFOLDER alcommon-ng/serialization ${headers})
subdirs(boost)
