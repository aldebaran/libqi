#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "binder.h"

int svcmgr_handler(struct binder_state *bs,
		   struct binder_txn *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
  uint16_t *s;
  unsigned len;

  s = bio_get_string16(msg, &len);
  for (; *s; s++)
    printf("%c", *s);
  printf("\n");

  return 0;
}

int main()
{
  struct binder_state *bs = binder_open(1024*1024);

  if (binder_become_context_manager(bs))
  {
    printf("cannot become context manager (%s)\n", strerror(errno));
    return -1;
  }

  binder_loop(bs, svcmgr_handler);
  return 0;
}
