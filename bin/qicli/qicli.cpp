/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <iomanip>
#include <istream>
#include <sstream>

#include <qitype/genericobject.hpp>
#include <qi/future.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/url.hpp>

qi::Session session;
std::map<const std::string, qi::ObjectPtr> services;

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
    if (servs[i].name() != *it)
      continue;
    std::cout << servs[i].name() << std::endl
              << "  id:       " << servs[i].serviceId() << std::endl
              << "  machine:  " << servs[i].machineId() << std::endl
              << "  process:  " << servs[i].processId() << std::endl
              << "  endpoints:" << std::endl;
    for (qi::UrlVector::const_iterator it2 = servs[i].endpoints().begin();
         it2 != servs[i].endpoints().end();
         ++it2)
    {
      std::cout << "    " << it2->str() << std::endl;
    }

    qi::Future<qi::ObjectPtr> fut = session.service(*it);
    if (fut.hasError()) {
      std::cerr << "service: could not get object: " << fut.error() << std::endl;
      continue;
    }

    qi::ObjectPtr obj = fut.value();
    services[*it] = obj;

    qi::details::printMetaObject(std::cout, obj->metaObject());
  }
}

/****************
*    SERVICES   *
****************/
static void cmd_services(const command           &cmd,
                         command::const_iterator &QI_UNUSED(it))
{
  bool enum_all = false;
  if (std::find(cmd.begin(), cmd.end(), "-v") != cmd.end())
    enum_all = true;
  std::vector<qi::ServiceInfo> servs = session.services();
  for (unsigned int i = 0; i < servs.size(); ++i)
  {
    if (enum_all) {
      command ncmd;
      command::const_iterator it;

      ncmd.push_back(servs[i].name());
      it = ncmd.begin();
      cmd_service(ncmd, it);
      std::cout << std::endl;
      continue;
    }
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

  for (int i = 2;
       i < argc;
       ++i)
  {
    cmd.push_back(argv[i]);
  }

  execute(cmd);
}

static void usage(char *argv0)
{
  std::cout << "Usage: " << argv0 << " [ADDRESS CMD]" << std::endl;
  std::cout << "  connect ADDRESS" << std::endl
            << "  services [-v]" << std::endl
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

  qi::Application app(argc, argv);

  if (argc == 1)
  {
    from_stdin();
  }
  else
  {
    from_argv(argc, argv);
  }

  session.close();
  return (0);
}
