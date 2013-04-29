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
   */
  template<typename Dispatcher>
  Dispatcher& typeDispatch(const Dispatcher& dispatcher, GenericValuePtr value);


  // class QITYPE_API TypeDispatcher
  // {
  // public:
  //   void visitUnknown(qi::GenericValuePtr value);
  //   void visitVoid();
  //   void visitInt(qi::int64_t value, bool isSigned, int byteSize);
  //   void visitFloat(double value, int byteSize);
  //   void visitString(char* data, size_t size);
  //   void visitList(qi::GenericIterator begin, qi::GenericIterator end);
  //   void visitMap(qi::GenericIterator begin, qi::GenericIterator end);
  //   void visitObject(qi::GenericObject value);
  //   void visitPointer(qi::GenericValuePtr pointee);
  //   void visitTuple(const std::vector<qi::GenericValuePtr>& tuple);
  //   void visitDynamic(qi::GenericValuePtr pointee);
  //   void visitRaw(qi::GenericValuePtr value);
  //   void visitIterator(qi::GenericValuePtr value);
  //   void visitObjectPtr(qi::ObjectPtr& ptr);
  // };

}

#include <qitype/details/typedispatcher.hxx>

#endif  // _QITYPE_TYPEDISPATCHER_HPP_
