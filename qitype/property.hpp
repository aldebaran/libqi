#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_PROPERTY_HPP_
#define _QITYPE_PROPERTY_HPP_

#include <boost/function.hpp>
#include <qitype/signal.hpp>

namespace qi
{
  /// Exception that can be thrown to abort a property set from setter callback.
  class AbortUpdate: public std::exception
  {
  public:
    virtual const char* what() const throw()
    {
      return "AbortUpdate";
    }
  };

  /** Type-erased virtual interface implemented by all Property classes.
  */
  class QITYPE_API PropertyBase
  {
  public:
    virtual ~PropertyBase() {}
    virtual SignalBase* signal() = 0;
    virtual void setValue(GenericValueRef value) = 0;
    virtual GenericValue getValue() = 0;
  };

  template<typename T>
  class PropertyImpl: public Signal<void(const T&)>, public PropertyBase
  {
  public:
    typedef boost::function<T (const T&)> Setter;
    typedef boost::function<T()> Getter;
    /**
     * @param getter value getter, default to reading _value
     * @param setter value setter, what it returns will be written to
     *        _value. If it throws AbortUpdate, update operation will
     *        be silently aborted (subscribers will not be called)
    */
    PropertyImpl(Getter getter = Getter(), Setter setter = Setter());
    virtual ~PropertyImpl() {}
    T get();
    void set(const T& v);
    void operator = (const T& v) { set(v);}
  private:
    Getter _getter;
    Setter _setter;
    T      _value;
  };

  template<typename T>
  class Property: public PropertyImpl<T>
  {
  public:
    typedef PropertyImpl<T> ImplType;
    typedef typename ImplType::Getter Getter;
    typedef typename ImplType::Setter Setter;
    Property(Getter getter = Getter(), Setter setter = Setter())
    : PropertyImpl<T>(getter, setter)
    {}
    virtual SignalBase* signal() { return this;}
    virtual void setValue(GenericValueRef value)  { set(value.to<T>());}
    virtual GenericValue getValue() { return GenericValue(GenericValueRef(PropertyImpl<T>::get()));}
  };

  template<>
  class QITYPE_API Property<GenericValue>: public PropertyImpl<GenericValue>
  {
  public:
    Property(Getter getter = Getter(), Setter setter = Setter())
    : PropertyImpl<GenericValue>(getter, setter)
    {
    }
    virtual SignalBase* signal() { return this;}
    virtual void setValue(GenericValueRef value)  { set(GenericValue(value, false, false));}
    virtual GenericValue getValue() { return get();}
  };

  /// Type-erased property, simulating a typed property but using GenericValue.
  class QITYPE_API GenericProperty: public Property<GenericValue>
  {
  public:
    GenericProperty(Type* type, Getter getter = Getter(), Setter setter = Setter())
    :Property<GenericValue>(getter, setter)
    , _type(type)
    {
    }
    virtual void setValue(GenericValueRef value)  { set(GenericValue(value, false, false));}
    void set(const GenericValue& v);
  private:
    Type* _type;
  };


}

#include <qitype/details/property.hxx>

#endif
