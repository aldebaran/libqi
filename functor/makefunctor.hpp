/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2007 All Rights Reserved - This file is confidential.\n
*
* Version : $Id$
*/


#ifndef AL_MESSAGING_AL_FUNCTOR_CREATE_H_
# define AL_MESSAGING_AL_FUNCTOR_CREATE_H_

#include <iostream>
#include <alcommon-ng/functor/functor.hpp>
#include <alcommon-ng/functor/memberfunctor.hxx>
#include <alcommon-ng/functor/voidmemberfunctor.hxx>
#include <alcore/alptr.h>

namespace AL
{

  // Create Functors
// Functor manager parameter of bound functions, limitation to 8 parameters

template <typename C, typename R>
Functor *makeFunctor(C *obj, R (C::*f) ()) {
  return new MemFunctor_0<C, R>(obj, f);
}

template <typename C, typename P1, typename R>
Functor *makeFunctor(C *obj, R (C::*f) (const P1 &)) {
  return new MemFunctor_1<C, P1, R>(obj, f);
}

template <typename C, typename P1, typename P2, typename R>
Functor *makeFunctor(C *obj, R (C::*f) (const P1 &, const P2 &)) {
  return new MemFunctor_2<C, P1, P2, R>(obj, f);
}
#if 0

template <typename C, typename P1, typename P2, typename P3, typename R>
Functor *createFunctor(C *obj, R (C::*f) (const P1 &, const P2 &, const  P3 &)) {
  return new MemFunctor_3<C, P1, P2, P3, R>(obj, f);
}

template <typename C, typename P1, typename P2, typename P3, typename P4, typename R>
Functor *createFunctor(C *obj, R (C::*f) (const P1 &, const P2 &, const P3 &, const P4 &)) {
  return new MemFunctor_4<C, P1, P2, P3, P4, R>(obj, f);
}

template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename R>
Functor *createFunctor(C *obj, R (C::*f) (const P1 &, const P2 &, const P3 &, const P4 &, const P5 &)) {
  return new MemFunctor_5<C, P1, P2, P3, P4, P5, R>(obj, f);
}

template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename R>
Functor *createFunctor(C *obj, R (C::*f) (const P1 &, const P2 &, const P3  &, const P4 &, const P5 &, const P6 &)) {
  return new MemFunctor_6<C, P1, P2, P3, P4, P5, P6, R>(obj, f);
}

template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename R>
Functor *createFunctor(C *obj, R (C::*f) (const P1 &, const  P2 &, const P3 &, const P4 &, const P5 &, const P6 &, const P7&)) {
  std::cerr << "7 parameter function binding is not allowed!" << std::endl;
  return 0;
}

template <typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8,typename R>
Functor *createFunctor (C *obj, R (C::*f) (const P1 &, const  P2 &, const P3 &, const P4 &, const P5 &, const P6 &, const P7&, const P8&)) {
  std::cerr << "8 parameter function binding is not allowed!" << std::endl;
  return 0;
}
#endif

}  // namespace AL
#endif
