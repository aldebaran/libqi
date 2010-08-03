/*
 * invite_generator.hpp
 *
 *  Created on: Oct 13, 2009 at 4:51:42 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_INVITEGENERATOR_HPP_
#define LIBIPPC_INVITEGENERATOR_HPP_

#include <cstdlib>

namespace AL {
  namespace Messaging {

/**
 * @brief A random segment name generator which use /dev/urandom (under linux)
 * to initialize the seed.
 */
class InviteGenerator {
public:
  /**
   * @brief The InviteGenerator class constructor.
   */
  InviteGenerator ();

  /**
   * @brief The InviteGenerator class desctructor.
   */
  virtual ~InviteGenerator ();

  /**
   * @brief Initialize the seed using /dev/urandom under linux, time(0) under other OS.
   */
  static void init ();

  /**
   * @brief Generate a random string and store it in invite.
   * @param invite The buffer where to store the generated string.
   * @param size The size of the generated string.
   */
  static void generate (char * invite, size_t size);
};

}
}

#endif /* !LIBIPPC_INVITEGENERATOR_HPP_ */
