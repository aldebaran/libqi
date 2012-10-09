/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

/*!
 * \internal
 * \class PopulationGenerator
 * \brief Handle multitude of qi::Session allocation and destruction.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \internal
 * \fn PopulationGenerator::populateClients(const std::string &serviceDirectoryUrl, int nbWantedClients)
 * \brief Allocate and connect a given number of qi::Session to service directory.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \internal
 * \fn PopulationGenerator::clientPopulation()
 * \brief Getter to client qi::Session test population.
 * \return Reference to internal vector containing client qi::Sessions.
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_POPULATIONGENERATOR_HPP_
#define _TESTS_LIBTESTSESSION_POPULATIONGENERATOR_HPP_

#include <vector>
#include <qimessaging/session.hpp>

class PopulationGenerator
{
public:
  PopulationGenerator();
  ~PopulationGenerator();

public:
  bool populateClients(const std::string &serviceDirectoryUrl, int nbWantedClients);
  const std::vector<qi::Session*>&   clientPopulation() const;

private:
  std::vector<qi::Session*> _clients;
};

#endif // !_TESTS_LIBTESTSESSION_POPULATIONGENERATOR_HPP_
