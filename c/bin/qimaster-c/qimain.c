/*
** Author(s):
**  - Guillaume OREAL <goreal@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qic/servicedirectory.h>
#include <qic/application.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(X) Sleep(X * 1000)
#else
#include <unistd.h>
#endif

int main(int argc, char **argv)
{

  qi_application_t*    app = qi_application_create(&argc, argv);

  //if no option provided to the programm
  if (argc == 1)
  {
    printf("Usage : ./qi-master-c ip-address\n");
    printf("Assuming address is tcp://127.0.0.1:9559\n");
    char *sd_addr = strdup("tcp://127.0.0.1:9559");

    qi_servicedirectory_t *sd = qi_servicedirectory_create();
    qi_servicedirectory_listen(sd, sd_addr);

    qi_application_run(app);
    return (0);
  }
  else
  {
    char* sd_addr[argc];

    qi_servicedirectory_t *sd = qi_servicedirectory_create();

    for (int i = 0; i < argc; i++) //listen at all endpoints.
    {
      sd_addr[i] = (char *)argv[i + 1];
      if( !strncmp(sd_addr[i-1], "tcps", 4))
      {
        qi_servicedirectory_set_identity(sd, "tests/server.key", "tests/server.crt");
      }
      qi_servicedirectory_listen(sd, (char *)sd_addr[i-1]);
    }
    qi_application_run(app);

    qi_servicedirectory_destroy(sd);
    qi_application_destroy(app);

    return (0);
  }

}
