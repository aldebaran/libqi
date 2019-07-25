#include <qi/anymodule.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/path.hpp>
#include <ka/scoped.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread/synchronized_value.hpp>

#ifdef _WIN32
  #include <Windows.h>
#endif

qiLogCategory("qitype.module");

namespace qi
{
  // function called to init a module
  using moduleInitFn = void (*)(ModuleBuilder*);

  // function called to init a module factory
  using moduleFactoryPluginFn = void(*)(void);

  using AnyModuleMap = std::map<std::string, AnyModule>;
  static boost::synchronized_value<AnyModuleMap, boost::mutex> registeredModules;

  /// Language -> Factory Function
  using ModuleFactoryMap = std::map<std::string, ModuleFactoryFunctor>;
  ModuleFactoryMap               gModuleFactory;

  static void loadModuleFactoryPlugins() {
    static bool loaded = false;
    if (loaded)
      return;
    loaded = true;
    std::vector<std::string> vs = qi::path::listLib("qi/plugins", "*qimodule_*_plugin*");
    for (unsigned i = 0; i < vs.size(); ++i) {
      qiLogVerbose() << "found module factory: '" << vs.at(i) << "'";
      void* lib;
      try {
#ifdef _WIN32
        auto errorMode = ka::scoped(GetErrorMode(), [](UINT oldErrorMode)
        {
          SetErrorMode(oldErrorMode);
        });
        SetErrorMode(errorMode.value | SEM_FAILCRITICALERRORS);
#endif
        lib = qi::Application::loadModule(vs.at(i));
      }
      catch (std::exception& e)
      {
        qiLogWarning() << "Can't load module: " << vs.at(i) << ", error: " << e.what();
        continue;
      }
      moduleFactoryPluginFn fn = (moduleFactoryPluginFn)qi::os::dlsym(lib, "module_factory_plugin");
      if (!fn) {
        qiLogWarning() << "Can't load module (no module_factory_plugin found): " << vs.at(i);
        continue;
      }
      fn();
    }
  }

  // Converts `.` to `/`.
  static std::string moduleNameToPath(const std::string& moduleName) {
    return boost::algorithm::replace_all_copy(moduleName, ".", "/");
  }

  static bool isValid(char c) {
    if (c >= 'a' && c <= 'z')
      return true;
    if (c >= 'A' && c <= 'Z')
      return true;
    if (c >= '0' && c <= '9')
      return true;
    if (c == '.' || c == '_')
      return true;
    return false;
  }

  static void checkModule(const std::string& moduleName) {
    if (moduleName.empty() || !boost::algorithm::all_of(moduleName, &isValid))
      throw std::runtime_error("Invalid module name: '" + moduleName + "', use only character from [_.a-zA-Z0-9]");
  }

  static void registerModuleInFactory(const AnyModule& module) {
    loadModuleFactoryPlugins();
    auto modules = registeredModules.synchronize();
    if (modules->find(module.moduleName()) != modules->end())
      throw std::runtime_error("module already registered: " + module.moduleName());
    qiLogVerbose() << "Registering module " << module.moduleName();
    (*modules)[module.moduleName()] = module;
  }

  //return value is pointless... just for MACRO. see QI_REGISTER_MODULE
  bool registerCppEmbeddedModule(const std::string& modulename, boost::function<void (ModuleBuilder*)> fun) {
    qi::ModuleInfo mi;
    mi.type = "cpp";
    mi.name = modulename;
    mi.path = "<internal>";
    ModuleBuilder mb(mi);
    fun(&mb);
    registerModuleInFactory(mb.module());
    return true;
  }

  bool registerModuleFactory(const std::string& name, ModuleFactoryFunctor fun)
  {
    gModuleFactory[name] = fun;
    return true;
  }

  static ModuleInfo findModuleInFs(const std::string& name) {
    //lookup for module in
    const qi::Path p(qi::path::findData("qi/module", name + ".mod"));

    //TODO: throwing seriously?
    if (!p.isRegularFile())
      throw std::runtime_error("no module found: '" + name + "'");

    ModuleInfo mi;
    mi.name = name;
    boost::filesystem::ifstream is(p);
    is >> mi.type;

    qiLogVerbose() << "type: '" << mi.type << "'";
    return mi;
  }

  static AnyModule findModuleInFactory(const std::string& name) {
    checkModule(name);
    auto modules = registeredModules.synchronize();
    auto it = modules->find(name);
    if (it != modules->end())
    {
      qiLogDebug() << "Module " << name << " already loaded.";
      return it->second;
    }
    return AnyModule();
  }

  AnyModule import(const std::string& name) {
    loadModuleFactoryPlugins();

    AnyModule mod = findModuleInFactory(name);
    if (mod)
      return mod;

    ModuleInfo mi = findModuleInFs(name);

    ModuleFactoryMap::const_iterator it = gModuleFactory.find(mi.type);
    if (it == gModuleFactory.end())
      throw std::runtime_error("module factory for module type: " + mi.type + " is not available");

    return it->second(mi);
  }

  AnyModule import(const ModuleInfo& mi) {
    loadModuleFactoryPlugins();

    AnyModule mod = findModuleInFactory(mi.name);
    if (mod)
      return mod;

    ModuleFactoryMap::const_iterator it = gModuleFactory.find(mi.type);
    if (it == gModuleFactory.end())
      throw std::runtime_error("module factory for module type: " + mi.type + " is not available");

    return it->second(mi);
  }

  std::vector<ModuleInfo> listModules() {
    std::vector<ModuleInfo> modules;
    std::vector<std::string> ret = qi::path::listData("qi/module", "*.mod");
    for (unsigned int i = 0; i < ret.size(); ++i)
    {
      const qi::Path p(ret.at(i));
      ModuleInfo mi;
      mi.name = p.filename().substr(0, p.filename().find(".mod"));
      boost::filesystem::ifstream is(p);
      is >> mi.type;
      modules.push_back(mi);
    }
    //look for all modules in sdk/share/qi/
    return modules;
  }

  static AnyModule loadCppModule(const ModuleInfo& moduleInfo) {
    if (moduleInfo.type != "cpp")
      throw std::runtime_error("Bad module type '" + moduleInfo.type + "' for module '" + moduleInfo.name);
    std::string path = moduleNameToPath(moduleInfo.name);
    void *mod = Application::loadModule(path);

    moduleInitFn fn = (moduleInitFn)qi::os::dlsym(mod, "qi_module_init");
    if (!fn)
      throw std::runtime_error("Module init function not found for: " + moduleInfo.name + " (path: " + moduleInfo.path + ")");

    ModuleBuilder mb(moduleInfo);

    mb.setModulePath(qi::path::findLib(path));
    fn(&mb);
    registerModuleInFactory(mb.module());
    return mb.module();
  }

}
QI_REGISTER_MODULE_FACTORY("cpp", &qi::loadCppModule);
