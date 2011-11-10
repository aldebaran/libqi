/*
 * Author(s):
 * - Baptiste Marchand <bmarchand@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

#include <iostream>
#include "binder_client_backend.hpp"
#include "binder.h"

namespace qi {
namespace transport {
namespace detail {

BinderClientBackend::BinderClientBackend(const std::string &serverAddress)
  : ClientBackend(serverAddress)
{}

void BinderClientBackend::send(const std::string &tosend, std::string &result)
{
  binder_state *bs     = binder_open(1024*1024);
  void         *svcmgr = BINDER_SERVICE_MANAGER;
  unsigned     iodata[512/4];
  binder_io    msg, reply;

  bio_init(&msg, iodata, sizeof(iodata), 4);
  bio_put_string16_x(&msg, tosend.c_str());

  if (binder_call(bs, &msg, &reply, svcmgr, 0))
    std::cerr << "Could not send message: binder_call failed\n" << std::endl;

  unsigned len;
  uint16_t *s = bio_get_string16(&reply, &len);

  for (; *s; s++)
    result += *s;

  binder_done(bs, &msg, &reply);
}

} // namespace detail
} // namespace transport
} // namespace qi

