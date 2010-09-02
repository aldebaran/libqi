/*
 * invite_generator.cpp
 *
 *  Created on: Oct 13, 2009 at 4:51:43 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/transport/shm/util/invite_generator.hpp>

#include <ctime>
#include <cstdlib>

#ifndef WIN32
# include <cstdio>
#else
#include <windows.h>
#endif

namespace AL {
  namespace Transport {

InviteGenerator::InviteGenerator () {
}

InviteGenerator::~InviteGenerator () {
}

void InviteGenerator::init () {
#ifndef WIN32
  int val;
  FILE * fd = std::fopen("/dev/urandom", "r");
  size_t r = std::fread(&val, sizeof(int), 1, fd);
  (void) r;
  std::fclose(fd);
  srand(val);
#else
    SYSTEMTIME localTime;
    GetLocalTime( &localTime );
    srand((unsigned int)time(0)+localTime.wMilliseconds);
#endif
}

void InviteGenerator::generate (char * invite, size_t size) {
  static int counter = rand();
  const char * chars = "abcdefghijklmnopqrstuvwxyz"
	  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	  "0123456789_";

//	unsigned int max = std::strlen(chars);
	unsigned int max = 63;
  counter++;
	for (unsigned int i = 0; i < size - 2; ++i ) {
	  invite[i] = chars[(rand()+counter) % max];
	}

	invite[size - 1] = 0;
}

}
}
