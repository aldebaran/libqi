/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_PUBLISHER_HPP__
#define   __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_PUBLISHER_HPP__

# include <qi/transport/publisher.hpp>
# include <zmq.hpp>
# include <boost/shared_ptr.hpp>

namespace qi {
  namespace transport {

    class ZMQPublisher : public Publisher {
    public:
      /// <summary>
      /// Creates a ZMQPublisher
      /// </summary>
      /// <param name="publisherAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQPublisher(const std::string &publisherAddress);

    /// <summary> Constructor. </summary>
    /// <param name="context"> An existing zmq context </param>
    /// <param name="publishAddress"> The server address. </param>
      ZMQPublisher(boost::shared_ptr<zmq::context_t> context, const std::string &publishAddress);

      boost::shared_ptr<zmq::context_t> getContext() const;

      virtual ~ZMQPublisher();

      virtual void publish(const std::string &tosend);

    protected:
      void bind();

    protected:
      boost::shared_ptr<zmq::context_t> _context;
      zmq::socket_t   _socket;
    };
  }
}

#endif // __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_PUBLISHER_HPP__
