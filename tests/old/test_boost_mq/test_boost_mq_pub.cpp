#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>

#include <qimessaging/perf/dataperftimer.hpp>

//using namespace qi::messaging;
using qi::perf::DataPerfTimer;

using namespace boost::interprocess;

const int gThreadCount = 1;
const int gLoopCount = 10000;
unsigned int                  numBytes = 100;

int main_server ()
{
   try{
      //Erase previous message queue
      message_queue::remove("message_queue_send");

      //Create a message_queue.
      message_queue mq
         (create_only               //only create
         ,"message_queue_send"      //name
         ,100                       //max message number
         ,numBytes                      //max message size
         );

      DataPerfTimer dt("Serv");
      std::string                   request = std::string(numBytes, 'B');

      //char data[4096];
      //unsigned int priority;
      //std::size_t recvd_size;

      sleep(1);
      dt.start(gLoopCount, numBytes);
      //Send 100 numbers
      for(int i = 0; i < gLoopCount; ++i){
        //std::cout << "serv:send" << std::endl;
        mq.send(request.data(), numBytes, 0);
        //std::cout << "serv:recv" << std::endl;
        //mqr.receive(data, numBytes, recvd_size, priority);
        //std::cout << "serv:recv end" << std::endl;
      }

      dt.stop();

   }
   catch(interprocess_exception &ex){
      std::cout << ex.what() << std::endl;
      return 1;
   }

   return 0;
}


// #include <boost/interprocess/ipc/message_queue.hpp>
// #include <iostream>
// #include <vector>
// using namespace boost::interprocess;

int main_client ()
{
   try{
      //Open a message queue.
      message_queue mq
         (open_only        //only create
         ,"message_queue_send"  //name
         );

      unsigned int priority;
      std::size_t recvd_size;

      DataPerfTimer dt("Messaging void -> ping -> void");
      std::string                   request = std::string(numBytes, 'B');


      std::cout << "numb" << numBytes << std::endl;
      sleep(2);
      dt.start(gLoopCount, numBytes);
      char data[4096];
      //Receive 100 numbers
      for(int i = 0; i < gLoopCount; ++i){
         //int number;
         //std::cout << "clie:recv" << std::endl;
         mq.receive(data, numBytes, recvd_size, priority);
         //std::cout << "clie:send" << std::endl;
         //mqr.send(data, numBytes, 0);
         //std::cout << "clie:send end" << std::endl;
         if(recvd_size != numBytes)
           std::cout << "FAIL" << std::endl;
      }
      dt.stop();
   }
   catch(interprocess_exception &ex){
     //message_queue::remove("message_queue");
      std::cout << ex.what() << std::endl;
      return 1;
   }
   //message_queue::remove("message_queue");
   return 0;
}



int main(int argc, char **argv)
{
  if (argc > 1 && !strcmp(argv[1], "--client"))
  {
    boost::thread thd[gThreadCount];

    for (int i = 0; i < gThreadCount; ++i)
    {
      std::cout << "starting thread: " << i << std::endl;
      thd[i] = boost::thread(&main_client);
    }

    for (int i = 0; i < gThreadCount; ++i)
      thd[i].join();
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server();
  }
  else
  {
    boost::thread             threadServer(&main_server);
    sleep(1);
    boost::thread             threadClient(&main_client);
    threadClient.join();
    sleep(1);
  }
  return 0;
}
