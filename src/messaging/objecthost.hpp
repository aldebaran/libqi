#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_OBJECTHOST_HPP_
#define _SRC_OBJECTHOST_HPP_

#include <map>
#include <mutex>
#include <memory>

#include <qi/atomic.hpp>
#include <qi/type/fwd.hpp>

#include "messagesocket.hpp"


namespace qi
{
  class Message;
  class BoundObject;
  using BoundObjectPtr = boost::shared_ptr<BoundObject>;

  namespace detail
  {
    namespace boundObject
    {
      class SocketBinding;
    }
  }

  /// This class is responsible for handling the ownership of bound objects created in the context
  /// of a socket.
  class ObjectHost
  {
  public:
    ObjectHost(unsigned int service);
    virtual ~ObjectHost();

    /// @throws A `std::logic_error` if an object with the same ID was already added on this host.
    // TODO: Throw an domain specific error.
    unsigned int addObject(BoundObjectPtr obj, MessageSocketPtr socket);

    /// @returns True if the object with this id was removed, false otherwise.
    /// @invariant `!(h.removeObject(id) && h.removeObject(id))`
    bool removeObject(unsigned int id) noexcept;

    /// @returns The number of objects removed.
    /// @invariant `forall s, o: noexcept(h.removeObjectsFromSocket(s), h.addObject(o, s))`
    std::size_t removeObjectsFromSocket(const MessageSocketPtr& socket) noexcept;

    /// @invariant `ObjectHost(id).service() == id`
    unsigned int service() const { return _service; }

    virtual unsigned int nextId() = 0;

  protected:
    /// Removes all objects.
    /// @post `removeObject(_) == false`
    void clear() noexcept;

  private:
    const unsigned int _service;
    using ObjSocketBindingList = std::vector<detail::boundObject::SocketBinding>;
    boost::synchronized_value<ObjSocketBindingList> _objSockBindings;
  };
}

#endif  // _SRC_OBJECTHOST_HPP_
