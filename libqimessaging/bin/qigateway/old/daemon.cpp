#include "daemon.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef TRUE
#  define TRUE 1
#endif

#ifndef FALSE
#  define FALSE 0
#endif

static
int do_fork()
{
  switch (fork())
  {
    case 0:
      return TRUE;
    case -1:
      return FALSE;
    default:
      exit(0);
  }
}

int daemonize()
{
  if (!do_fork())
    return FALSE;

  setsid();

  if (!do_fork())
    return FALSE;

  umask(0);
  chdir("/");

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  int nullfd;

  if ((nullfd = open("/dev/null", O_RDWR)) == -1)
    return FALSE;

  dup2(nullfd, STDIN_FILENO);
  dup2(nullfd, STDOUT_FILENO);
  dup2(nullfd, STDERR_FILENO);

  return TRUE;
}
