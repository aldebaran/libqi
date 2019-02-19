#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_EVENTLOOP_P_HPP_
#define _SRC_EVENTLOOP_P_HPP_

#include <atomic>
#include <thread>
#include <boost/asio.hpp>
#include <qi/api.hpp>
#include <ka/ark/mutable.hpp>
#include <ka/macroregular.hpp>
#include <qi/eventloop.hpp>
#include <boost/thread/synchronized_value.hpp>

namespace qi {
  class AsyncCallHandlePrivate
  {
  public:
    AsyncCallHandlePrivate()
      : canceled(false)
      , sd(NULL)
    { }

    void cancel() { canceled = true;}
    bool canceled;
    // Callback used with notifyFd.
#ifdef _WIN32
    // Assume it is a socket under WIN32, no way to poll
    // a generic descriptor.
    using Stream = boost::asio::ip::tcp::socket;
#else
    using Stream = boost::asio::posix::stream_descriptor;
#endif
    Stream* sd;
  };

  class QI_API_TESTONLY EventLoopPrivate
  {
  public:
    EventLoopPrivate(std::string name) : _name(std::move(name)) {}
    virtual ~EventLoopPrivate() = default;

    virtual bool isInThisContext() const =0;
    virtual void start(int nthreads)=0; // 0=auto
    virtual void join()=0;
    virtual void stop()=0;
    virtual qi::Future<void> asyncCall(qi::Duration delay, boost::function<void ()> callback, ExecutionOptions options = defaultExecutionOptions())=0;
    virtual void post(qi::Duration delay, const boost::function<void ()>& callback, ExecutionOptions options = defaultExecutionOptions())=0;
    virtual qi::Future<void> asyncCall(qi::SteadyClockTimePoint timepoint, boost::function<void ()> callback, ExecutionOptions options = defaultExecutionOptions())=0;
    virtual void post(qi::SteadyClockTimePoint timepoint, const boost::function<void ()>& callback, ExecutionOptions options = defaultExecutionOptions())=0;
    virtual void* nativeHandle()=0;
    virtual void setMinThreads(unsigned int min)=0;
    virtual void setMaxThreads(unsigned int max)=0;
    boost::synchronized_value<boost::function<void()>> _emergencyCallback;
    const std::string _name;
  };

  class QI_API_TESTONLY EventLoopAsio final: public EventLoopPrivate
  {
  public:
    static const char* const defaultName;

    explicit EventLoopAsio(int threadCount = 0, std::string name = defaultName,
      bool spawnOnOverload = true);

    EventLoopAsio(int threadCount, int minThreadCount, int maxThreadCount,
                  std::string name, bool spawnOnOverload);

    ~EventLoopAsio() override;

    bool isInThisContext() const override;
    void start(int nthreads) override;
    void join() override;
    void stop() override;
    qi::Future<void> asyncCall(qi::Duration delay,
      boost::function<void ()> callback, ExecutionOptions options = defaultExecutionOptions()) override;
    void post(qi::Duration delay,
      const boost::function<void ()>& callback, ExecutionOptions options = defaultExecutionOptions()) override;
    qi::Future<void> asyncCall(qi::SteadyClockTimePoint timepoint,
        boost::function<void ()> callback, ExecutionOptions options = defaultExecutionOptions()) override;
    void post(qi::SteadyClockTimePoint timepoint,
        const boost::function<void ()>& callback, ExecutionOptions options = defaultExecutionOptions()) override;
    void* nativeHandle() override;
    void setMinThreads(unsigned int min) override;
    void setMaxThreads(unsigned int max) override;
    int workerCount() const;
    MilliSeconds maxIdleDuration() const;
  private:
    // Strongly-typed wrapper around a boolean.
    using UpdateLastWorkDate = ka::ark_mutable_t<bool>;

    /// Destructible D
    template<typename D>
    void invoke_maybe(boost::function<void()> f, qi::uint64_t id, qi::Promise<void> p,
        const boost::system::error_code& erc, D countTask, UpdateLastWorkDate);
    void runWorkerLoop();
    void runPingLoop();

    qi::Future<void> asyncCallInternal(
      qi::Duration delay, boost::function<void ()> callback,
      ExecutionOptions options, UpdateLastWorkDate);

    qi::Future<void> asyncCallInternal(
      qi::SteadyClockTimePoint timepoint, boost::function<void ()> callback,
      ExecutionOptions options, UpdateLastWorkDate);

    boost::asio::io_service _io;
    std::atomic<boost::asio::io_service::work*> _work; // keep io.run() alive
    std::atomic<int> _minThreads;
    std::atomic<int> _maxThreads;

    class WorkerThreadPool;
    std::unique_ptr<WorkerThreadPool> _workerThreads;
    std::thread _pingThread;

    std::atomic<int64_t> _totalTask {0};
    std::atomic<int64_t> _activeTask {0};
    const bool _spawnOnOverload;
  };
}

#endif  // _SRC_EVENTLOOP_P_HPP_
