/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <qi/messaging/publisher.hpp>
#include <boost/program_options.hpp>


namespace po = boost::program_options;

bool gContinue;

void exitHandler(int sig) {
  // Trap CTRL-C
  // The fgets will receive a NULL pointer and exit cleanly.
}

void qichatprompt(std::string& username, const std::string& addr) {
  qi::Publisher publisher("qichat-prompt");
  publisher.connect(addr);
  publisher.advertiseTopic<std::string>("qichat", true);

  std::string connectMessage = "[";
  connectMessage += username;
  connectMessage += " has joined the session]\n";

  std::string disconnectMessage = "[";
  disconnectMessage += username;
  disconnectMessage += " has left the session]\n";

  std::string prompt = username + "> ";
  publisher.publish("qichat", connectMessage);
  std::cout <<
    "============== qichat-prompt ==============" << std::endl <<
    "== Welcome " << username << ","              << std::endl <<
    "== Type a message, and hit return to send."  << std::endl <<
    "== Press CTRL-C to quit."                    << std::endl <<
    "===========================================" << std::endl;

  char textbuf[1024];
  while (gContinue) {
    std::cout << prompt;
    char *rcc = fgets(textbuf, sizeof(textbuf), stdin);
    if (rcc != NULL) {
       publisher.publish("qichat", username + std::string(": ") + std::string(rcc));
    } else {
      // if CTRL-C was pressed we come here
      gContinue = false;
      publisher.publish("qichat", disconnectMessage);
    }
  }
  std::cout << std::endl <<
    "===========================================" << std::endl;
}

int main(int argc, char *argv[])
{
  gContinue = true;
  (void) signal(SIGINT, exitHandler);
  // declare the program options
  po::options_description desc("Usage:\n  qichat-prompt username masterAddress [options]\nOptions");
  desc.add_options() 
    ("help", "Print this help.")
    ("master-address",
    po::value<std::string>()->default_value(std::string("127.0.0.1:5555")),
    "The master address")
    ("username", "Your username");

  // allow master address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("username", 1);
  pos.add("master-address", 2);

  // parse and store
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
      options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    std::string username;
    std::string address;

    if(vm.count("username")==1) {
      username = vm["username"].as<std::string>();
    } else {
      std::cout << desc << "\n";
      return 1;
    }

    if(vm.count("master-address")==1) {
      address = vm["master-address"].as<std::string>();
    } else {
      std::cout << desc << "\n";
      return 1;
    }
    qichatprompt(username, address);

  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
