#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_PROPERTY_HXX_
#define _QITYPE_PROPERTY_HXX_


namespace qi
{

  inline void GenericProperty::set(const GenericValue& v)
  {
    std::pair<GenericValuePtr, bool> conv = v.convert(_type);
    if (!conv.first.type)
      throw std::runtime_error(std::string("Failed converting ") + v.type->infoString() + " to " + _type->infoString());

    Property<GenericValue>::set(GenericValue(conv.first, false, false));
    if (conv.second)
      conv.first.destroy();
  }

  template<typename T>
  PropertyImpl<T>::PropertyImpl(Getter getter, Setter setter)
  : _getter(getter)
  , _setter(setter)
  {
  }

  template<typename T>
  T PropertyImpl<T>::get()
  {
    if (_getter)
      return _getter();
    else
      return _value;
  }
  template<typename T>
  void PropertyImpl<T>::set(const T& v)
  {
    if (_setter)
    {
      try
      { // Allow setter to abort operation by throwing AbortUpdate
        _value = _setter(v);
        (*this)(_value);
      }
      catch (const AbortUpdate& e)
      {}
    }
    else
    {
      _value = v;
      (*this)(_value);
    }
  }
}

#endif
