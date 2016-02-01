#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_ASSERT_HPP_
#define _QI_ASSERT_HPP_


#include <boost/predef/compiler.h>

/* TODO:
    - add assertion variants (assert_if);
    - add a way to force assert activation whatever the build mode;
    - add a way to chose assertion behaviour at compile time (not necessarilly abort);
    - add a way to force assertion behaviour whatever the platform;
*/
# if defined(NDEBUG)
#   define QI_ASSERT(expr__)
# else
#   if BOOST_COMP_MSVC
#     include <cstdlib>
#     define QI_ASSERT( expr__ ) { if( !( expr__ ) ){ __debugbreak(); ::std::abort(); } }
#   else
#     include <cassert>
#     define QI_ASSERT( expr__ ) assert( expr__ )
#   endif
# endif

# define QI_ASSERT_TRUE( expr__ ) QI_ASSERT( expr__ )
# define QI_ASSERT_FALSE( expr__ ) QI_ASSERT( !(expr__) )

#endif  // _QI_ASSERT_HPP_
