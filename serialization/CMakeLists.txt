
set(headers
  serialization.h
  call_definition.hpp
  call_definition.hxx
  result_definition.hpp
  result_definition.hxx
  iarchive.hpp
  oarchive.hpp
  serialization.hpp
  serialization.hxx
  definition_type.hpp
)

install_header(LIBIPPC SUBFOLDER alippc/serialization ${headers})

