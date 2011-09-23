#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

int main(int argc, char *argv[])
{
  char        utf8[] = {0xC5, 0xAA, 0x6E, 0xC4, 0xAD, 0x63, 0xC5, 0x8D, 0x64, 0x65, 0xCC, 0xBD, 0};
  std::string utf8xx(utf8);

  // Get the default locale
  std::local loc = boost::locale::generator().generate("");

  // Set the global locale to loc
  std::locale::global(loc);

  // Make boost.filesystem use it by default
  boost::filesystem::path::imbue(std::locale());

  // Create the path ("foo" should be utf-8)
  boost::filesystem::path path("foo");

  path /= "bar";
  path /= utf8xx;
  std::cout << "path:" << path.string() << std::endl;
  return 0;
}
