/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <alcommon-ng/common/detail/mutexednamelookup.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

static const int gLoopCount   = 1000000;

struct Foo {
  static std::string bar() {
    return "42";
  }
};

TEST(TestMaps, std_map_string_find) {

  std::map<std::string, std::string> map;
  for (unsigned int i=0; i < gLoopCount; i++) {
    std::stringstream s;
    s << i;
    std::string key = s.str();
    map.insert(make_pair(key, key));
  }

  AL::Test::DataPerfTimer dt("Map std::string find");
  dt.start(gLoopCount);
  for (unsigned int i=0; i < gLoopCount; i++) {
    std::map<std::string, std::string>::const_iterator it;
    const std::string& s = map.find("500")->second;
  }
  dt.stop();
}

TEST(TestMaps, MutexedNameLookup_string_get) {
  AL::Common::MutexedNameLookup<std::string> mutexedMap;
  for (unsigned int i=0; i < gLoopCount; i++) {
    std::stringstream s;
    s << i;
    std::string key = s.str();
    mutexedMap.insert(key, key);
  }

  AL::Test::DataPerfTimer dt("MutexedMap std::string get");
  dt.start(gLoopCount);
  for (unsigned int i=0; i < gLoopCount; i++) {
    const std::string& s = mutexedMap.get("500");
  }
  dt.stop();
}



TEST(TestMaps, MutexedNameLookup_struct_ptr_method) {
  AL::Common::MutexedNameLookup<Foo*> mutexedMap;
  for (unsigned int i=0; i < gLoopCount; i++) {
    std::stringstream s;
    s << i;
    std::string key = s.str();
    Foo* f = new Foo();
    mutexedMap.insert(key, f);
  }

  AL::Test::DataPerfTimer dt("MutexedMap struct ptr method");
  dt.start(gLoopCount);
  for (unsigned int i=0; i < gLoopCount; i++) {
    const std::string& s = mutexedMap.get("500")->bar();
  }
  dt.stop();
}

TEST(TestMaps, MutexedNameLookup_struct_shared_ptr_method) {
  AL::Common::MutexedNameLookup<boost::shared_ptr<Foo> > mutexedMap;
  for (unsigned int i=0; i < gLoopCount; i++) {
    std::stringstream s;
    s << i;
    std::string key = s.str();
    boost::shared_ptr<Foo> f = boost::shared_ptr<Foo>(new Foo());
    mutexedMap.insert(key, f);
  }

  AL::Test::DataPerfTimer dt("MutexedMap struct shared_ptr method");
  dt.start(gLoopCount);
  for (unsigned int i=0; i < gLoopCount; i++) {
    const std::string& s = mutexedMap.get("500")->bar();
  }
  dt.stop();
}