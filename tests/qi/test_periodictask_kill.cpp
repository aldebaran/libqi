
#include <thread>
#include <chrono>
#include <signal.h>

#include <boost/predef.h>

#include <qi/anymodule.hpp>
#include <qi/periodictask.hpp>
#include <qi/applicationsession.hpp>
#include <qi/os.hpp>

qiLogCategory("qi.test_periodictask_kill_service");

class TestPeriodicTaskKillService
{
public:
  TestPeriodicTaskKillService(qi::Application& app)
    : _app(app)
  {
    _perTask.setPeriod(qi::MilliSeconds(100));
    _perTask.compensateCallbackTime(true);
    _perTask.setCallback([=] {
      qiLogInfo() << "NYAAAA";
    });

    _perTask.start(true);
  }

  ~TestPeriodicTaskKillService()
  {
    qiLogInfo() << "STOP SERVICE ...";
    _perTask.stop();
    qiLogInfo() << "STOP SERVICE - DONE";
    // If we didn't block, then we succeeded.
    // (necessary for test driver to know that it's a success)
    _app.stop();
  }

  int dummy;

private:
  qi::Application& _app;
  qi::PeriodicTask _perTask;
};
QI_REGISTER_OBJECT(TestPeriodicTaskKillService, dummy)

void raiseTerminateSignal() // To reviewers: seems super useful. Should I put it in our test library?
{
  std::raise(SIGTERM);
}

int main(int argc, char** argv)
{
  // We want this service to be kept alive even after
  // the whole application (including the thread-pool) is destroyed.
  // PeriodicTask should never block on destruction even in this situation.
  qi::Object<TestPeriodicTaskKillService> service;
  qi::Promise<void> promiseReady;
  auto ftReady = promiseReady.future();

  std::thread killer{ [=] {
      // We send a terminate request to this process to see if it's blocking on destruction.
      ftReady.wait();
      std::this_thread::sleep_for(std::chrono::seconds(3));
      raiseTerminateSignal();
  }};

  { // The scope is necessary for the test as we want to destro the Application first.
    qi::ApplicationSession app{ argc, argv };
    app.startSession();

    auto session = app.session();
    service = new TestPeriodicTaskKillService{ app };
    session->registerService("TestPeriodicTaskKillService", service);
    promiseReady.setValue(nullptr);
    app.run();
  }

  killer.join();
}

