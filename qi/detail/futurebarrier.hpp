#pragma once
/*
**  Copyright (C) 2015 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QI_DETAIL_FUTURE_BARRIER_HXX_
#define QI_DETAIL_FUTURE_BARRIER_HXX_

namespace qi
{

namespace detail
{

template<typename T>
class FutureBarrierPrivate {
public:
  /// FutureBarrier constructor taking no argument.
  FutureBarrierPrivate(FutureCallbackType async = FutureCallbackType_Async)
    : _closed(0)
    , _count(0)
    , _futures()
    , _promise(&qi::PromiseNoop<std::vector<Future<T> > >, async)
  {}

  void onFutureFinish() {
    if (--(this->_count) == 0 && this->_closed.load()) {
      if (!_set.swap(true))
        this->_promise.setValue(this->_futures);
    }
  }

  void cancelAll() {
    QI_ASSERT(_closed.load());
    for (typename std::vector< Future<T> >::iterator it = this->_futures.begin();
         it != this->_futures.end();
         ++it)
      it->cancel();
  }

  Atomic<bool> _closed;
  Atomic<bool> _set;
  Atomic<int> _count;
  std::vector< Future<T> > _futures;
  Promise< std::vector< Future<T> > > _promise;

};

}

/**
 * \class qi::FutureBarrier
 * \includename{qi/future.hpp}
 * \brief This class helps waiting on multiple futures at the same point.
 *
 * \verbatim
 * This class helps waiting on multiple futures at the same point. If you want
 * to make several calls in a function and wait for all results at some point.
 * (:cpp:func:`qi::waitForAll(std::vector<Future<T>>&)` and
 * :cpp:func:`qi::waitForFirst(std::vector<Future<T>>&)` may help you
 * for simple cases).
 *
 * :cpp:class:`qi::FutureBarrier` is used like a builder. You must give it the
 * futures with :cpp:func:`qi::FutureBarrier<T>::addFuture(qi::Future<T>)`. On
 * first call to :cpp:func:`qi::FutureBarrier<T>::future()`, barrier will be closed
 * and won't except any more future. :cpp:func:`qi::FutureBarrier<T>::future()`
 * returns the vector of all the futures given to the barrier.
 *
 * With this code, you can easily write asynchronous map code.
 *
 * Simple example: waitForAll
 * **************************
 *
 * .. code-block:: cpp
 *
 *     void waitForAll(std::vector< Future<int> >& vect) {
 *         qi::FutureBarrier<int> barrier;
 *         std::vector< Future<int> >::iterator it;
 *
 *         for (it = vect.begin(); it != vect.end(); ++it) {
 *             barrier.addFuture(*it);
 *         }
 *         barrier.future().wait();
 *
 *         // [1]: Do something here with all the results.
 *     }
 *
 * This function is the simplest one you can write with FutureBarrier. Lets say
 * you have a vector of calls and you eant to wait on all of them before
 * executing something, this is typically the kind of code you would write.
 *
 * .. note::
 *
 *     This function is already provided with the API in ``qi`` namespace,
 *     as a templated implementation. Don't recode it.
 *
 * Complete example
 * ****************
 *
 * .. code-block:: cpp
 *
 *     qi::Future<int> returnAsynchronouslyNumber(int number);
 *     void mult42(qi::Promise<int> prom, qi::Future<int> number);
 *     void sumList(qi::Promise<int> prom,
 *                  qi::Future< std::vector< qi::Future<int> > > fut);
 *
 *     qi::Future<int> sum42ProductTable() {
 *         qi::FutureBarrier barrier;
 *
 *         // [1]:
 *         for (int it = 0; it < 10; ++it) {
 *             // [1.1]:
 *             qi::Future<int> fut = returnAsynchronouslyNumber(it);
 *
 *             qi::Promise<int> prom;
 *             fut.connect(boost::bind(&mult42, prom, _1));
 *             barrier.addFuture(prom.future());
 *
 *             // [1.2]
 *         }
 *
 *         // The following line would hang until the results are ready:
 *         // std::vector< qi::Future<int> > values = barrier.future();
 *         // Vector would then contain promises results, when they are all
 *         // ready, so [0, 42, 84, 126, 168, 210, 252, 294, 336, 378]
 *
 *         // [2]:
 *         qi::Promise<int> res;
 *         barrier.future().connect(boost::bind(&sumList, res, _1));
 *         return res.future();
 *     }
 *
 * This is a complete example of how to do a map. This is the standart usage
 * of futures but within a loop. If you look at *[1.1]* part, you have an
 * asynchronous call to returnAsynchronouslyNumber function, a treatment of this
 * result with function *mult42* to which we give a promise and we use the future
 * of the promise. Instead of returning it, we give it to the FutureBarrier.
 *
 * This is due to the fact that *[2]* needs *[1]* to be completely executed
 * before executing, including the callback *mult42*. FutureBarrier makes sure of
 * this synchronisation.
 *
 * Since it is returning a :cpp:class:`qi::Future`. You can connect to it using
 * the standard pattern again and execute a callback (*sunList*) when all the
 * results has been acquired. This what *[2]* does.
 *
 * To summaries, this function will: use an asynchronous call to the function
 * identity (just to have an asynchronous call), multiply all the results with
 * the number 42, and the sum the complete vector, to return it.
 *
 * .. note::
 *
 *     If you add any callback to the future after the call to
 *     :cpp:func:`qi::FutureBarrier<T>::addFuture(qi::Future<T>)`,
 *     replacing *[1.2]*, the callback on barrier's future will be executed
 *     asynchronously with it. If you are not sure, always call
 *     :cpp:func:`qi::FutureBarrier<T>::addFuture(qi::Future<T>)` in last.
 * \endverbatim
 *
 * \deprecated since 2.4, use waitForAll
 */
template<typename T>
class FutureBarrier : boost::noncopyable {
public:
  /// FutureBarrier constructor taking no argument.
  FutureBarrier(FutureCallbackType async = FutureCallbackType_Async)
    : _p(boost::make_shared<detail::FutureBarrierPrivate<T> >(async))
  {
    _p->_promise.setOnCancel(qi::bindWithFallback(
          boost::function<void()>(),
          &detail::FutureBarrierPrivate<T>::cancelAll,
          boost::weak_ptr<detail::FutureBarrierPrivate<T> >(_p)));
  }

  /**
   * \brief Adds the future to the barrier.
   * \return Whether the future could be added.
   *
   * \verbatim
   * This adds the future to the barrier. It means barrier's future won't return
   * until this one returns. It will also be added to the resulting vector.
   *
   * When :cpp:func:`qi::FutureBarrier::future()` has been called, this function
   * will throw.
   * \endverbatim
   */
  void addFuture(qi::Future<T> fut) {
    // Can't add future from closed qi::FutureBarrier.
    if (_p->_closed.load())
      throw std::runtime_error("Adding future to closed barrier");

    ++(_p->_count);
    fut.connect(boost::bind<void>(&detail::FutureBarrierPrivate<T>::onFutureFinish, _p));
    _p->_futures.push_back(fut);
  }

  /**
   * \brief Gets the future result for the barrier.
   *
   * \verbatim
   * Returns a future containing the vector of all the futures given to the barrier.
   *
   * .. warning::
   *
   *     Once called, you will not be able to add a new future to the barrier.
   * \endverbatim
   */
  Future< std::vector< Future<T> > > future() {
    this->close();
    return _p->_promise.future();
  }

protected:
  boost::shared_ptr<detail::FutureBarrierPrivate<T> > _p;

private:
  void close() {
    _p->_closed = true;
    if (_p->_count.load() == 0) {
      if (!_p->_set.swap(true))
        _p->_promise.setValue(_p->_futures);
    }
  }
};

/**
 * \brief Helper function to wait on a vector of futures.
 * \param vect The vector of futures to wait on.
 *
 * \verbatim
 * This function will wait on all the futures of the given vector and return
 * when they have all been set, either with an error or a valid value.
 * \endverbatim
 */
template <typename T>
qi::FutureSync<std::vector<Future<T> > > waitForAll(std::vector<Future<T> >& vect) {
  typename std::vector< Future<T> >::iterator it;
  qi::FutureBarrier<T> barrier;

  for (it = vect.begin(); it != vect.end(); ++it) {
    barrier.addFuture(*it);
  }
  return barrier.future();
}

/**
 * \brief Helper function to wait for the first valid future.
 * \param vect The vector of futures to wait on.
 * \return The first valid future, or an error.
 *
 * \verbatim
 * This function will wait on all the futures of the vector. It returns the
 * first valid future that returns. If no future is valid, a future set with
 * an error is returned.
 * \endverbatim
 */
template <typename T>
qi::FutureSync< qi::Future<T> > waitForFirst(std::vector< Future<T> >& vect) {
  typename std::vector< Future<T> >::iterator it;
  qi::Promise< qi::Future<T> > prom;
  qi::Atomic<int>* count = new qi::Atomic<int>();
  count->swap((int)vect.size());
  for (it = vect.begin(); it != vect.end(); ++it) {
    it->connect(boost::bind<void>(&detail::waitForFirstHelper<T>, prom, *it, count));
  }
  return prom.future();
}

}

#endif
