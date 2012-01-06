/*
** main-client.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Jan  3 13:52:00 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>

#include <cstdio>
#include <cstdlib>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#define MAX_LINE 16384

void readcb(struct bufferevent *bev,
            void *ctx)
{
  struct evbuffer *input, *output;
  char *line;
  size_t n;
  input = bufferevent_get_input(bev);
  output = bufferevent_get_output(bev);

  while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF)))
  {
    printf("%s", line);

    evbuffer_add(output, line, n);
    evbuffer_add(output, "\n", 1);
    free(line);
  }

  if (evbuffer_get_length(input) >= MAX_LINE)
  {
    /* Too long; just process what there is and go on so that the buffer
         * doesn't grow infinitely long. */
    char buf[1024];
    while (evbuffer_get_length(input))
    {
      int n = evbuffer_remove(input, buf, sizeof(buf));
      evbuffer_add(output, buf, n);
    }
    evbuffer_add(output, "\n", 1);
  }
}

void errorcb(struct bufferevent *bev,
             short error,
             void *ctx)
{
  if (error & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
  }
  else if (error & BEV_EVENT_ERROR)
  {
    // check errno to see what error occurred
  }
  else if (error & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
  }

  bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener,
               short event,
               void *arg)
{
  struct event_base *base = (struct event_base *)arg;
  struct sockaddr_storage ss;
  socklen_t slen = sizeof(ss);

  int fd = accept(listener, (struct sockaddr*)&ss, &slen);

  if (fd < 0)
  {
    perror("accept");
  }
  else if (fd > FD_SETSIZE)
  {
    close(fd);
  }
  else
  {
    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, NULL, errorcb, NULL);
    bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
  }
}

void run(void)
{
  evutil_socket_t listener;
  struct sockaddr_in sin;
  struct event_base *base;
  struct event *listener_event;

  base = event_base_new();
  if (!base)
    return; /*XXXerr*/

  // Create socket
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = 0;
  sin.sin_port = htons(12345);

  listener = socket(AF_INET, SOCK_STREAM, 0);
  evutil_make_socket_nonblocking(listener);

#ifndef WIN32
  int one = 1;
  setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#endif

  if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)
  {
    perror("bind");
    return;
  }

  if (listen(listener, 16) < 0)
  {
    perror("listen");
    return;
  }

  listener_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);
  event_add(listener_event, NULL);

  event_base_dispatch(base);
}

int main(int argc, char *argv[])
{
  setvbuf(stdout, NULL, _IONBF, 0);

  run();
  return 0;
}

