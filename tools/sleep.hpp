/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef    AL_MESSAGING_SLEEP_HXX_
# define   AL_MESSAGING_SLEEP_HXX_

#ifdef _WIN32
#  include <winsock2.h>
#  define sleep(x) Sleep(x * 1000)
#else
#  include <unistd.h>
#endif

#endif    /* !AL_MESSAGING_SLEEP_HXX_ */
