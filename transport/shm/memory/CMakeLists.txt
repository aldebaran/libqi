
set(headers
  mapped_device.hpp
  mapped_segment_selector.hpp
  mapped_shared_segment.hpp
  process_shared_segment.hpp
  process_shared_segment.hxx
)

install_header(LIBIPPC SUBFOLDER alippc/transport/shm/memory ${headers})

