#include <stdlib.h>
#include <stdio.h>
#include "binder.h"

int main(int argc, char* argv[])
{
  struct binder_state *bs;
  void *svcmgr = BINDER_SERVICE_MANAGER;

  bs = binder_open(1024*1024);

  unsigned iodata[512/4];
  struct binder_io msg, reply;

  char s[100];
  for (;;)
  {
    scanf("%s", s);
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_string16_x(&msg, s);

    if (binder_call(bs, &msg, &reply, svcmgr, 0))
      printf("error!\n");
  }

  binder_done(bs, &msg, &reply);

  return 0;
}
