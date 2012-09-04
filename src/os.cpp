/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/types.hpp>
#include <qi/error.hpp>
#include <qi/path.hpp>

namespace qi {
  namespace os {
    /* Have the static variable we need inside the function so that we
     * give correct results at initialization time, but also force
     * initialization so that timestamp 0 corresponds to pogram start time.
     */
    static int64_t _unused_base = ustime();

    int64_t ustime()
    {
      static bool initialized = false;
      static int64_t base;
      timeval tv;
      gettimeofday(&tv);
      if (!initialized)
      {
        base = tv.tv_sec*1000000LL + tv.tv_usec;
        initialized = true;
      }
      return tv.tv_sec*1000000LL + tv.tv_usec - base;
    }

    /* getMachineId will return an uuid as a string.
     * If the uuid is not created yet, it will generate it and store it
     * in machine_id file.
     * Otherwise it will read the file to retrieve the uuid and cache it
     * for further uses.
     * If the file is removed, a new file with a different uuid will be
     * created.
     */
    std::string getMachineId()
    {
      static bool initialized = false;
      static std::string idString;

      if (initialized)
        return idString;

      std::string idFilePath(qi::path::userWritableConfPath("qimessaging", "machine_id"));
      std::ifstream idFile(idFilePath.c_str());

      if (idFile)
      {
        idFile >> idString;
        idFile.close();
        initialized = true;
        return idString;
      }

      std::ofstream newIdFile(idFilePath.c_str());
      boost::uuids::uuid u = boost::uuids::random_generator()();

      idString = to_string(u);
      if (newIdFile)
      {
        newIdFile << idString;
        newIdFile.close();
        initialized = true;
      }
      else
      {
        qiLogError("getMachineId") << "Unable to create file: " << idFilePath;
      }

      return idString;
    }
  }
}
