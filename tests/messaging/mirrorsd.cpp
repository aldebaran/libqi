#include <mutex>
#include <string>
#include <unordered_map>
#include <qi/applicationsession.hpp>
#include <qi/log.hpp>

qiLogCategory("MirrorSD");

int main(int argc, char** argv)
{
  qi::ApplicationSession app{argc, argv};
  auto originalSd = app.session();
  originalSd->connect(app.url());

  auto mirroredSd = qi::makeSession();
  mirroredSd->listenStandalone(app.allListenUrl());

  std::unordered_map<std::string, unsigned int> registeredServices;
  std::mutex m;

  auto tryMirrorService = [&](const std::string& serviceName)
  {
    try
    {
      std::lock_guard<std::mutex> l(m);
      registeredServices[serviceName] =
        mirroredSd->registerService(serviceName, originalSd->service(serviceName).value()).value();
    }
    catch (const std::exception& e)
    {
      qiLogInfo() << "Could not register " << serviceName << ": " << e.what();
    }
  };

  // When one of the services we track is registered to the private service
  // directory, register it to the public service directory.
  originalSd->serviceRegistered.connect([&](unsigned int, const std::string& service)
  {
    tryMirrorService(service);
  });

  // When one of the services we track is unregistered from the private service
  // directory, unregister it from the public service directory.
  originalSd->serviceUnregistered.connect([&](unsigned int, const std::string& service)
  {
    {
      std::lock_guard<std::mutex> l(m);
      auto it = registeredServices.find(service);
      if (it == registeredServices.end())
        return;

      auto id = it->second;
      registeredServices.erase(it);

      try
      {
        mirroredSd->unregisterService(id);
      }
      catch (const std::exception& e)
      {
        qiLogInfo() << "Could not unregister " << service << ": " << e.what();
      }
    }
  });

  // For all services already available, mirror directly
  auto services = originalSd->services().value();
  for (const auto& service: services)
  {
    tryMirrorService(service.name());
  }

  app.run();

  return EXIT_SUCCESS;

}
