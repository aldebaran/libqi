#include <gtest/gtest.h>
#include <qipreference/preference.hpp>

int main(int argc, char *argv[])
{
  qi::pref::PreferenceMap pm;

  pm.load(argv[1]);

  return 0;
}
