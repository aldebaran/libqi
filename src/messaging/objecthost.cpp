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
    auto syncObjSockBindings = _objSockBindings.synchronize();
    const auto bindingsEnd = syncObjSockBindings->end();

    // Put all bindings related to the socket at the end of the vector. Do not use `std::remove_if`
    // because values to remove are move-assigned into, effectively resulting in their immediate
    // destruction preventing us to defer the destruction of the bound objects.
    const auto bindingsWithSocketIt =
      std::partition(syncObjSockBindings->begin(), bindingsEnd,
                     [&](const detail::boundObject::SocketBinding& binding) {
                       return binding.socket() != socket;
                     });

    const auto count = std::distance(bindingsWithSocketIt, bindingsEnd);
    syncObjSockBindings->erase(bindingsWithSocketIt, bindingsEnd);
    return count;
  }

  bool ObjectHost::removeObject(unsigned int id) noexcept
  {
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

    syncObjSockBindings->erase(it);
    qiLogDebug() << this << " Object " << id << " removed.";

    return true;
  }

  void ObjectHost::clear() noexcept
  {
    ObjSocketBindingList bindings;
    _objSockBindings.swap(bindings);
  }
}

