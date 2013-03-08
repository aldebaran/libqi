/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qic/servicedirectory.h>
#include <qimessaging/servicedirectory.hpp>
#include <qic/value.h>
#include "value_p.h"

inline qi::ServiceDirectory *qi_servicedirectory_cpp(qi_servicedirectory_t *value) {
  return reinterpret_cast<qi::ServiceDirectory *>(value);
};

qi_servicedirectory_t *qi_servicedirectory_create()
{
  qi::ServiceDirectory* sd = new qi::ServiceDirectory();
  return (qi_servicedirectory_t *) sd;
}

void              qi_servicedirectory_destroy(qi_servicedirectory_t *servicedirectory)
{
  qi::ServiceDirectory* sd = qi_servicedirectory_cpp(servicedirectory);
  delete sd;
}

void              qi_servicedirectory_listen(qi_servicedirectory_t *servicedirectory, const char *url)
{
  qi::ServiceDirectory* sd = qi_servicedirectory_cpp(servicedirectory);
  qi::Url ur(url);
  sd->listen(ur);
}

void              qi_servicedirectory_close(qi_servicedirectory_t *servicedirectory)
{
  qi::ServiceDirectory* sd = qi_servicedirectory_cpp(servicedirectory);
  sd->close();
}

qi_value_t*       qi_servicedirectory_endpoints(qi_servicedirectory_t *servicedirectory) {
  qi::ServiceDirectory* sd = qi_servicedirectory_cpp(servicedirectory);

  std::vector<qi::Url> eps = sd->endpoints();
  std::vector<std::string> epss;

  std::vector<qi::Url>::iterator it;
  for (it = eps.begin(); it != eps.end(); ++it) {
    epss.push_back(it->str());
  }
  qi_value_t *val = qi_value_create("");
  qi::GenericValuePtr &gvp = qi_value_cpp(val);
  gvp.destroy();
  gvp = qi::GenericValuePtr::from(epss).clone();
  return val;
}
