
set(headers
  shmclient.hpp
  shmconnection.hpp
  result_handler.hpp
  result_info.hpp
  result_info.hxx
  process_shared_segment_connector.hpp
  boost_process_shared_segment_connector.hpp
)

install_header(LIBIPPC SUBFOLDER qi/transport/shm/client ${headers})

