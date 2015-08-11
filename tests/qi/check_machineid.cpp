/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/filesystem/fstream.hpp>
#include <string>
#include <qi/os.hpp>


int main()
{
  const qi::Path saveUuidFileName = (qi::os::tmp()).append("machine_id_test_42");
  boost::filesystem::ofstream saveUuidFile(saveUuidFileName);
  const std::string uuid = qi::os::getMachineId();

  if (!saveUuidFile)
    return 1;

  saveUuidFile << uuid;
  saveUuidFile.close();

  return 0;
}
