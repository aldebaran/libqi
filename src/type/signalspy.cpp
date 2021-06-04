#include <qi/signalspy.hpp>
#include <qi/log.hpp>

qiLogCategory("qi.signalspy");

namespace qi
{
namespace detail
{
  struct WaitTracking
  {
    WaitTracking(qi::Signal<void>& signal) : trackedSignal(signal) {}
    qi::Signal<void>& trackedSignal;
    SignalLink signalLink = SignalBase::invalidSignalLink;
    Future<void> timingOut;
    Promise<bool> waitingPromise;

    Future<void> untrack()
    {
      QI_ASSERT(!waitingPromise.future().isRunning()); // just a security
      timingOut.cancel();

      const auto link = exchangeInvalidSignalLink(signalLink);
      return trackedSignal.disconnectAsync(link).andThen([](bool){});
    }
  };
} // detail

SignalSpy::SignalSpy(qi::AnyObject& object, const std::string& signalOrPropertyName)
  : _records()
{
  using namespace ka::functional_ops;
  SignalLink link = object.connect(
    signalOrPropertyName,
    qi::AnyFunction::fromDynamicFunction(
        SrcFuture{} * stranded([this](qi::AnyReferenceVector anything) {
          return this->recordAnyCallback(anything);
        })
    )).value();
  _disconnect = [link, object]{ object.disconnect(link).value(); };
}

SignalSpy::~SignalSpy()
{
  try
  {
    _disconnect();
  }
  catch (const std::exception& e)
  {
    qiLogDebug() << "Error while disconnecting from signal: " << e.what();
  }
  joinTasks();
}

std::vector<SignalSpy::Record> SignalSpy::allRecords() const
{
  return async([this]
  { return _records;
  }).value();
}

SignalSpy::Record SignalSpy::record(size_t index) const
{
  qiLogDebug() << "Getting record #" << index << " "
               << (strand()->isInThisContext() ? "from strand" : "from outside");

  return async([this, index]
  {
    if(index >= _records.size())
    {
      std::stringstream message;
      message << "index " << index << " is out of range";
      throw std::runtime_error(message.str());
    }
    return _records[index];
  }).value();
}

SignalSpy::Record SignalSpy::lastRecord() const
{
  return async([this]
  {
    assert(!_records.empty()); return _records.back();
  }).value();
}
size_t SignalSpy::recordCount() const
{
  qiLogDebug() << "Getting record count "
               << (strand()->isInThisContext() ? "from strand" : "from outside");
  return async([this]
  {
    qiLogDebug() << "Getting record count";
    return _records.size();
  }).value();
}

unsigned int SignalSpy::getCounter() const
{
  return async([&]{ return static_cast<unsigned int>(_records.size()); }).value();
}

FutureSync<bool> SignalSpy::waitUntil(size_t nofRecords, const qi::Duration& timeout) const
{
  return async([this, nofRecords, timeout]
  {
    // If the target is already reached, return immediately.
    if(nofRecords <= _records.size())
      return Future<bool>{true};

    // We will track the list of records.
    // The subscriber will be known to all callbacks to allow them to disconnect from the signal
    auto waitTracking = std::make_shared<detail::WaitTracking>(recorded);

    // The wait can be cancelled.
    waitTracking->waitingPromise = Promise<bool>{stranded([waitTracking](Promise<bool> p)
    {
      if (!p.future().isRunning())
        return;
      p.setCanceled();
      waitTracking->untrack();
    })};

    // Tracking the timeout.
    waitTracking->timingOut = asyncDelay([waitTracking]() mutable
    {
      auto& waiting = waitTracking->waitingPromise;
      if (!waiting.future().isRunning())
        return;

      waiting.setValue(false);
      waitTracking->untrack();
    }, timeout);

    // Tracking the records.
    // Since the record list is only edited from the strand, a direct signal connection
    // will keep us in the strand. So there is no need for stranding the callback.
    waitTracking->signalLink = recorded.connect([waitTracking, this, nofRecords]() mutable
    {
      auto& waiting = waitTracking->waitingPromise;
      if (!waiting.future().isRunning())
        return;

      if (nofRecords <= _records.size())
      {
        waiting.setValue(true);
        waitTracking->untrack();
      }
    }).setCallType(qi::MetaCallType_Direct);

    return waitTracking->waitingPromise.future();
  }).unwrap();
}

AnyReference SignalSpy::recordAnyCallback(const qi::AnyReferenceVector& args)
{
  QI_ASSERT(strand()->isInThisContext());
  Record record;
  for (const auto& arg: args)
    record.args.emplace_back(arg.to<qi::AnyValue>());
  _records.emplace_back(std::move(record));
  recorded();
  return AnyReference();
}
} // qi
