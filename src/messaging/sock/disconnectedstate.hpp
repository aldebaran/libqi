#pragma once
#ifndef _QI_SOCK_DISCONNECTEDSTATE_HPP
#define _QI_SOCK_DISCONNECTEDSTATE_HPP

namespace qi
{
  namespace sock
  {
    /// Disconnected state of the socket.
    /// It does nothing but is provided for the sake of consistency.
    template<typename N>
    struct Disconnected
    {
    };
}} // namespace qi::sock

#endif // _QI_SOCK_DISCONNECTEDSTATE_HPP
