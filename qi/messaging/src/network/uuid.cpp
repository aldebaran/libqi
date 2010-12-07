/*

Copyright (c) 2007-2010 iMatix Corporation

This file is part of 0MQ.

0MQ is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

0MQ is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Adapted for qi from zmq
TODO: use uuid from boost 1.44
*/

#include "uuid.hpp"
#include "assert.h"

namespace qi {
  namespace detail {

#if defined _WIN32
    uuid_t::uuid_t ()
    {
      RPC_STATUS ret = UuidCreate (&uuid);
      assert (ret == RPC_S_OK);
      ret = UuidToString (&uuid, &string_buf);
      assert (ret == RPC_S_OK);

      create_blob ();
    }

    uuid_t::~uuid_t ()
    {
      if (string_buf)
        RpcStringFree (&string_buf);
    }

    const char *uuid_t::to_string ()
    {
      return (char*) string_buf;
    }

#else

#include <uuid/uuid.h>

    uuid_t::uuid_t ()
    {
      uuid_generate (uuid);
      uuid_unparse (uuid, string_buf);

      create_blob ();
    }

    uuid_t::~uuid_t ()
    {
    }

    const char *uuid_t::to_string ()
    {
      return string_buf;
    }

#endif

    const unsigned char *uuid_t::to_blob ()
    {
      return blob_buf;
    }

    unsigned char uuid_t::convert_byte (const char *hexa_)
    {
      unsigned char byte;

      if (*hexa_ >= '0' && *hexa_ <= '9')
        byte = *hexa_ - '0';
      else if (*hexa_ >= 'A' && *hexa_ <= 'F')
        byte = *hexa_ - 'A' + 10;
      else if (*hexa_ >= 'a' && *hexa_ <= 'f')
        byte = *hexa_ - 'a' + 10;
      else {
        byte = 0;
      }

      byte *= 16;

      hexa_++;
      if (*hexa_ >= '0' && *hexa_ <= '9')
        byte += *hexa_ - '0';
      else if (*hexa_ >= 'A' && *hexa_ <= 'F')
        byte += *hexa_ - 'A' + 10;
      else if (*hexa_ >= 'a' && *hexa_ <= 'f')
        byte += *hexa_ - 'a' + 10;


      return byte;
    }

    void uuid_t::create_blob ()
    {
      const char *buf = (const char*) string_buf;

      blob_buf [0] = convert_byte (buf + 0);
      blob_buf [1] = convert_byte (buf + 2);
      blob_buf [2] = convert_byte (buf + 4);
      blob_buf [3] = convert_byte (buf + 6);

      blob_buf [4] = convert_byte (buf + 9);
      blob_buf [5] = convert_byte (buf + 11);

      blob_buf [6] = convert_byte (buf + 14);
      blob_buf [7] = convert_byte (buf + 16);

      blob_buf [8] = convert_byte (buf + 19);
      blob_buf [9] = convert_byte (buf + 21);

      blob_buf [10] = convert_byte (buf + 24);
      blob_buf [11] = convert_byte (buf + 26);
      blob_buf [12] = convert_byte (buf + 28);
      blob_buf [13] = convert_byte (buf + 30);
      blob_buf [14] = convert_byte (buf + 32);
      blob_buf [15] = convert_byte (buf + 34);
    }



    std::string getUUID() {
      uuid_t u;
      return std::string(u.to_string());
    }

  }
}
