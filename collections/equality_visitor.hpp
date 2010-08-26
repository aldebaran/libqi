#ifndef LIBIPPC_EQUALITY_VISITOR_HPP_
#define LIBIPPC_EQUALITY_VISITOR_HPP_

#include <boost/variant.hpp>

namespace AL {
  namespace Messaging {

    class EqualityVisitor
        : public boost::static_visitor<bool>
    {
    public:

        template <typename T, typename U>
        bool operator()( const T &, const U & ) const
        {
            return false; // cannot compare different types
        }

        template <typename T>
        bool operator()( const T & lhs, const T & rhs ) const
        {
            return lhs == rhs;
        }

    };
  }
}

#endif  // LIBIPPC_EQUALITY_VISITOR_HPP_
