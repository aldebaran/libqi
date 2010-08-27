/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2007 All Rights Reserved - This file is confidential.\n
*
* Version : $Id$
*/


#ifndef AL_FUNCTOR_WITH_RETURN_H_
# define AL_FUNCTOR_WITH_RETURN_H_

#include <alcommon-ng/functor/functor.hpp>


namespace AL
{
  template <typename C, typename R = ALVoid>
  class MemFunctor_0 : public Functor
  {
  public:
    MemFunctor_0(C *pObject, R(C::*pFunction) ()) : fObject(pObject), fFunction (pFunction) {}

    R operator()() {
      return (R)(fObject->*fFunction)();
    }

    void call(const AL::Messaging::CallDefinition&, AL::Messaging::ResultDefinition& pResult)
    {
      //AL::converter<R> conv;
      //pResult = conv.convertToALValue((R)(fObject->*fFunction)());
    };

  private:
    C            *fObject;
    typedef R(C::*functionType) ();
    functionType  fFunction;
  };

  template <typename C, typename P1, typename R = ALVoid>
  class MemFunctor_1 : public Functor
  {
  public:
    MemFunctor_1(C *pObject, R(C::*pFunction) (const P1 &))
      : fObject(pObject),
        fFunction(pFunction)
    {}

    R operator()(const P1 &p1) {
      return (R)(fObject->*fFunction)(p1);
    }

    // remote case (only understand ALValue)
    virtual void call(const ::AL::Messaging::CallDefinition& pParams, ::AL::Messaging::ResultDefinition& pResult) {
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 1);
      pResult.value((fObject->*fFunction)(pParams.getParameters()[0].as<P1>()));
    };

  private:
    typedef R (C::*functionType) (const P1 &);
    C            *fObject;
    functionType  fFunction;
  };

  template <typename C, typename P1, typename P2, typename R = ALVoid>
  class MemFunctor_2 : public Functor
  {
  public:
    MemFunctor_2(C *pObject, R(C::*pFunction) (const P1 &, const P2 &))
      : fObject(pObject),
        fFunction(pFunction)
    {}

    R operator() (const P1 &p1, const P2 &p2) {
      return (R)(fObject->*fFunction)(p1, p2);
    }

    void call(const AL::Messaging::CallDefinition &pParams, AL::Messaging::ResultDefinition &pResult) {
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 2);
      pResult.value((fObject->*fFunction)(pParams.getParameters()[0].as<P1>(), pParams.getParameters()[1].as<P2>()));
    };

  private:
    typedef R(C::*functionType) (const P1 &, const P2 &);
    C            *fObject;
    functionType  fFunction;
  };

#if 0
  template <typename C, typename P1, typename P2, typename P3, typename R = ALVoid>
  class MemFunctor_3 : public Functor
  {
  public:

    MemFunctor_3(C *pObject, R(C::*pFunction) (const P1 &, const P2 &, const P3 &)) : fObject(pObject), fFunction (pFunction) {}

    R operator() (const P1 &p1, const P2 &p2, const P3 &p3) { return (R)(fObject->*fFunction)(p1, p2, p3);}

    void call(const AL::ALValue& pParams, AL::ALValue& pResult)
    {
#ifdef DEBUG
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 3);
#endif
      AL::converter<P1> conv1;
      AL::converter<P2> conv2;
      AL::converter<P3> conv3;
      P1 p1 = conv1.convertFromALValue(pParams[0]);
      P2 p2 = conv2.convertFromALValue(pParams[1]);
      P3 p3 = conv3.convertFromALValue(pParams[2]);

      pResult = (R)(fObject->*fFunction)(p1, p2, p3);
    };

  private:
    C *fObject;
    typedef R (C::*functionType) (const P1 &, const P2 &, const P3 &);
    functionType fFunction;
  };


  template <typename C, typename P1, typename P2, typename P3, typename P4, typename R = ALVoid>
  class MemFunctor_4 : public Functor
  {
  public:

    MemFunctor_4(C *pObject, R(C::*pFunction) (const P1 &, const P2 &, const P3 &, const P4 &)) : fObject(pObject), fFunction (pFunction) {}

    R operator() (const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) { return (R)(fObject->*fFunction)(p1, p2, p3, p4);}

    void call (const AL::ALValue& pParams, AL::ALValue& pResult)
    {
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

      pResult = (R)(fObject->*fFunction)(p1, p2, p3, p4);
    };

  private:
    C *fObject;
    typedef R (C::*functionType) (const P1 &, const P2 &, const P3 &, const P4 &);
    functionType fFunction;
  };

  // 5 parameters Functor

  template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename R = ALVoid>
  class MemFunctor_5 : public Functor
  {
  public:

    MemFunctor_5(C *pObject, R(C::*pFunction) (const P1 &, const  P2 &, const P3 &, const P4 &, const P5 &)) : fObject(pObject), fFunction (pFunction) {}

    R operator() (const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) { return (R)(fObject->*fFunction)(p1, p2, p3, p4, p5);}

    void call (const AL::ALValue& pParams, AL::ALValue& pResult)
    {
#ifdef DEBUG
      AL_FUNCTOR_ASSUME_NBR_PARAMS(pParams, 5);
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

      pResult = (R)(fObject->*fFunction)(p1, p2, p3, p4, p5);
    };

  private:
    C *fObject;
    typedef R(C::*functionType) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &);
    functionType fFunction;
  };


  template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename R = ALVoid>
  class MemFunctor_6 : public Functor
  {
  public:

    MemFunctor_6(C *pObject, R (C::*pFunction) (const P1 &, const  P2 &, const P3 &, const  P4 &, const  P5 &, const P6 &)) : fObject(pObject), fFunction (pFunction) {}

    R operator() (const P1  &p1, const  P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) { return (R)(fObject->*fFunction)(p1, p2, p3, p4, p5, p6);}

    void call (const AL::ALValue& pParams, AL::ALValue& pResult)
    {
      // Parameters checkin'
#ifdef DEBUG
      AL_FUNCTOR_ASSUME_NBR_PARAMS( pParams, 6 );
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

      pResult = (R)(fObject->*fFunction)(p1, p2, p3, p4, p5, p6);
    };

  private:
    C *fObject;
    typedef R(C::*functionType) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &, const P6 &);
    functionType fFunction;
  };
#endif
}  //namespace AL
#endif
