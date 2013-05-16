/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/log.hpp>
#include <qitype/objectfactory.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <qi/application.hpp>

qiLogCategory("qitype.objectfactory");

namespace qi {

  // Factory system
  // We need thread-safeness, and we can be used at static init.
  // But at static init, thread-safeness is not required.
  // So lazy-init of the mutex should do the trick.
  static boost::recursive_mutex *_f_mutex_struct = 0;
  static boost::recursive_mutex *_f_mutex_load = 0;
  static std::vector<std::string>* _f_keys = 0;

  typedef std::map< std::string, std::vector<std::string> > LoadedServiceMap;
  static LoadedServiceMap       *_f_loadedService;


  typedef std::map<std::string, boost::function<qi::ObjectPtr (const std::string&)> > FactoryMap;
  static FactoryMap* _f_map = 0;

  static void _f_init()
  {
    if (!_f_mutex_struct)
    {
      _f_mutex_struct = new boost::recursive_mutex;
      _f_mutex_load = new boost::recursive_mutex;
      _f_keys = new std::vector<std::string>;
      _f_map = new FactoryMap;
      _f_loadedService = new LoadedServiceMap;
    }
  }

  bool registerObjectFactory(const std::string& name, boost::function<qi::ObjectPtr (const std::string&)> factory)
  {
    qiLogDebug() << "registering " << name;
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i != _f_map->end())
      qiLogWarning() << "Overriding factory for " <<name;
    else
      _f_keys->push_back(name);
    (*_f_map)[name] = factory;
    return true;
  }

  ObjectPtr createObject(const std::string& name)
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i == _f_map->end())
      return ObjectPtr();
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
    std::vector<std::string> ret;
    std::vector<std::string>& keys = *_f_keys;
    {
      boost::recursive_mutex::scoped_lock sl(*_f_mutex_load);

      //already loaded?
      LoadedServiceMap::iterator it;
      it = _f_loadedService->find(name);
      if (it != _f_loadedService->end()) {
        qiLogDebug() << "Library " << name << " already loaded.";
        return it->second;
      }

      unsigned int count = keys.size();
      qiLogDebug() << count <<" object before load";
      Application::loadModule(name, flags);
      {
        boost::recursive_mutex::scoped_lock sl2(*_f_mutex_struct);
        qiLogDebug() << keys.size() <<" object after load";
        if (count != keys.size())
          ret = std::vector<std::string>(&keys[count], &keys[0] + keys.size());
        (*_f_loadedService)[name] = ret;
      }
    }
    return ret;
  }

}

