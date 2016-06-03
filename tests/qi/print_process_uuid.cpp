#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <qi/os.hpp>

/// @file Helper to check that process uuids are different in different processes.
///
/// see test Os.getProcessUuidDifferentInDifferentProcesses

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <output_file>\n\n"
                 "Write the current process uuid in the given file.\n";
    return 1;
  }

  boost::filesystem::ofstream file{argv[1], std::ios_base::out};
  file << qi::os::getProcessUuid();
  file.close();
  return 0;
}
