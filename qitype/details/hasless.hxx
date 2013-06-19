/*
** hasless.hxx
** Login : <ctaf@cgestes-de.aldebaran.lan>
** Started on  Wed Jun 19 18:42:08 2013
** $Id$
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef _QITYPE_DETAILS_HASLESS_HXX_
#define _QITYPE_DETAILS_HASLESS_HXX_

#include <boost/type_traits/has_less.hpp>

namespace qi {
  namespace detail {



    // boost::has_less gives true for a vector<F> even if has_less<F> gives false
    template<typename T>
    struct HasLess
    {
      static const bool value = boost::has_less<T, T>::value;
    };

    template<typename T>
    struct HasLess<std::vector<T> >
    {
      static const bool value = boost::has_less<T, T>::value;
    };

    template<typename T>
    struct HasLess<std::list<T> >
    {
      static const bool value = boost::has_less<T, T>::value;
    };

    template<typename K, typename V>
    struct HasLess<std::map<K, V> >
    {
      static const bool value = boost::has_less<K, K>::value
      && boost::has_less<V, V>::value;
    };

    template<typename A, typename B>
    struct HasLess<std::pair<A, B> >
    {
      static const bool value = boost::has_less<A, A>::value
      && boost::has_less<B, B>::value;
    };

    //boost::has_less fails for member function pointer, gard
    template<typename T, bool v>
    struct HasLessSwitch
    {};

    template<typename T>
    struct HasLessSwitch<T, false>
    {
      static const bool value = false;
    };

    template<typename T> struct HasLessSwitch<T, true>
    {
      static const bool value = HasLess<T>::value;
    };

    template<typename T>
    struct HasLessGard
    {
      static const bool switchVal =
          boost::is_member_function_pointer<T>::value
          || boost::is_function<T>::value
          || boost::is_function<typename boost::remove_pointer<T>::type>::value
          || boost::is_member_pointer<T>::value;
      static const bool value = HasLessSwitch<T, !switchVal>::value;

    };


    template<typename T, bool b>
    struct LessHelper
    {};

    template<typename T>
    struct LessHelper<T, true>
    {
      bool operator()(T* a, T* b) { return *a<*b;}
    };

    template<typename T>
    struct LessHelper<T, false>
    {
      bool operator()(T*a, T*b) { return a<b;}
    };

    template<typename T>
    struct Less: public LessHelper<T, HasLessGard<T>::value>
    {};
  }
}

#endif  // _QITYPE_DETAILS_HASLESS_HXX_
