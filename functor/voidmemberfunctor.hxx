/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2007 All Rights Reserved - This file is confidential.\n
*
* Version : $Id$
*/


#ifndef  AL_MESSAGING_AL_FUNCTOR_WITHOUT_RETURN_H_
# define AL_MESSAGING_AL_FUNCTOR_WITHOUT_RETURN_H_

#include <alcommon-ng/functor/functor.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

namespace AL
{
  template <typename C>
  class MemFunctor_0 <C, ALVoid> : public Functor
  {
  public:
    MemFunctor_0(C *pObject, void (C::*pFunction)())
      : fObject(pObject),
        fFunction(pFunction)
    {}

    void operator()() {
      (fObject->*fFunction)();
    }

    void call(const AL::Messaging::CallDefinition& , AL::Messaging::ResultDefinition&) {
      (fObject->*fFunction)();
    };

  private:
    typedef void (C::*functionType) ();
    C           *fObject;
    functionType fFunction;
  };


  template <typename C, typename P1>
  class MemFunctor_1 <C, P1, ALVoid> : public Functor
  {
  public:
    MemFunctor_1(C *pObject, void (C::*pFunction) (const P1 &))
      : fObject(pObject),
        fFunction(pFunction)
    {}

    void operator()(const P1 &p1) {
      (fObject->*fFunction)(p1);
    }

    void call(const AL::Messaging::CallDefinition &pParams, AL::Messaging::ResultDefinition &) {
      //TODO: use assert?
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 1);
      (fObject->*fFunction)(pParams.getParameters()[0].as<P1>());
    };

  private:
    typedef void (C::*functionType) (const P1 &);
    C            *fObject;
    functionType  fFunction;
  };


  // 2 parameters Functor

  template <typename C, typename P1, typename P2>
  class MemFunctor_2 <C, P1, P2, ALVoid> : public Functor
  {
  public:
    MemFunctor_2(C *pObject, void (C::*pFunction) (const P1 &, const P2 &))
      : fObject(pObject),
        fFunction (pFunction)
    {}

    void operator()(const P1 &p1, const P2 &p2) {
      (fObject->*fFunction)(p1, p2);
    }

    void call(const ::AL::Messaging::CallDefinition &pParams, ::AL::Messaging::ResultDefinition &) {
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 2);
      (fObject->*fFunction)(pParams.getParameters()[0].as<P1>(), pParams.getParameters()[0].as<P2>());
    };

  private:
    typedef void (C::*functionType) (const P1 &, const P2 &);
    C            *fObject;
    functionType  fFunction;
  };

#if 0
  // 3 parameters Functor


  template <typename C, typename P1, typename P2, typename P3>
  class MemFunctor_3 <C, P1, P2, P3, ALVoid> : public Functor
  {
  public:
    MemFunctor_3(C *pObject, void (C::*pFunction) (const P1 &, const P2 &, const P3 &)) : fObject(pObject), fFunction (pFunction) {}

    inline void operator() (const P1 &p1, const P2 &p2, const P3 &p3) { (fObject->*fFunction)(p1, p2, p3);}

    void call(const CallDefinition &pParams, const ResultDefinition & /*pResult*/)
    {
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 3);
      AL::converter<P1> conv1;
      AL::converter<P2> conv2;
      AL::converter<P3> conv3;
      P1 p1 = conv1.convertFromALValue(pParams[0]);
      P2 p2 = conv2.convertFromALValue(pParams[1]);
      P3 p3 = conv3.convertFromALValue(pParams[2]);
      (fObject->*fFunction)(p1, p2, p3);
    };

  private:
    C *fObject;
    typedef void (C::*functionType) (const P1 & ,const P2 &, const P3 &);
    functionType fFunction;
  };


  // 4 parameters Functor

  template <typename C, typename P1, typename P2, typename P3, typename P4>
  class MemFunctor_4 <C, P1, P2, P3, P4, ALVoid> : public Functor
  {
  public:

    MemFunctor_4(C *pObject, void (C::*pFunction) (const P1 &, const P2&, const P3&, const P4 &)) : fObject(pObject), fFunction (pFunction) {}

    void operator() (const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) { (fObject->*fFunction)(p1, p2, p3, p4);}

    void call (const CallDefinition &pParams, const ResultDefinition &  /*pResult*/)
    {
      // Parameters checkin'
#ifdef DEBUG
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 4);
#endif
      AL::converter<P1> conv1;
      AL::converter<P2> conv2;
      AL::converter<P3> conv3;
      AL::converter<P4> conv4;
      P1 p1 = conv1.convertFromALValue(pParams[0]);
      P2 p2 = conv2.convertFromALValue(pParams[1]);
      P3 p3 = conv3.convertFromALValue(pParams[2]);
      P4 p4 = conv4.convertFromALValue(pParams[3]);
      (fObject->*fFunction)(p1, p2, p3, p4);
    };

  private:
    C *fObject;
    typedef void (C::*functionType) (const P1 &, const P2 &, const P3 &, const P4 &);
    functionType fFunction;
  };

  // 5 parameters Functor

  template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5>
  class MemFunctor_5 <C, P1, P2, P3, P4, P5, ALVoid> : public Functor
  {
  public:

    MemFunctor_5(C *pObject, void (C::*pFunction) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &)) : fObject(pObject), fFunction (pFunction) {}

    void operator() (const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
        (fObject->*fFunction)(p1, p2, p3, p4, p5);
    }

    void call (const CallDefinition &pParams, const ResultDefinition &  /*pResult*/)
    {
#ifdef DEBUG
      AL_FUNCTOR_ASSUME_NBR_PARAMS( pParams, 5 );
#endif
      AL::converter<P1> conv1;
      AL::converter<P2> conv2;
      AL::converter<P3> conv3;
      AL::converter<P4> conv4;
      AL::converter<P5> conv5;
      P1 p1 = conv1.convertFromALValue(pParams[0]);
      P2 p2 = conv2.convertFromALValue(pParams[1]);
      P3 p3 = conv3.convertFromALValue(pParams[2]);
      P4 p4 = conv4.convertFromALValue(pParams[3]);
      P5 p5 = conv5.convertFromALValue(pParams[4]);
      (fObject->*fFunction)(p1, p2, p3, p4, p5);
    };

  private:
    C *fObject;
    typedef void (C::*functionType) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &);
    functionType fFunction;
  };

  // 6 parameters Functor

  template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  class MemFunctor_6 <C, P1, P2, P3, P4, P5, P6, ALVoid> : public Functor
  {
  public:

    MemFunctor_6(C *pObject, void (C::*pFunction) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &, const P6 &)) : fObject(pObject), fFunction (pFunction) {}

    void operator() (const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
        (fObject->*fFunction)(p1, p2, p3, p4, p5, p6);
    }

    void call(const CallDefinition &pParams, const ResultDefinition &)
    {
      // Parameters checkin'
#ifdef DEBUG
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 6);
#endif
      AL::converter<P1> conv1;
      AL::converter<P2> conv2;
      AL::converter<P3> conv3;
      AL::converter<P4> conv4;
      AL::converter<P5> conv5;
      AL::converter<P6> conv6;
      P1 p1 = conv1.convertFromALValue(pParams[0]);
      P2 p2 = conv2.convertFromALValue(pParams[1]);
      P3 p3 = conv3.convertFromALValue(pParams[2]);
      P4 p4 = conv4.convertFromALValue(pParams[3]);
      P5 p5 = conv5.convertFromALValue(pParams[4]);
      P6 p6 = conv6.convertFromALValue(pParams[5]);

      (fObject->*fFunction)(p1,p2,p3,p4,p5,p6);
    };

  private:
    C *fObject;
    typedef void (C::*functionType) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &, const P6 &);
    functionType fFunction;
  };

#endif
}  // namespace
#endif
