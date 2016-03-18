#pragma once

#ifndef _QI_SIGNALHELPER_HPP_
#define _QI_SIGNALHELPER_HPP_

#include <qi/actor.hpp>
#include <qi/anyobject.hpp>
#include <qi/clock.hpp>
#include <qi/signal.hpp>

namespace qi
{
/**
 * @brief A tool to track signal emissions, specialized for testing.
 * A signal spy can acknowledge every signal emission of a given signal, type-erased or not.
 * Every emission is recorded, so that they can be compared to expectations, or to produce a
 * history.
 *
 * It could also be used in production code for the timeout mechanism implemented in waitUntil.
 */
class SignalSpy: public qi::Actor
{
public:
  /// Constructor taking a signal instance.
  template<typename... Args>
  SignalSpy(SignalF<void(Args...)>& signal)
    : _records()
  {
    signal.connect(stranded([this](const Args&... args)
    {
      this->recordCallback(args...);
    }));
  }

  /// Constructor taking a type-erased signal.
  SignalSpy(qi::AnyObject& object, const std::string& signalOrPropertyName)
    : _records()
  {
    object.connect(
          signalOrPropertyName,
          qi::AnyFunction::fromDynamicFunction(
            stranded([this](qi::AnyReferenceVector anything)
    {
      return this->recordAnyCallback(anything);
    })));
  }

  // Non-copyable
  SignalSpy(const SignalSpy&) = delete;
  SignalSpy& operator=(const SignalSpy&) = delete;

  ~SignalSpy()
  {
    joinTasks();
  }

  /// A record data, corresponding to one signal emission.
  struct Record
  {
    /// Signal arguments are stored here, in a type-erased way for compatibility.
    std::vector<qi::AnyValue> args;

    /// Use this to access an argument in the type you expect it.
    template<typename T>
    const T& arg(int index) const
    {
      return args[index].asReference().as<T>();
    }
  };

  /// Retrieve all the records in one shot.
  std::vector<Record> allRecords() const
  {
    return async([this]
    { return _records;
    }).value();
  }

  /// Direct access to a record, by order of arrival.
  Record record(size_t index) const
  {
    qiLogDebug("qi.signalspy") << "Getting record #" << index << " "
                               << (strand()->isInThisContext() ? "from strand" : "from outside");

    return async([this, index]
    {
      qiLogDebug("qi.signalspy") << "Getting record #" << index;
      return _records[index];
    }).value();
  }

  /// Direct access to last record.
  Record lastRecord() const
  {
    return async([this]
    {
      assert(!_records.empty()); return _records.back();
    }).value();
  }

  /// The number of records.
  size_t recordCount() const
  {
    qiLogDebug("qi.signalspy") << "Getting record count "
                               << (strand()->isInThisContext() ? "from strand" : "from outside");
    return async([this]
    {
      qiLogDebug("qi.signalspy") << "Getting record count";
      return _records.size();
    }).value();
  }

  QI_API_DEPRECATED_MSG(Use 'recordCount' instead)
  unsigned int getCounter() const
  {
    return async([&]{ return static_cast<unsigned int>(_records.size()); }).value();
  }

  /// Waits for the given number of records to be reached, before the given timeout.
  qi::FutureSync<bool> waitUntil(unsigned int nofRecords, const qi::Duration& timeout) const
  {
    qi::Promise<bool> waiting;
    async([this, waiting, nofRecords, timeout]() mutable
    {
      if(nofRecords <= _records.size())
      {
        waiting.setValue(true);
        return;
      }

      qi::SignalLink recordedSubscription;

      // track timeout
      auto timingOut = asyncDelay([this, waiting, &recordedSubscription]() mutable
      {
        waiting.setValue(false);
        recorded.disconnect(recordedSubscription);
      }, timeout);

      // be called after signal emissions are recorded
      recordedSubscription = recorded.connect(stranded(
      [this, waiting, &recordedSubscription, timingOut, nofRecords]() mutable
      {
        if (nofRecords <= _records.size())
        {
          waiting.setValue(true);
          timingOut.cancel();
          recorded.disconnect(recordedSubscription);
        } // no need for scheduling in the strand because it is a direct connection
      })).setCallType(qi::MetaCallType_Direct);
    });
    return waiting.future();
  }

private:
  /// The signal records.
  std::vector<Record> _records;

  /// Emitted for internal synchronziation.
  mutable qi::Signal<void> recorded;

  /// Internal generic typed callback for signals.
  template <typename... Args>
  void recordCallback(const Args&... args)
  {
    assert(strand()->isInThisContext());
    _records.emplace_back(Record{{qi::AnyValue::from<Args>(args)...}});
    recorded();
  }

  /// Internal type-erased callback for type-erased signals.
  AnyReference recordAnyCallback(const qi::AnyReferenceVector& args)
  {
    assert(strand()->isInThisContext());
    Record record;
    for (const auto& arg: args)
      record.args.emplace_back(arg.to<qi::AnyValue>());
    _records.emplace_back(std::move(record));
    recorded();
    return AnyReference();
  }
};
}

#endif
