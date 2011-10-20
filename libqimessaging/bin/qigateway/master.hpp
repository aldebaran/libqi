#ifndef QIMESSAGING_BIN_QIGATEWAY_MASTER_HPP
#define QIMESSAGING_BIN_QIGATEWAY_MASTER_HPP

#include <iostream>
#include <qi/macro.hpp>

namespace qi {
namespace gateway {

class Master
{
  QI_DISALLOW_COPY_AND_ASSIGN(Master);

public:
  static void launch(const std::string& address);

private:
};

} // namespace gateway
} // namespace qi

#endif // QIMESSAGING_BIN_QIGATEWAY_MASTER_HPP
