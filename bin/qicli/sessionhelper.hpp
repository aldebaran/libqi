#ifndef QICLI_SESSIONHELPER_HPP_
# define QICLI_SESSIONHELPER_HPP_

# include <qimessaging/session.hpp>

# include "servicehelper.hpp"

class SessionHelper
{
private:
  typedef std::list<std::string> (ServiceHelper::*ShPatternResolver)(const std::string &memberPattern, bool getHidden) const;
  typedef boost::function<bool (ServiceHelper& sh, const std::string& memberName)> ShMethod;
  typedef std::map<std::string, std::pair<ServiceHelper, std::set<std::string> > > MatchMap;
  typedef std::pair<const std::string, std::pair<ServiceHelper, std::set<std::string> > > MatchMapPair;

public:
  SessionHelper(const std::string &address);
  ~SessionHelper();

public:
  void info(const std::vector<std::string> &patternVec, bool verbose, bool showHidden, bool showDoc);
  void call(const std::string &pattern, const std::vector<std::string> &jsonArgList, bool hidden, bool json, bool cont);
  void post(const std::string &pattern, const std::vector<std::string> &jsonArgList, bool hidden, bool json);
  void postOnAlmemory(const std::string &pattern, const std::string &arg, bool json);
  void get(const std::vector<std::string> &patternList, bool hidden, bool cont);
  void set(const std::vector<std::string> &patternList, const std::string &jsonValue, bool hidden, bool json, bool cont);
  void watch(const std::vector<std::string> &patternList, bool showTime, bool hidden, bool cont);
  void watchAlmemory(const std::vector<std::string> &patternList, bool showTime);

private:
  bool                          byPassService(const std::string &name, bool showHidden);
  MatchMap                      getMatchMap(const std::vector<std::string> &patternList, ShPatternResolver patternResolver, bool hidden);
  void                          forEachService(const std::string &pattern, ShMethod methodToCall, ShPatternResolver patternResolver, bool hidden, bool cont);
  void                          forEachService(const std::vector<std::string> &patternList, ShMethod methodToCall, ShPatternResolver patternResolver, bool hidden, bool cont);
  ServiceHelper                 getServiceHelper(const std::string &serviceName);
  std::list<std::string>        getMatchingServices(const std::string &patternList, bool getHidden);
  void                          showServiceInfo(const qi::ServiceInfo &infos, bool verbose, bool showHidden, bool showDoc);
  bool                          splitName(const std::string &fullName, std::string &beforePoint, std::string &afterPoint, bool throwOnError);

private:
  qi::Session                   _session;
  MatchMap                      _currentMatchMap;
  std::vector<qi::ServiceInfo>  _servicesInfos;
};

#endif /* !QICLI_SESSIONHELPER_HPP_ */
