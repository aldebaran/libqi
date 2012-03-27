/**
 * This is a simple qiMessaging example
 */

#include <qimessaging/future.hpp>
#include <qi/os.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>

void delay_and_set(qi::Promise<int> promise) {
  qi::os::sleep(1);
  promise.setValue(42);
}

int main(int, char *[])
{
  qi::Promise<int> promise;
  qi::Future<int>  future;

  //get the future from the promise
  future = promise.future();

  boost::thread thd(boost::bind(&delay_and_set, promise));

  std::cout << "waiting for the future value" << std::endl;
  //future.value will block until the value is available
  std::cout << "future value:" << future.value() << std::endl;
  return 0;
}
