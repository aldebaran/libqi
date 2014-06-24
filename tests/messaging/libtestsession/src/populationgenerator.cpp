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
  for (std::vector<qi::Session *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    delete (*it);
}

bool PopulationGenerator::populateClients(const std::string &serviceDirectoryUrl, int nbWantedClients)
{
  int i = 0;
  qi::Session *session;
  bool ret = true;

  while (i < nbWantedClients)
  {
    session = new qi::Session();

    if (session->connect(serviceDirectoryUrl).wait(1000) != qi::FutureState_FinishedWithValue)
      ret = false;

    _clients.push_back(session);
    i++;
  }

  return ret;
}

const std::vector<qi::Session*>&   PopulationGenerator::clientPopulation() const
{
  return _clients;
}
