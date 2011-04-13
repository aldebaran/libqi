/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/qi.h>
#include <qi/messaging.hpp>
#include <qi/messaging/src/client_impl.hpp>

qi_context_t *qi_context_create() {
  qi::Context *pctx = new qi::Context();
  return (qi_context_t *)pctx;
}

void qi_context_destroy(qi_context_t *ctx) {
  if (!ctx)
    return;
  qi::Context *pctx = (qi::Context *)ctx;
  delete pctx;
}
