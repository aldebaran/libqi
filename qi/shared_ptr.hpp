#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */


#ifndef _QI_SHARED_PTR_HPP_
# define _QI_SHARED_PTR_HPP_

# include <qi/atomic.hpp>
# include <qi/log.hpp>

namespace qi
{

  /**
   * \brief Lightweight implementation of shared pointers.
   * \includename{qi/shared_ptr.hpp}
   */
  template <typename T>
  class SharedPtr
  {
  public:
    /**
     * \brief Initialization of the SharedPtr with the pointer it will manage.
     * \param ptr pointer to the managed data.
     */
    SharedPtr(T *ptr)
      : _ptr(ptr)
      , _refcount(new qi::Atomic<int>(1))
    {
    }

    /**
     * \brief Destruct the shared pointer and the pointer if current
     *        is the last one to hold the pointer.
     */
    ~SharedPtr()
    {
      if (--(*_refcount) == 0)
      {
        delete _ptr;
        delete _refcount;
      }
    }

    /**
     * \brief Copy shared pointer.
     * \param sp shared pointer also holding the pointer.
     */
    SharedPtr(const SharedPtr<T> &sp)
    {
      /*
       * Note that this line is racy.
       * If someone is deleting _refcount,
       * it cannot be used below.
       */
      if (++(*sp._refcount) != 1)
      {
        _ptr = sp._ptr;
        _refcount = sp._refcount;
      }
      else
      {

        _ptr = 0;
        _refcount = 0;
        qiLogDebug("qi.log.shared_ptr")
          << "tried to copy a shared pointer targeted for deletion"
          << std::endl;
      }
    }


    /**
     * \brief Link current SharedPtr to a new pointer. If old pointer was
     * only held by the current SharedPtr, it is freed.
     * \param sp shared pointer also holding the pointer.
     */
    SharedPtr& operator=(SharedPtr<T> &sp)
    {
      // release the current pointer
      if (--(*_refcount) == 0)
      {
        delete _ptr;
        delete _refcount;
      }
      if (++(*sp._refcount) != 1)
      {
        _ptr = sp._ptr;
        _refcount = sp._refcount;
      }
      else
      {
        qiLogDebug("qi.log.shared_ptr")
          << "tried to copy a shared pointer targeted for deletion"
          << std::endl;
      }
      return *this;
    }


    /// \brief Value accessor.
    T &operator*() const
    {
      return *_ptr;
    }

    /// \brief Pointer accessor.
    T *operator->() const
    {
      return _ptr;
    }

  private:
    T               *_ptr;
    qi::Atomic<int> *_refcount;
  };
}

#endif  // _QI_SHARED_PTR_HPP_
