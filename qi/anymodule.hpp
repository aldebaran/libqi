/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_PACKAGE_HPP_
#define _QITYPE_PACKAGE_HPP_

#include <string>
#include <map>
#include <vector>

#include <qi/log.hpp>
#include <qi/anyobject.hpp>
#include <qi/anyvalue.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>

namespace qi
{
  class ModuleBuilder;

  /* Factory helper functions
  */

  struct ModuleInfo {
    std::string name;
    std::string type;
    std::string path;
  };

  /** A qi Module. basically an Object and a ModuleInfo
   *
   * \includename{qi/anymodule.hpp}
   */
  class QI_API AnyModule : public AnyObject
  {
  public:
    AnyModule()
    {}

    AnyModule(const ModuleInfo &moduleInfo, const qi::AnyObject& object)
      : AnyObject(object)
      , _moduleInfo(moduleInfo)
    {}

    const std::string& moduleName() const {
      return _moduleInfo.name;
    }

    const ModuleInfo &moduleInfo() const {
      return _moduleInfo;
    }

    ModuleInfo _moduleInfo;
  };


  /** class given to the user to building a module
   */
  class QI_API ModuleBuilder : public DynamicObjectBuilder {
  public:
    ModuleBuilder(const ModuleInfo& mi)
      : _moduleInfo(mi)
    {
      this->setThreadingModel(ObjectThreadingModel_MultiThread);
    }

    void setModuleInfo(const ModuleInfo& mi) {
      _moduleInfo = mi;
    }

    void setModulePath(const std::string& name) {
      _moduleInfo.path = name;
    }

    ModuleInfo& moduleInfo() {
      return _moduleInfo;
    }

    const std::string& moduleName() const {
      return _moduleInfo.name;
    }

    AnyModule module() {
      if (!_mod)
        _mod = AnyModule(_moduleInfo, object());
      return _mod;
    }

    ModuleInfo _moduleInfo;
    AnyObject  _object;
    AnyModule  _mod;
  };

  /** register a module into the module map
   */
  QI_API bool registerCppEmbeddedModule(const std::string& moduleName, boost::function<void (ModuleBuilder*)> fun);

  // ###### MODULE API ######
  /**
   * list all available modules (cross language
   */
  QI_API std::vector<ModuleInfo> listModules();


  /** find a module and import it, this is cross language, that's the main module entry point
   *
   *  this function dispatch to \<lang\> module factory, if the module is not already registered
   */
  QI_API AnyModule import(const std::string& name);
  QI_API AnyModule import(const ModuleInfo& name);

}

QI_TYPE_STRUCT(qi::ModuleInfo, name, type, path);

/** register a normal module, a function will be exported to allow the module loader to init the module
 */
#define QI_REGISTER_MODULE(modname, fun)                                   \
  extern "C" QI_EXPORT_API void qi_module_init(::qi::ModuleBuilder *mb) {  \
    if (modname != mb->moduleName()) {                                     \
      qiLogCategory("qi.module");                                          \
      qiLogError() << "module name do not match: '"                        \
                   << modname << "' vs '" << mb->moduleName() << "'";      \
      return;                                                              \
    }                                                                      \
    (*fun)(mb);                                                            \
  }

/** register an embedded module. (the library do not have to be loaded)
 */
#define QI_REGISTER_MODULE_EMBEDDED(name, fun) \
  static bool BOOST_PP_CAT(_register_module, __COUNTER__) = ::qi::registerCppEmbeddedModule(name, fun);


//####################### MODULE LANGUAGE FACTORY #########################

/**
 * Language factory, for each language (C++, Python, ...) a factory should be registered.
 * The factory will be used to load the module
 */

namespace qi {

  using ModuleFactoryFunctor = boost::function<AnyModule(const qi::ModuleInfo&)>;
  /** Register a module factory for a given language
   */
  QI_API bool registerModuleFactory(const std::string& name, ModuleFactoryFunctor fun);
}

/** Register a Module Factory.
 *  Module factory are by language, the default one is for C++ and provided by libqi.
 *  A python one is provided by libqi-python as a plugin.
 */
#define QI_REGISTER_MODULE_FACTORY(factoryType, fun) \
  static bool BOOST_PP_CAT(_register_module_factory, __COUNTER__) = ::qi::registerModuleFactory(factoryType, fun)

/** Register a Module Factory as a plugins.
 */
#define QI_REGISTER_MODULE_FACTORY_PLUGIN(factoryType, fun)   \
  extern "C" QI_EXPORT_API void module_factory_plugin() {     \
    ::qi::registerModuleFactory(factoryType, fun);            \
  }




#endif
