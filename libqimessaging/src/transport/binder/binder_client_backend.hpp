/*
 * Author(s):
 * - Baptiste Marchand <bmarchand@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

#ifndef BINDER_CLIENT_BACKEND_HPP
#define BINDER_CLIENT_BACKEND_HPP

#include "src/transport/client_backend.hpp"

namespace qi {
namespace transport {
namespace detail {

class BinderClientBackend : public ClientBackend
{
public:

  explicit BinderClientBackend(const std::string &servername);

  /**
   * Sends data
   * \param tosend The data to send
   * \param result [in,out] The result
   */
  virtual void send(const std::string &tosend, std::string &result);
};

} // namespace detail
} // namespace transport
} // namespace qi


#endif // BINDER_CLIENT_BACKEND_HPP
