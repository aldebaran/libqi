/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/log.h>
#include <stdio.h>


int main(int argc, char **argv)
{
  qi_debug("toto %d %s\n", 42, "titi");
  qi_info("it's interessting\n");
  qi_warning("ouch\n");
  qi_error("ouch ouch\n");
  qi_fatal("fail\n");
}

