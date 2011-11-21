/*
 * Author(s):
 * - Baptiste Marchand <bmarchand@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

#include <iostream>
#include <cerrno>
#include <cstring>

#include <qimessaging/exceptions.hpp>
#include "binder_server_backend.hpp"
#include "binder.h"

namespace qi {
namespace transport {
namespace detail {

BinderServerBackend::BinderServerBackend(const std::vector<std::string> &serverAddresses)
  : ServerBackend(serverAddresses)
{}

int svcmgr_handler(struct binder_state *bs,
                   struct binder_txn *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
  uint16_t *s;
  unsigned len;

  s = bio_get_string16(msg, &len);
  std::cout << "Received: ";
  for (; *s; s++)
    std::cout << *s;
  std::cout << std::endl;

  bio_put_string16_x(reply, "I HAVE DONE THIS!");

  if (binder_call(bs, msg, reply, BINDER_SERVICE_MANAGER, 0))
    std::cerr << "Could not send reply: binder_call failed\n" << std::endl;

  return 0;
}

void BinderServerBackend::run()
{
  struct binder_state *bs = binder_open(1024*1024);

  if (binder_become_context_manager(bs))
    throw qi::transport::Exception("cannot become context manager: "
                                   + std::string(strerror(errno)));

  binder_loop(bs, svcmgr_handler);
}


} // namespace detail
} // namespace transport
} // namespace qi
