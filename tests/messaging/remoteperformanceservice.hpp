/*
**
** Copyright (C) 2018 Softbank Robotics Europe
*/
#pragma once
#include <thread>
#include <random>
#include <chrono>
#include <qi/anyobject.hpp>

struct RemotePerformanceService
{
  using nanosecs = std::int64_t;

  void setObject(qi::AnyObject obj)
  {
    objects.push_back(obj);
  }

  void setObjectList(std::vector<qi::AnyObject> objs)
  {
    objects = std::move(objs);
  }

  // Returns nanoseconds taken for each call the object's function.
  std::vector<nanosecs> measureCallDuration(const std::string& functionToCall)
  {
    return measureOperationDuration([&](qi::AnyObject& o){
      auto ft = o.async<void>(functionToCall);
      handleCallResult(ft, __func__);
    });
  }

  // Returns nanoseconds taken to call the object's function.
  std::vector<nanosecs> measureCallDurationArgument(const std::string& functionToCall, qi::AnyValue arg)
  {
    return measureOperationDuration([&](qi::AnyObject& o) {
      auto ft = o.async<void>(functionToCall, arg);
      handleCallResult(ft, __func__);
    });
  }

  // Returns nanoseconds taken to get an object's property value.
  std::vector<nanosecs> measureGetPropertyDuration(const std::string& propertyName)
  {
    return measureOperationDuration([&](qi::AnyObject& o) {
      auto ft = o.property<qi::AnyValue>(propertyName).async();
      handleCallResult(ft, __func__);
    });
  }

  // Returns nanoseconds taken to set an object's property value.
  std::vector<nanosecs> measureSetPropertyDuration(const std::string& propertyName)
  {
    return measureOperationDuration([&](qi::AnyObject& o) {
      auto connectFt = o.connect(propertyName, [propertyName](int value){
        qiLogInfo("RemotePerformanceService") << "Property '" << propertyName << "' set to '" << value << "'";
      }).async();
      auto disconnectOnScopeExit = ka::scoped([&]{
        auto linkId = connectFt.value();
        o.disconnect(linkId);
      });

      handleCallResult(connectFt, std::string{__func__} + ".Property::connect()");

      auto ft = o.setProperty(propertyName, qi::AnyValue::from(99)).async();
      handleCallResult(ft, __func__);
    });
  }

  void clear()
  {
    objects.clear();
  }

  void setMeasureCount(int count)
  {
    measureCount = count;
  }

private:

  std::vector<qi::AnyObject> objects;
  int measureCount = 4; // arbitrary default
  int totalCallCount = 0;

  std::default_random_engine randomEngine{ std::random_device{}() };

  qi::AnyObject randomObject()
  {
    if(objects.empty())
      return {};

    std::uniform_int_distribution<size_t> distribution(0, objects.size() -1);
    const auto randomIdx = distribution(randomEngine);
    return objects[randomIdx];
  }

  // Returns nanoseconds taken to do the operation
  // Requires: Procedure< _ (qi::AnyObject)> Operation
  template<class Operation>
  std::vector<nanosecs> measureOperationDuration(Operation&& operation)
  {
    using namespace std::chrono;

    if (objects.empty())
      throw std::runtime_error("No stored object to call");

    std::vector<nanosecs> results;

    for (int i = 0; i < measureCount; ++i)
    {
      qi::AnyObject o = randomObject();

      ++totalCallCount;

      qiLogInfo("RemotePerformanceService") << "Test Operation START: call num "
        << totalCallCount << "/measure " << i
        << " on " << " UID{" << o.uid() << "}";

      const auto startTime = high_resolution_clock::now();
      operation(o);
      const auto endTime = high_resolution_clock::now();

      const auto recordedDuration = endTime - startTime;
      const auto recordedNanosecs = static_cast<nanosecs>(nanoseconds{recordedDuration}.count());

      qiLogInfo("RemotePerformanceService") << "Test Operation STOP: call num "
        << totalCallCount << "/measure " << i
        << " on " << " UID{" << o.uid() << "}: "
        << duration_cast<milliseconds>(recordedDuration).count() << " ms ("
        << recordedNanosecs << " ns)"
        ;

      results.emplace_back(recordedNanosecs);
    }

    return results;
  }

  template<class T>
  void handleCallResult(const qi::Future<T>& ft, std::string source)
  {
    const auto timeout = qi::Seconds{ 20 };

    auto state = ft.wait(timeout);
    switch (state)
    {
    case qi::FutureState_FinishedWithError:
    {
      qiLogWarning("RemotePerformanceService") << source << ": Measured call returned an error: " << ft.error();
      return;
    }
    case qi::FutureState_Canceled:
    {
      qiLogWarning("RemotePerformanceService") << source << ": Measured call was canceled";
      return;
    }
    case qi::FutureState_Running:
    {
      auto newState = ft.wait(timeout);
      if (newState == qi::FutureState_Running)
      {
        std::stringstream stream;
        stream << source << ": Measured call did not finish after timeout ("
          << boost::chrono::duration_cast<qi::MilliSeconds>(timeout).count() << " ms)";
        qiLogError("RemotePerformanceService") << stream.str();
        throw std::runtime_error(stream.str());
      }
      return handleCallResult(ft, source);
    }
    case qi::FutureState_FinishedWithValue:
    {
      qiLogInfo("RemotePerformanceService") << source << ": Measured call finished successfully.";
      return;
    }
    default:
      std::stringstream stream;
      stream << source << ": Unhandled future state";
      qiLogError("RemotePerformanceService") << stream.str();
      throw std::runtime_error(stream.str());
    }
  }

};

QI_REGISTER_OBJECT(RemotePerformanceService, setObject, setObjectList,
  measureCallDuration, measureGetPropertyDuration, measureSetPropertyDuration, measureCallDurationArgument,
  clear, setMeasureCount)
