/*
 * Author(s):
 * - Baptiste Marchand <bmarchand@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

#ifndef BINDER_SERVER_BACKEND_HPP
#define BINDER_SERVER_BACKEND_HPP

#include "src/transport/server_backend.hpp"

namespace qi {
namespace transport {
namespace detail {

class BinderServerBackend : public ServerBackend
{
public:
    BinderServerBackend(const std::vector<std::string> &serverAddresses);
    void run();
};

} // namespace detail
} // namespace transport
} // namespace qi



#endif // BINDER_SERVER_BACKEND_HPP
