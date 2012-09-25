/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/type.hpp>

namespace qi
{

class QIMESSAGING_API TypeInt: public virtual Type
{
public:
  virtual int64_t get(void* value) = 0;
  virtual Kind kind() const { return Int;}
};


}
