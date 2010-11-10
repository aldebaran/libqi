
set(headers
  print_visitor.hpp
  equality_visitor.hpp
  serializable_visitor.hpp
  variables_list.hpp
)

install_header(LIBIPPC SUBFOLDER qi/collections/ ${headers})
