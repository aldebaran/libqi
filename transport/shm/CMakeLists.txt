
subdirs(client)
subdirs(memory)
subdirs(server)
subdirs(util)

set(headers
  transport_config.hpp
  shm.hpp
)

install_header(LIBIPPC SUBFOLDER alippc/transport/shm ${headers})

