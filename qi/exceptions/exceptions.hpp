/*
* exceptions.hpp
*
*  Created on: Oct 1, 2009 at 10:47:42 AM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef QI_TRANSPORT_EXCEPTIONS_HPP_
#define QI_TRANSPORT_EXCEPTIONS_HPP_

#include <exception>
#include <string>

namespace qi {
  namespace transport {

    /// <summary>
    /// A basic exception class, base to other ippc:: exceptions.
    /// </summary>
    class Exception : public std::exception {
    public:
      Exception () {}
      Exception (const std::string & message) { this->message = message; }
      virtual ~Exception () throw () {}

      virtual const char * what () const throw () { return message.c_str(); }

    private:
      std::string message;
    };

    /// <summary>
    /// Exception thrown when creating/opening shared memory segments.
    /// </summary>
    class SharedSegmentInitializationException : public Exception {
    public:
      SharedSegmentInitializationException () {}
      SharedSegmentInitializationException (const std::string & message) : Exception(message) {}
      virtual ~SharedSegmentInitializationException () throw () {}
    };

    /// <summary>
    /// Exception thrown when the connection to the server fail.
    /// </summary>
    class ConnectionException : public Exception {
    public:
      ConnectionException () {}
      ConnectionException (const std::string & message) : Exception(message) {}
      virtual ~ConnectionException () throw () {}
    };

    /// <summary>
    /// Exception thrown when the server encountered a problem.
    /// </summary>
    class ServerException : public Exception {
    public:
      ServerException () {}
      ServerException (const std::string & message) : Exception(message) {}
      virtual ~ServerException () throw () {}
    };

    /// <summary>
    ///Exception thrown when the server encountered a problem when reading.
    /// </summary>
    class ReadException : public Exception {
    public:
      ReadException () {}
      ReadException (const std::string & message) : Exception(message) {}
      virtual ~ReadException () throw () {}
    };

    /// <summary>
    /// Exception thrown when the client encountered a problem when writing.
    /// </summary>
    class WriteException : public Exception {
    public:
      WriteException () {}
      WriteException (const std::string & message) : Exception(message) {}
      virtual ~WriteException () throw () {}
    };

    /// <summary>
    /// Exception thrown when the client can't find a service
    /// </summary>
    class ServiceNotFoundException : public Exception {
    public:
      ServiceNotFoundException () {}
      ServiceNotFoundException (const std::string & message) : Exception(message) {}
      virtual ~ServiceNotFoundException () throw () {}
    };


  }
}

#endif  // QI_TRANSPORT_EXCEPTIONS_HPP_
