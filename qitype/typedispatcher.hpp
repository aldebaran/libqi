#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_TYPEDISPATCHER_HPP_
#define _QITYPE_TYPEDISPATCHER_HPP_

#include <qitype/type.hpp>

namespace qi {

  /** Invoke one of the visitor functions in dispatcher based on kind().
   * Dispatcher must implement TypeDispatcher.
   * Passing 0 in storage is possible, in which case visitor functions will
   * get a dummy value but correct Type informations.
   */
  template<typename Dispatcher>
  Dispatcher& typeDispatch(const Dispatcher& dispatcher, Type* type, void** storage);



  // class QITYPE_API TypeDispatcher
  // {
  // public:
  //   void visitUnknown(Type* type, void* storage);
  //   void visitVoid(Type*);
  //   void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize);
  //   void visitFloat(TypeFloat* type, double value, int byteSize);
  //   void visitString(TypeString* type, void* storage);
  //   void visitList(GenericListPtr value);
  //   void visitMap(GenericMap value);
  //   void visitObject(GenericObject value);
  //   void visitPointer(TypePointer* type, void* storage, GenericValuePtr pointee);
  //   void visitTuple(TypeTuple* type, void* storage);
  //   void visitDynamic(Type* type, GenericValuePtr pointee);
  //   void visitRaw(TypeBuffer* type, Buffer* value);
  // };

}

#include <qitype/details/typedispatcher.hxx>

#endif  // _QITYPE_TYPEDISPATCHER_HPP_
