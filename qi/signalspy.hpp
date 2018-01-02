#pragma once

#ifndef _QI_SIGNALSPY_HPP_
#define _QI_SIGNALSPY_HPP_

#include <qi/actor.hpp>
#include <qi/anyobject.hpp>
#include <qi/api.hpp>
#include <qi/clock.hpp>
#include <qi/signal.hpp>
#include <ka/src.hpp>

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
class QI_API SignalSpy: public Actor
{
public:
  /// Constructor taking a signal instance.
  /// The signal instance must outlive the signal spy.
  template<typename... Args>
  SignalSpy(SignalF<void(Args...)>& signal)
    : _records()
  {
    SignalLink link = signal.connect(stranded([this](const Args&... args)
    {
      this->recordCallback(args...);
    }));
    _disconnect = [link, &signal]{ signal.disconnect(link); };
  }

  /// Constructor taking a type-erased signal.
  SignalSpy(AnyObject& object, const std::string& signalOrPropertyName);

  // Non-copyable
  SignalSpy(const SignalSpy&) = delete;
  SignalSpy& operator=(const SignalSpy&) = delete;

  ~SignalSpy();

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
  std::vector<Record> allRecords() const;

  /// Direct access to a record, by order of arrival.
  Record record(size_t index) const;

  /// Direct access to last record.
  Record lastRecord() const;

  /// The number of records.
  size_t recordCount() const;

  QI_API_DEPRECATED_MSG(Use 'recordCount' instead)
  unsigned int getCounter() const;

  /// Waits for the given number of records to be reached, before the given timeout.
  FutureSync<bool> waitUntil(size_t nofRecords, const Duration& timeout) const;

private:
  /// Disconnects from the signal.
  std::function<void()> _disconnect;

  /// The signal records.
  std::vector<Record> _records;

  /// Emitted for internal synchronization.
  mutable Signal<void> recorded;

  /// Internal generic typed callback for signals.
  template <typename... Args>
  void recordCallback(const Args&... args)
  {
    QI_ASSERT(strand()->isInThisContext());
    _records.emplace_back(Record{{AnyValue::from<Args>(args)...}});
    recorded();
  }

  /// Internal type-erased callback for type-erased signals.
  AnyReference recordAnyCallback(const AnyReferenceVector& args);
};
}

#endif // _QI_SIGNALSPY_HPP_
