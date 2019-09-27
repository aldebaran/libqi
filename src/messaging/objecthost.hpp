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

  /// This class is responsible for handling the ownership of bound objects
  /// created in the context of a socket.
  //
  // Warning:
  // There is a circular dependency between `ObjectHost` and `BoundObject`.
  //
  // When a `BoundObject` object receives a `terminate` message, it tries to
  // remove itself from its owner (an `ObjectHost` object). This sequence
  // locks, in order, a mutex in the `BoundObject` that protects its internal
  // data and then a mutex in `ObjectHost` that protects the object/socket
  // bindings.
  //
  // So the thread T1 that executes this sequence locks two mutexes M1 then
  // M2.
  //
  // When a socket disconnects from a libqi TCP server, the `Server`
  // object tries to remove all bound objects related to this socket from the
  // service bound objects it contains. This sequence locks in order the mutex
  // in each of service bound objects as `ObjectHost` that protects the
  // object/socket bindings and then a mutex in each of the `BoundObject`
  // objects that protects its internal data.
  //
  // The thread T2 that executes this sequence locks two mutexes M2 then
  // M1.
  //
  // ╔════╗         ╔════╗
  // ║ T1 ║         ║ T2 ║
  // ╚════╝         ╚════╝
  //   ▼              ▼
  //   |              |
  //   locks M1       |
  //   ▼              locks M2
  //   |              ▼
  //   blocks on M2   |
  //   ▼              blocks on M1
  //   |              ▼
  // threads block one another = deadlock
  //
  // Obviously this can cause deadlocks if operations are not executed
  // carefully and if mutexes are not released early enough. Therefore we need
  // to be careful when modifying methods of this class.
  class ObjectHost
  {
  public:
    ObjectHost(unsigned int service);
    virtual ~ObjectHost();

    /// @pre `obj != nullptr && socket != nullptr`
    /// @throws A `std::logic_error` if an object with the same ID was already added on this host.
    // TODO: Throw an domain specific error.
    unsigned int addObject(BoundObjectPtr obj, MessageSocketPtr socket);

    /// Removes the object with the given id.
    ///
    /// @returns True if the object with this id was removed, false otherwise.
    /// @invariant `!(h.removeObject(id) && h.removeObject(id))`
    bool removeObject(unsigned int id) noexcept;

    /// Removes all objects bound to the given socket.
    ///
    /// @returns The number of objects removed.
    /// @invariant `forall s, o: noexcept(h.removeObjectsFromSocket(s), h.addObject(o, s))`
    std::size_t removeObjectsFromSocket(const MessageSocketPtr& socket) noexcept;

    /// @invariant `ObjectHost(id).service() == id`
    unsigned int service() const { return _service; }

    virtual unsigned int nextId() = 0;

  protected:
    /// Removes all objects.
    ///
    /// @returns The number of objects removed.
    /// @post `removeObject(_) == false`
    std::size_t clear() noexcept;

  private:
    const unsigned int _service;
    using ObjSocketBindingList = std::vector<detail::boundObject::SocketBinding>;
    boost::synchronized_value<ObjSocketBindingList> _objSockBindings;
  };
}

#endif  // _SRC_OBJECTHOST_HPP_
