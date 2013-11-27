/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <cstring>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

qiLogCategory("qimaster");

int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);

  if (argc == 2 &&
      (strcmp(argv[0], "-h") == 0 || strcmp(argv[1], "--help") == 0))
  {
    std::cerr << "Usage: " << argv[0] << " [ENDPOINTS...]" << std::endl;
    return 0;
  }

  {
    qi::Session sd;
    std::vector<qi::Url> endpoints;
    if (argc == 1)
    {
      endpoints.push_back("tcp://0.0.0.0:9559");
    }
    else
    {
      for (int i = 1; i < argc; i++)
      {
        qi::Url url(argv[i]);

        if (url.protocol() == "tcps")
        {
          sd.setIdentity("tests/server.key", "tests/server.crt");
        }

        endpoints.push_back(url);
      }
    }

    for (std::vector<qi::Url>::const_iterator it = endpoints.begin();
         it != endpoints.end();
         it++)
    {
      qi::Future<void> f = sd.listenStandalone(*it);
      f.wait(3000);
      if (f.hasError())
      {
        qiLogError() << "Failed to listen on " << it->str() <<
                        ". Is there another service running on this address?";
        exit(1);
      }
    }

    app.run();
  }

  return 0;
}
