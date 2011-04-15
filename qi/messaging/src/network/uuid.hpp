#pragma once
#ifndef _QI_MESSAGING_SRC_NETWORK_UUID_HPP_
#define _QI_MESSAGING_SRC_NETWORK_UUID_HPP_


namespace qi {
  namespace detail {
    // FIXME: UUID are usually stored in 16 bytes.
    // So it's a bit silly to use strings everywhere.
    std::string getUUID();
  }
}

#endif  // _QI_MESSAGING_SRC_NETWORK_UUID_HPP_
