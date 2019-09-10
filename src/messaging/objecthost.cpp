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

  namespace
  {

    /// ForwardIterator FwdIter
    template<typename FwdIter>
    std::vector<BoundObjectPtr> consumeObjectFromBindings(FwdIter begin, FwdIter end)
    {
      std::vector<BoundObjectPtr> objects;
      objects.reserve(static_cast<std::size_t>(distance(begin, end)));

      // Transform the range of bindings into a range of objects so that we can defer destruct all of
      // them.
      std::transform(std::make_move_iterator(begin), std::make_move_iterator(end),
                     std::back_inserter(objects),
                     [](detail::boundObject::SocketBinding binding) -> BoundObjectPtr {
                       return binding.object();
                     });
      return objects;
    }

  }

  std::size_t ObjectHost::removeObjectsFromSocket(const MessageSocketPtr& socket) noexcept
  {
    std::vector<BoundObjectPtr> objects;

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

      objects = consumeObjectFromBindings(bindingsWithSocketIt, bindingsEnd);
      syncObjSockBindings->erase(bindingsWithSocketIt, bindingsEnd);
    }

    // Defer destruction outside of the lock of the list to avoid potential deadlocks.
    const auto count = objects.size();
    sequentializeDeferDestruction(std::move(objects));
    return count;
  }

  bool ObjectHost::removeObject(unsigned int id) noexcept
  {
    BoundObjectPtr object;

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

      auto binding = std::move(*it);
      syncObjSockBindings->erase(it);
      qiLogDebug() << this << " Object " << id << " removed.";
      object = binding.object();
    }

    // Defer destruction outside of the lock of the list to avoid potential deadlocks.
    BoundObject::deferDestruction(std::move(object));
    return true;
  }

  void ObjectHost::clear() noexcept
  {
    ObjSocketBindingList bindings;
    _objSockBindings.swap(bindings);

    sequentializeDeferDestruction(consumeObjectFromBindings(bindings.begin(), bindings.end()));
  }

  template<typename Range>
  Future<void> ObjectHost::sequentializeDeferDestruction(Range objects)
  {
    // We use future continuations to implement chaining/sequencing.
    auto seqFut = futurize();

    for (BoundObjectPtr& object : std::move(objects))
    {
      seqFut = deferConsumeWhenReady<BoundObjectPtr>(
        std::move(object), [&](Future<void> ready, PtrHolder<BoundObjectPtr> holder) {
          return seqFut.then(FutureCallbackType_Async,
                  [=](Future<void>) mutable {
                    return ready.andThen([=](void*) {
                      const auto res = consumePtr(holder, &BoundObject::deferDestruction);
                      return res.empty() ? futurize() : *res;
                    }).unwrap();
                  }).unwrap();
        });
    }
    return seqFut;
  }
}

