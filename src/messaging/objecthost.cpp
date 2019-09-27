/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/actor.hpp>
#include "objecthost.hpp"

#include "boundobject.hpp"
#include <ka/typetraits.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/transformed.hpp>

qiLogCategory("qimessaging.objecthost");

namespace qi
{

  ObjectHost::ObjectHost(unsigned int service)
    : _service(service)
  {
  }

  ObjectHost::~ObjectHost()
  {
    clear();
  }

  unsigned int ObjectHost::addObject(BoundObjectPtr obj, MessageSocketPtr socket)
  {
    QI_ASSERT_NOT_NULL(obj);
    QI_ASSERT_NOT_NULL(socket);

    const auto id = obj->id();
    {
      auto syncObjSockBindings = _objSockBindings.synchronize();
      const auto objIsAlreadyAdded =
        std::any_of(syncObjSockBindings->begin(), syncObjSockBindings->end(),
                    [&](const detail::boundObject::SocketBinding& binding) {
                      return binding.object()->id() == id;
                    });
      if (objIsAlreadyAdded)
        // TODO: Throw an domain specific error.
        throw std::logic_error("This BoundObject already exists in this host.");
    }

    // Create the value outside of the lock of the list to avoid potential deadlocks.
    detail::boundObject::SocketBinding binding(std::move(obj), std::move(socket));
    _objSockBindings->emplace_back(std::move(binding));

    return id;
  }

  std::size_t ObjectHost::removeObjectsFromSocket(const MessageSocketPtr& socket) noexcept
  {
    // Destroy the socket bindings outside of the lock.
    ObjSocketBindingList bindings;

    { // Lock all bindings just to move the ones from the socket into the local storage.
      // Do not destroy a binding while locking this object bindings storage, as it can induce a deadlock if
      // the bound object of the binding locks a mutex while waiting for this object bindings to unlock.
      auto syncObjSockBindings = _objSockBindings.synchronize();
      const auto syncBindingsEnd = syncObjSockBindings->end();

      // Put all bindings related to the socket at the end of the vector. Do not use `std::remove_if`
      // because values that are removed are move-assigned into, effectively resulting in their immediate
      // destruction preventing us to defer the destruction of the bound objects.
      const auto bindingsWithSocketIt =
        std::partition(syncObjSockBindings->begin(), syncBindingsEnd,
                       [&](const detail::boundObject::SocketBinding& binding) {
                         return binding.socket() != socket;
                       });

      // Early return if no object exists for this socket to avoid unnecessary operations.
      if (bindingsWithSocketIt == syncBindingsEnd)
        return {};

      // Move the socket bindings into the local storage.
      bindings.reserve(static_cast<std::size_t>(std::distance(bindingsWithSocketIt, syncBindingsEnd)));
      std::move(bindingsWithSocketIt, syncBindingsEnd, std::back_inserter(bindings));
      syncObjSockBindings->erase(bindingsWithSocketIt, syncBindingsEnd);
    }
    return bindings.size();
  }

  bool ObjectHost::removeObject(unsigned int id) noexcept
  {
    // Destroy the socket bindings outside of the lock.
    detail::boundObject::SocketBinding binding;

    { // Lock all bindings just to move the ones for this object into the local object.
      // Do not destroy a binding while locking this object bindings storage, as it can induce a deadlock if
      // the bound object of the binding locks a mutex while waiting for this object bindings to unlock.
      auto syncObjSockBindings = _objSockBindings.synchronize();
      const auto bindingsEnd = syncObjSockBindings->end();
      const auto it = std::find_if(syncObjSockBindings->begin(), bindingsEnd,
                                   [&](const detail::boundObject::SocketBinding& binding) {
                                     return binding.object()->id() == id;
                                   });
      if (it == bindingsEnd)
      {
        qiLogDebug() << this << " No match in host for " << id;
        return false;
      }

      // Move the socket binding into the local object.
      binding = std::move(*it);
      syncObjSockBindings->erase(it);
      qiLogDebug() << this << " Object " << id << " removed.";
    }

    return true;
  }

  std::size_t ObjectHost::clear() noexcept
  {
    // Destroy the socket bindings outside of the lock of this object storage.
    ObjSocketBindingList bindings;
    _objSockBindings.swap(bindings);
    return bindings.size();
  }
}

