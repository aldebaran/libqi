
set(headers
  serialization.h
  serialization_type.h
  boost_serializer.h
  call_definition.hpp
  call_definition.hxx
  result_definition.hpp
  result_definition.hxx
  definition_type.hpp
)

install_header(LIBIPPC SUBFOLDER alcommon-ng/serialization ${headers})

