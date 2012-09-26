#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#ifndef _QIMESSAGING_GENERICVALUESPECIALIZED_HPP_
#define _QIMESSAGING_GENERICVALUESPECIALIZED_HPP_

#include <qimessaging/genericvalue.hpp>


namespace qi
{
  class GenericIterator: public GenericValue
  {
  public:
    void operator ++();
    void operator ++(int);
    GenericValue operator *();
    bool operator ==(const GenericIterator& b) const;
    inline bool operator !=(const GenericIterator& b) const;
  };

  class GenericList: public GenericValue
  {
  public:
    GenericIterator begin();
    GenericIterator end();
    void pushBack(GenericValue val);
    Type* elementType();
  };

}

#include <qimessaging/details/genericvaluespecialized.hxx>

#endif
