#pragma once
/*
 *  Author(s):
 *  - Laurent Lec <llec@aldebaran-robotics.com>
 *
 *  Copyright (C) 2012 Aldebaran Robotics
 */


#ifndef _QIMESSAGING_SERIALIZATION_IODEV_HPP_
#define _QIMESSAGING_SERIALIZATION_IODEV_HPP_

# include <event2/bufferevent.h>

namespace qi
{
  class Iodev
  {
    public:
      virtual int write(const void *data, size_t size) = 0;
      virtual int read(const void *data, size_t size) = 0;
  };

  class IodevLibEvent : public Iodev
  {
    private:
      struct bufferevent *_bufev;

    public:
      IodevLibEvent(struct bufferevent *bufev) :
          _bufev(bufev)
      {
      }

      int write(const void *data, size_t size)
      {
          return bufferevent_write(_bufev, data, size);
      }

      int read(const void *data, size_t size)
      {
          return bufferevent_read(_bufev, data, size);
      }
  };
}

#endif // _QI_MESSAGING_SERIALIZATION_IODEV_HPP
