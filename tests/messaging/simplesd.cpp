#include <qi/applicationsession.hpp>

int main(int argc, char** argv)
{
  qi::ApplicationSession app{argc, argv};
  app.session()->setIdentity(qi::path::findData("qi", "server.key"),
                             qi::path::findData("qi", "server.crt"));
  app.run();
}
