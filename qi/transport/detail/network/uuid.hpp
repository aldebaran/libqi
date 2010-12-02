#pragma once
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

#ifndef _QI_TRANSPORT_DETAIL_NETWORK_UUID_HPP_
#define _QI_TRANSPORT_DETAIL_NETWORK_UUID_HPP_

#include <string>
#ifdef _WIN32
#include <windows.h>
#include <rpc.h>
#else
#include <uuid/uuid.h>
#endif

namespace qi {
  namespace detail {

    //  This class provides RFC 4122 (a Universally Unique IDentifier)
    //  implementation.
    class uuid_t
    {
    public:
        uuid_t ();
        ~uuid_t ();

        //  The length of textual representation of UUID.
        enum { uuid_string_len = 36 };

        //  Returns a pointer to buffer containing the textual
        //  representation of the UUID. The callee is reponsible to
        //  free the allocated memory.
        const char *to_string ();

        //  The length of binary representation of UUID.
        enum { uuid_blob_len = 16 };

        const unsigned char *to_blob ();

    private:

        //  Converts one byte from hexa representation to binary.
        unsigned char convert_byte (const char *hexa_);

        //  Converts string representation of UUID into standardised BLOB.
        //  The function is endianness agnostic.
        void create_blob ();

#if defined _WIN32
        ::UUID uuid;
        RPC_CSTR string_buf;
#else
        ::uuid_t uuid;
        char string_buf [uuid_string_len + 1];
#endif
        unsigned char blob_buf [uuid_blob_len];
    };

    std::string getUUID();
  }
}

#endif  // _QI_TRANSPORT_DETAIL_NETWORK_UUID_HPP_
