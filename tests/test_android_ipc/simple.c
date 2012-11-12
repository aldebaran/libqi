#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "binder.h"

int main(int argc, char* argv[])
{
  struct binder_state *bs;
  void *svcmgr = BINDER_SERVICE_MANAGER;

  bs = binder_open(1024*1024);

  unsigned iodata[512/4];
  struct binder_io msg, reply;

  char s[100];
  int  ret;
  for (;;)
  {
    // If this code doesn't work any more, replace the read with scanf("%s", s)
    ret = read(0, s, sizeof(s) - 1);
    if (ret <= 1)
      continue;
    s[ret - 1] = 0;
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_string16_x(&msg, s);

    if (binder_call(bs, &msg, &reply, svcmgr, 0))
      printf("error!\n");
  }

  binder_done(bs, &msg, &reply);

  return 0;
}
