
set(headers
  connection_handler.hpp
  shmserver.hpp
  boost_server_shared_segment_connector.hpp
  boost_server_shared_segment_connector.hxx
  server_shared_segment_connector.hpp
)

install_header(LIBIPPC SUBFOLDER alippc/transport/shm/server ${headers})

