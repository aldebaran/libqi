/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_OBJECT_HPP_
#define _QIMESSAGING_OBJECT_HPP_

#include <map>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/details/makefunctor.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/metaevent.hpp>
#include <qimessaging/metamethod.hpp>

namespace qi {

  class MetaObjectPrivate;
  class QIMESSAGING_API MetaObject {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    int methodId(const std::string &name);
    int eventId(const std::string &name);

    typedef std::map<unsigned int, MetaMethod> MethodMap;
    MethodMap methods() const;

    typedef std::map<unsigned int, MetaEvent> EventMap;
    EventMap events() const;

    MetaMethod *method(unsigned int id);
    const MetaMethod *method(unsigned int id) const;

    MetaEvent *event(unsigned int id);
    const MetaEvent *event(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaEvent> findEvent(const std::string &name);

    MetaObjectPrivate   *_p;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaObject &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaObject &meta);

  class QIMESSAGING_API ObjectInterface {
  public:
    virtual ~ObjectInterface() = 0;
    virtual void onObjectDestroyed(Object *object, void *data) = 0;
  };

  class ObjectPrivate;
  class QIMESSAGING_API Object {
  public:
    Object();
    virtual ~Object();

    enum MetaCallType {
      MetaCallType_Auto   = 0,
      MetaCallType_Direct = 1,
      MetaCallType_Queued = 2,
    };

    void addCallbacks(ObjectInterface *callbacks, void *data = 0);
    void removeCallbacks(ObjectInterface *callbacks);

    MetaObject &metaObject();

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function);


    template <typename RETURN_TYPE>
    qi::Future<RETURN_TYPE> call(const std::string& methodName);
    template <typename RETURN_TYPE, typename P0>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0);
    template <typename RETURN_TYPE, typename P0, typename P1>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8);

    virtual void metaCall(unsigned int method, const FunctorParameters &in, FunctorResult out, MetaCallType callType = MetaCallType_Auto);

    template<typename FUNCTION_TYPE>
    inline unsigned int advertiseEvent(const std::string& eventName);
    template<typename T>
    inline unsigned int advertiseMethod(const std::string& name,
      boost::function<T> func);
    int xForgetMethod(const std::string &meth);

    inline
    void emitEvent(const std::string& eventName);
    template <typename P0>
    void emitEvent(const std::string& eventName, const P0 &p0);
    template <typename P0, typename P1>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1);
    template <typename P0, typename P1, typename P2>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2);
    template <typename P0, typename P1, typename P2, typename P3>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3);
    template <typename P0, typename P1, typename P2, typename P3, typename P4>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4);
    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5);
    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6);
    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7);
    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    void emitEvent(const std::string& eventName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8);

    virtual void metaEmit(unsigned int event, const FunctorParameters &args);

    /** Connect an event to an arbitrary callback.
     *
     * If you are within a service, it is recommended that you connect the
     * event to one of your Slots instead of using this method.
     */
    template <typename FUNCTOR_TYPE>
    unsigned int connect(const std::string& eventName, FUNCTOR_TYPE callback);
    unsigned int xConnect(const std::string &signature, const Functor* functor);
    unsigned int connect(unsigned int event, const Functor* Functor);

    /// Calls given functor when event is fired. Takes ownership of functor.
    virtual unsigned int connect(unsigned int event,
      const MetaEvent::Subscriber& subscriber);

    /// Disconnect an event link. Returns if disconnection was successful.
    virtual bool disconnect(unsigned int uid);

    int xAdvertiseMethod(const std::string &retsig, const std::string& signature, const Functor *functor);
    /// Resolve the method Id and bounces to metaCall
    void xMetaCall(const std::string &retsig, const std::string &signature, const FunctorParameters &in, FunctorResult out);
    int xAdvertiseEvent(const std::string& signature);
    //// Resolve and bounce to metaEmit
    bool xMetaEmit(const std::string &signature, const FunctorParameters &in);

    /** Connect an event to a method.
     * Recommended use is when target is not a proxy.
     * If target is a proxy and this is server-side, the event will be
     *    registered localy and the call will be forwarded.
     * If target and this are proxies, the message will be routed through
     * the current process.
     */
    unsigned int connect(unsigned int signal, qi::Object* target, unsigned int slot);
    /** Same as connect(signal, target, slot) but with reverse signature,
     * so that we can advertise this method.
     *
     */
  protected:
    /// Trigger event handlers
    void trigger(unsigned int event, const FunctorParameters &in);
  protected:
    boost::shared_ptr<ObjectPrivate> _p;
  };

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int Object::advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
  {
    std::string signature(name);
    std::string sigret;
    signature += "::(";
    typedef typename boost::function_types::parameter_types<METHOD_TYPE>::type MemArgsType;
    typedef typename boost::mpl::pop_front< MemArgsType >::type                ArgsType;

    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    typedef typename boost::function_types::result_type<METHOD_TYPE>::type     ResultType;
    signatureFromType<ResultType>::value(sigret);
    return xAdvertiseMethod(sigret, signature, makeFunctor(object, method));
  }

  template <typename FUNCTION_TYPE>
  inline unsigned int Object::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    std::string signature(name);
    std::string sigret;
    signature += "::(";
    typedef typename boost::function_types::parameter_types<FUNCTION_TYPE>::type ArgsType;
    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    typedef typename boost::function_types::result_type<FUNCTION_TYPE>::type     ResultType;
    signatureFromType<ResultType>::value(sigret);
    return xAdvertiseMethod(sigret, signature, makeFunctor(function));
  }

  template<typename T>
  inline unsigned int Object::advertiseMethod(const std::string& name,
    boost::function<T> function)
  {
    std::string signature(name);
    std::string sigret;
    signature += "::(";
    typedef typename boost::function_types::parameter_types<T>::type ArgsType;
    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    typedef typename boost::function_types::result_type<T>::type ResultType;
    signatureFromType<ResultType>::value(sigret);
    return xAdvertiseMethod(sigret, signature, makeFunctor(function));
  }

  template<typename FUNCTION_TYPE>
  inline unsigned int Object::advertiseEvent(const std::string& eventName)
  {
    std::string signature(eventName);
    signature += "::(";
    typedef typename boost::function_types::parameter_types<FUNCTION_TYPE>::type ArgsType;
    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    return xAdvertiseEvent(signature);
  }

  template <typename FUNCTION_TYPE>
  unsigned int Object::connect(const std::string& eventName, FUNCTION_TYPE callback)
  {
    std::string signature(eventName);
    signature += "::(";
    typedef typename boost::function_types::parameter_types<FUNCTION_TYPE>::type ArgsType;
    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    return xConnect(signature, makeFunctor(callback));
  }

};

/** Register struct with QI binding system.
 * Once called, your structure can be passed as argument to call(), and method
 * using it can be bound with advertiseMethod().
 * Usage: pass the name of the structure, and the list of members.
 */
#define QI_REGISTER_STRUCT(Cname, ...)        \
  QI_DATASTREAM_STRUCT(Cname, __VA_ARGS__)    \
  QI_SIGNATURE_STRUCT(Cname, __VA_ARGS__)

/// Only declare required functions for class registration
#define QI_REGISTER_STRUCT_DECLARE(Cname)    \
  QI_DATASTREAM_STRUCT_DECLARE(Cname)        \
  QI_SIGNATURE_STRUCT_DECLARE(Cname)

/// Implement functions required for class registration
#define QI_REGISTER_STRUCT_IMPLEMENT(Cname, ...) \
  __QI_DATASTREAM_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)    \
  __QI_SIGNATURE_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)

#include <qimessaging/object.hxx>
#endif  // _QIMESSAGING_OBJECT_HPP_
