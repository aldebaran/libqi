/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/c/application_c.h>
#include <qi/application.hpp>

qi_application_t *qi_application_create(int ac, char **av)
{
  qi::Application* app = new qi::Application(ac, av);

  return (qi_application_t *) app;
}

void              qi_application_destroy(qi_application_t *application)
{
  qi::Application* app = reinterpret_cast<qi::Application*>(application);

  delete app;
}

void              qi_application_run(qi_application_t *application)
{
  qi::Application* app = reinterpret_cast<qi::Application *>(application);

  app->run();
}