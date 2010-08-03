
set(headers
  common.hpp
  handlers_pool.hpp
  server_command_delegate.hpp
  server_response_delegate.hpp
  threadable.hpp
  runnable.hpp
)

install_header(LIBIPPC SUBFOLDER alcommon-ng/transport/common ${headers})

