#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/container/flat_map.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <qi/future.hpp>
#include <qi/trackable.hpp>

namespace qi
{
  /** Cancel a group of unfinished registered futures on destruction.
   *  Guarantees that the registered set of futures will be canceled
   *  whatever the reason of the destruction of the group.
   *  @remark All public member functions are thread-safe unless specified.
   *
   *  @includename{qi/futuregroup.hpp}
   */
  class ScopedFutureGroup
    : boost::noncopyable
    , public qi::Trackable<ScopedFutureGroup>
  {
  public:
    /** Destructor, cancel all unfinished futures registered.
     */
    ~ScopedFutureGroup()
    {
      destroy();
      cancelAll();
    }

    /** Register a future to be canceled if not finished when this object is destroyed,
     *  or if cancelAll() is called.
     *  Futures finishing before cancelation will be automatically unregistered.
     *  Non-cancelable futures will be ignored.
     *  @param future Future to register.
     */
    template< class T >
    void add(Future<T> future)
    {
      _futureCancelList->emplace(future.uniqueId(), [future]() mutable { future.cancel(); });
      future.then(qi::track( [this](Future<T> f){ onFutureFinished(f); }, this ));
    }

    /** Cancel all registered futures and unregister them.
     */
    void cancelAll()
    {
      FutureCancelList cancelList;
      {
        auto synched_futureCancelList = _futureCancelList.synchronize();
        swap(cancelList, *synched_futureCancelList);
      }
      for (auto& slot : cancelList)
      {
        try
        {
          slot.second();
        }
        catch (std::exception& ex)
        {
          qiLogWarning("qi.scopedfuturegroup") << "Failed to cancel scoped future: " << ex.what();
        }
        catch (...)
        {
          qiLogWarning("qi.scopedfuturegroup") << "Failed to cancel scoped future: unknown error.";
        }

      }
    }

    /** @return True if there is no future registered, false otherwise. */
    bool empty() const
    {
      return _futureCancelList->empty();
    }

    /** @return Count of registered futures. */
    size_t size() const
    {
      return _futureCancelList->size();
    }

  private:

    using FutureCancelList = boost::container::flat_map< FutureUniqueId, boost::function<void()>>;
    boost::synchronized_value<FutureCancelList> _futureCancelList;

    template<class T>
    void onFutureFinished(Future<T> future)
    {
      _futureCancelList->erase(future.uniqueId());
    }
  };
}
