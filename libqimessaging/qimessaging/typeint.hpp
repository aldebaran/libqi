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
  virtual int64_t get(void* value) const = 0;
  virtual void set(void** storage, int64_t value) = 0;
  virtual Kind kind() const { return Int;}
};

class QIMESSAGING_API TypeFloat: public virtual Type
{
public:
  virtual double get(void* value) const = 0;
  virtual void set(void** storage, double value) = 0;
  virtual Kind kind() const { return Float;}
};


}
