
set(headers
  print_visitor.hpp
  alvalue_convertor.hpp
  serializable_visitor.hpp
  variables_list.hpp
)

install_header(LIBIPPC SUBFOLDER alcommon-ng/collections/ ${headers})
