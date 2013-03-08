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
  Dispatcher& typeDispatch(const Dispatcher& dispatcher, GenericValuePtr value);



  // class QITYPE_API TypeDispatcher
  // {
  // public:
  //   void visitUnknown(GenericValuePtr value);
  //   void visitVoid();
  //   void visitInt(int64_t value, bool isSigned, int byteSize);
  //   void visitFloat(double value, int byteSize);
  //   void visitString(char* data, size_t size);
  //   void visitList(GenericIterator begin, GenericIterator end);
  //   void visitMap(GenericIterator begin, GenericIterator end);
  //   void visitObject(GenericObject value);
  //   void visitPointer(GenericValuePtr pointee);
  //   void visitTuple(const std::vector<GenericValuePtr>& tuple);
  //   void visitDynamic(GenericValuePtr pointee);
  //   void visitRaw(GenericValuePtr value);
  //   void visitIterator(GenericValuePtr value);
  // };

}

#include <qitype/details/typedispatcher.hxx>

#endif  // _QITYPE_TYPEDISPATCHER_HPP_
