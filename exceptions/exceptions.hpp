/*
 * exceptions.hpp
 *
 *  Created on: Oct 1, 2009 at 10:47:42 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_EXCEPTIONS_HPP_
#define LIBIPPC_EXCEPTIONS_HPP_

#include <exception>
#include <string>

namespace AL {
  namespace Messaging {

/**
 * @brief A basic exception class, base to other ippc:: exceptions.
 */
class Exception : public std::exception {
public:
  Exception () {}
  Exception (const std::string & message) { this->message = message; }
  virtual ~Exception () throw () {}

  virtual const char * what () const throw () { return message.c_str(); }

private:
  std::string message;
};

/**
 * @brief Exception thrown when creating/opening shared memory segments.
 */
class SharedSegmentInitializationException : public Exception {
public:
  SharedSegmentInitializationException () {}
  SharedSegmentInitializationException (const std::string & message) : Exception(message) {}
  virtual ~SharedSegmentInitializationException () throw () {}
};

/**
 * @brief Exception thrown when the connection to the server fail.
 */
class ConnectionException : public Exception {
public:
  ConnectionException () {}
  ConnectionException (const std::string & message) : Exception(message) {}
  virtual ~ConnectionException () throw () {}
};

/**
 * @brief Exception thrown when the server encountered a problem.
 */
class ServerException : public Exception {
public:
  ServerException () {}
  ServerException (const std::string & message) : Exception(message) {}
  virtual ~ServerException () throw () {}
};

/**
 * @brief Exception thrown when the server encountered a problem when reading.
 */
class ReadException : public Exception {
public:
  ReadException () {}
  ReadException (const std::string & message) : Exception(message) {}
  virtual ~ReadException () throw () {}
};

/**
 * @brief Exception thrown when the client encountered a problem when writing.
 */
class WriteException : public Exception {
public:
  WriteException () {}
  WriteException (const std::string & message) : Exception(message) {}
  virtual ~WriteException () throw () {}
};


  }
}

#endif /* !LIBIPPC_EXCEPTIONS_HPP_ */
