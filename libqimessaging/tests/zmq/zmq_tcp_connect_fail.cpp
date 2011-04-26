#include <iostream>
#include <zmq.hpp>

const char* gAddress = "tcp://127.0.0.1:4242";


/*
 this should fail as soon as possible.

 the tricky part is to detect if the tcp connection failed or no. Tcp can tell if a server does not
 exist, but how could we get that information from zmq?
 */
int main(int argc, char *argv[])
{
  zmq::context_t ctx(1);
  {
    zmq::socket_t  cli(ctx, ZMQ_REQ);
    zmq::message_t msg;

    //async will not return
    cli.connect(gAddress);

    //we always want send and recv.
    //send will not fail because it's buffered, but recv on a non existent tcp connection should fail quickly.
    cli.send(msg);
    cli.recv(&msg);
  }

  //when the context is destroy, it wait on cli.send to be really done
}
