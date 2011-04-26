/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "binder.h"

void *svcmgr_lookup(struct binder_state *bs, void *target, const char *name)
{
    void *ptr;
    int i;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);


    //for (i = 0; i < 100000; ++i) {
      if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE)) {
        printf("error ouille %d\n", i);
        return 0;
      }
      //}  //

    ptr = bio_get_ref(&reply);

    if (ptr)
        binder_acquire(bs, ptr);

    binder_done(bs, &msg, &reply);

    return ptr;
}

int svcmgr_publish(struct binder_state *bs, void *target, const char *name, void *ptr)
{
    unsigned status;
    unsigned iodata[512/4];
    int i;
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);

    //for (i = 0; i < 100000; ++i) {
      //printf("i %d\n", i);
      if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE)) {
        printf("error man\n");
        return -1;
      }
      //}
    printf("done 100000\n");
    status = bio_get_uint32(&reply);

    binder_done(bs, &msg, &reply);

    return status;
}

unsigned token;

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    void *svcmgr = BINDER_SERVICE_MANAGER;

    bs = binder_open(1024*1024);

    argc--;
    argv++;
    while (argc > 0) {
        if (!strcmp(argv[0],"alt")) {
            void *ptr = svcmgr_lookup(bs, svcmgr, "alt_svc_mgr");
            if (!ptr) {
                fprintf(stderr,"cannot find alt_svc_mgr\n");
                return -1;
            }
            svcmgr = ptr;
            fprintf(stderr,"svcmgr is via %p\n", ptr);
        } else if (!strcmp(argv[0],"lookup")) {
            void *ptr;
            if (argc < 2) {
                fprintf(stderr,"argument required\n");
                return -1;
            }
            int i = 0;
            for (i = 0; i < 100000; ++i) {
              ptr = svcmgr_lookup(bs, svcmgr, argv[1]);
            }
            fprintf(stderr,"lookup(%s) = %p\n", argv[1], ptr);
            argc--;
            argv++;
        } else if (!strcmp(argv[0],"publish")) {
            if (argc < 2) {
                fprintf(stderr,"argument required\n");
                return -1;
            }
            svcmgr_publish(bs, svcmgr, argv[1], &token);
            argc--;
            argv++;
        } else {
            fprintf(stderr,"unknown command %s\n", argv[0]);
            return -1;
        }
        argc--;
        argv++;
    }
    return 0;
}
