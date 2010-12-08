/*
** Author(s):
**  - Chris Kilner <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/transport_context.hpp>
#include <qi/transport/src/zmq/zmq_forwarder.hpp>

int main(int argc, char **argv)
{
  zmq::context_t ctx(1);
  qi::transport::detail::ZMQForwarder f(ctx);
  f.run("tcp://127.0.0.1:6000", "tcp://127.0.0.1:6001");
  return 0;
}
