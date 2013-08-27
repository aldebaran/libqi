#ifndef QICLI_ALMEMORYHELPER_HPP_
# define QICLI_ALMEMORYHELPER_HPP_

# include <string>

# include <qitype/anyobject.hpp>
# include <qimessaging/session.hpp>

struct WatchOptions;

class ALMemoryHelper
{
public:
  ALMemoryHelper(qi::Session &session);
  void post(const std::string &pattern, const std::string &arg, bool json);
  void watch(const std::vector<std::string> &patternList, bool showTime);

private:
  std::vector<std::string> getMatchingEvents(const std::string &pattern);
  std::vector<std::string> getMatchingEvents(const std::vector<std::string> &patternList);
  std::vector<std::string> getEventList();
  qi::AnyReference defaultWatcher(const WatchOptions &options, const std::vector<qi::AnyReference> &params);

private:
  std::list<qi::AnyObject> _subscriberList;
  qi::AnyObject _alm;
};

#endif /* !QICLI_ALMEMORYHELPER_HPP_ */
