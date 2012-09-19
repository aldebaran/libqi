/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/object_factory.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <qi/application.hpp>

namespace qi {

  // Factory system
  // We need thread-safeness, and we can be used at static init.
  // But at static init, thread-safeness is not required.
  // So lazy-init of the mutex should do the trick.
  static boost::recursive_mutex *_f_mutex_struct = 0;
  static boost::recursive_mutex *_f_mutex_load = 0;
  static std::vector<std::string>* _f_keys = 0;
  typedef std::map<std::string, boost::function<qi::GenericObject (const std::string&)> > FactoryMap;
  static FactoryMap* _f_map = 0;
  static void _f_init()
  {
    if (!_f_mutex_struct)
    {
      _f_mutex_struct = new boost::recursive_mutex;
      _f_mutex_load = new boost::recursive_mutex;
      _f_keys = new std::vector<std::string>;
      _f_map = new FactoryMap;
    }
  }

  bool registerObjectFactory(const std::string& name, boost::function<qi::GenericObject (const std::string&)> factory)
  {
    qiLogDebug("qi.factory") << "registering " << name;
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i != _f_map->end())
      qiLogWarning("qi.object") << "Overriding factory for " <<name;
    else
      _f_keys->push_back(name);
    (*_f_map)[name] = factory;
    return true;
  }

  GenericObject createObject(const std::string& name)
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i == _f_map->end())
      return GenericObject();
    return (i->second)(name);
  }

  std::vector<std::string> listObjectFactories()
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    return *_f_keys;
  }

  std::vector<std::string> loadObject(const std::string& name, int flags)
  {
    /* Do not hold mutex_struct while calling dlopen/loadModule,
  * just in case static initialization of the module happens
  * in an other thread: it will likely call registerObjectFactory
  * which will acquire the mutex_struct.
  * We are still asserting that said initialization synchronously
  * finishes before dlopen/loadModule returns.
  */
    _f_init();
    std::vector<std::string>& keys = *_f_keys;
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_load);
    unsigned int count = keys.size();
    qiLogDebug("qi.factory") << count <<" object before load";
    Application::loadModule(name, flags);
    boost::recursive_mutex::scoped_lock sl2(*_f_mutex_struct);
    qiLogDebug("qi.factory") << keys.size() <<" object after load";
    if (count != keys.size())
      return std::vector<std::string>(&keys[count], &keys[keys.size()]);
    else
      return std::vector<std::string>();
  }

}

