#include <qi/applicationsession.hpp>

int main(int argc, char** argv)
{
  qi::ApplicationSession app{argc, argv};
  app.run();
}
