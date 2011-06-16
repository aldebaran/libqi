/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#ifndef   	NONCOPYABLE_HPP_
# define   	NONCOPYABLE_HPP_

namespace qi {

  class noncopyable
  {
  protected:
    //object not directly instantiable
    noncopyable()  {}
    ~noncopyable() {}

  private:
    //private copy
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
  };
}

#endif	    /* !NONCOPYABLE_PP_ */
