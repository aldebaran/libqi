/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <istream>
#include <sstream>

#include <qimessaging/object.hpp>
#include <qimessaging/session.hpp>

qi::Session session;
std::map<const std::string, qi::Object *> services;

typedef std::vector<std::string> command;

/****************
*    SERVICE    *
****************/
static void cmd_service(const command           &cmd,
                        command::const_iterator &it)
{
  if (it == cmd.end())
  {
    std::cerr << "service: not enough parameters" << std::endl;
    return;
  }

  std::vector<qi::ServiceInfo> servs = session.services();
  for (unsigned int i = 0; i < servs.size(); ++i)
  {
    if (servs[i].name() == *it)
    {
      std::cout << servs[i].name() << std::endl
                << "  id:       " << servs[i].serviceId() << std::endl
                << "  machine:  " << servs[i].machineId() << std::endl
                << "  process:  " << servs[i].processId() << std::endl
                << "  endpoints:" << std::endl;
      for (std::vector<std::string>::const_iterator it = servs[i].endpoints().begin();
           it != servs[i].endpoints().end();
           it++)
      {
        std::cout << "    " << *it << std::endl;
      }

      std::cout << "  methods:" << std::endl;
      qi::Object *obj = session.service(*it);
      if (obj)
      {
        services[*it] = obj;

        qi::MetaObject &mobj = obj->metaObject();
        for (std::vector<qi::MetaMethod>::const_iterator it2 = mobj._methods.begin();
             it2 != mobj._methods.end();
             ++it2)
        {
          std::cout << "    " << (*it2).signature() << std::endl;
        }
      }
      else
      {
        std::cerr << "service: could not get object" << std::endl;
      }
    }
  }
}

/****************
*    SERVICES   *
****************/
static void cmd_services(const command           &cmd,
                         command::const_iterator &it)
{
  std::vector<qi::ServiceInfo> servs = session.services();
  for (unsigned int i = 0; i < servs.size(); ++i)
  {
    std::cout << "[" << servs[i].serviceId() << "] "
              << servs[i].name() << std::endl;
  }
}

/****************
*    SESSION    *
****************/
static void cmd_session(const command           &cmd,
                        command::const_iterator &it)
{
  if (it == cmd.end())
  {
    std::cerr << "session: not enough parameters" << std::endl;
    return;
  }

  if (*it == "connect")
  {
    ++it;
    if (it == cmd.end())
    {
      std::cerr << "session connect: not enough parameters" << std::endl;
      return;
    }
    else
    {
      session.connect(*it);
      session.waitForConnected();
    }
  }
  else if (*it == "services")
  {
    ++it;
    std::vector<qi::ServiceInfo> servs = session.services();
    for (unsigned int i = 0; i < servs.size(); ++i)
    {
      std::cout << "[" << servs[i].serviceId() << "] "
                << servs[i].name() << std::endl;
    }
  }
  else
  {
    std::cerr << "unexpected token: " << *it << std::endl;
  }
}

static void execute(const command &cmd)
{
  command::const_iterator it = cmd.begin();

  if (it == cmd.end())
  {
    return;
  }

  if (*it == "session")
  {
    cmd_session(cmd, ++it);
  }
  else if (*it == "service")
  {
    cmd_service(cmd, ++it);
  }
  else if (*it == "services")
  {
    cmd_services(cmd, ++it);
  }
  else
  {
    std::cerr << "unexpected token: " << *it << std::endl;
  }
}

static void from_stdin()
{
  char line[8192];

  std::cout << "% ";
  while (std::cin.getline(line, sizeof(line)))
  {
    std::stringstream ssin(line, std::stringstream::in);
    command cmd;
    std::string input;

    while (ssin >> input)
    {
      cmd.push_back(input);
    }

    execute(cmd);

    std::cout << "% ";
  }
}

static void from_argv(int   argc,
                      char *argv[])
{
  command cmd;

  session.connect(argv[1]);
  session.waitForConnected();

  for (unsigned int i = 2;
       i < argc;
       ++i)
  {
    cmd.push_back(argv[i]);
  }

  execute(cmd);
}

static void usage(char *argv0)
{
  std::cout << "Usage: " << basename(argv0) << " [ADDRESS CMD]" << std::endl;
  std::cout << "  connect ADDRESS" << std::endl
            << "  services" << std::endl
            << "  service SERVICE" << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc == 2 &&
      (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
  {
    usage(argv[0]);
    return (0);
  }

  if (argc == 1)
  {
    from_stdin();
  }
  else
  {
    from_argv(argc, argv);
  }

  return 0;
}
