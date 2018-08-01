/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "populationgenerator.hpp"

PopulationGenerator::PopulationGenerator()
{

}

PopulationGenerator::~PopulationGenerator()
{
}

bool PopulationGenerator::populateClients(const std::string &serviceDirectoryUrl, int nbWantedClients)
{
  int i = 0;
  qi::SessionPtr session;
  bool ret = true;

  while (i < nbWantedClients)
  {
    session = qi::makeSession();

    if (session->connect(serviceDirectoryUrl).wait(1000) != qi::FutureState_FinishedWithValue)
      ret = false;

    _clients.push_back(session);
    i++;
  }

  return ret;
}

const std::vector<qi::SessionPtr>&   PopulationGenerator::clientPopulation() const
{
  return _clients;
}
