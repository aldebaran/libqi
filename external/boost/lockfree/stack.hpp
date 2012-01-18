//  Copyright (C) 2008, 2009, 2010 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_STACK_HPP_INCLUDED
#define BOOST_LOCKFREE_STACK_HPP_INCLUDED

#include <boost/lockfree/detail/atomic.hpp>
#include <boost/checked_delete.hpp>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <boost/lockfree/detail/tagged_ptr.hpp>
#include <boost/lockfree/detail/freelist.hpp>
#include <boost/noncopyable.hpp>


namespace boost
{

namespace lockfree
{

/** It uses a freelist for memory management, freed nodes are pushed to the freelist and not returned to
 *  the os before the stack is destroyed.
 *
 *  The memory management of the stack can be controlled via its freelist_t template argument. Two different
 *  freelists can be used. struct caching_freelist_t selects a caching freelist, which can allocate more nodes
 *  from the operating system, and struct static_freelist_t uses a fixed-sized freelist. With a fixed-sized
 *  freelist, the push operation may fail, while with a caching freelist, the push operation may block.
 *
 * */
template <typename T,
          typename freelist_t = caching_freelist_t,
          typename Alloc = std::allocator<T>
          >
class stack:
    boost::noncopyable
{
private:
#ifndef BOOST_DOXYGEN_INVOKED
    struct node
    {
        typedef tagged_ptr<node> tagged_ptr_t;

        node(T const & v):
            v(v)
        {}

        tagged_ptr_t next;
        T v;
    };
#endif

    typedef tagged_ptr<node> tagged_ptr_t;

    typedef typename Alloc::template rebind<node>::other node_allocator;

    typedef typename boost::mpl::if_<boost::is_same<freelist_t, caching_freelist_t>,
                                     detail::freelist_stack<node, true, node_allocator>,
                                     detail::freelist_stack<node, false, node_allocator>
                                     >::type pool_t;

public:
    /**
     * \return true, if implementation is lock-free.
     *
     * \warning \b Warning: It only checks, if the stack root node is lockfree. On most platforms, the whole implementation is
     *                      lockfree, if this is true. Using c++0x-style atomics, there is no possibility to provide a completely
     *                      accurate implementation, though.
     * */
    bool is_lock_free (void) const
    {
        return tos.is_lock_free();
    }

    //! Construct stack.
    stack(void):
        tos(tagged_ptr_t(NULL, 0))
    {}

    //! Construct stack, allocate n nodes for the freelist
    explicit stack(std::size_t n):
        tos(tagged_ptr_t(NULL, 0))
    {
        pool.reserve(n);
    }

    //! Allocate n nodes for freelist
    void reserve(std::size_t n)
    {
        pool.reserve(n);
    }

    /** Destroys stack, free all nodes from freelist.
     *
     *  \warning not threadsafe
     *
     * */
    ~stack(void)
    {
        if (!empty())
        {
            T dummy;
            for(;;)
            {
                if (!pop(&dummy))
                    break;
            }
        }
    }

    /** Pushes object t to the fifo. May fail, if the freelist is not able to allocate a new fifo node.
     *
     * \returns true, if the push operation is successful.
     *
     * \note Thread-safe and non-blocking
     * \warning \b Warning: May block if node needs to be allocated from the operating system
     * */
    bool push(T const & v)
    {
        node * newnode = alloc_node(v);

        if (newnode == 0)
            return false;

        tagged_ptr_t old_tos = tos.load(detail::memory_order_relaxed);

        for (;;)
        {
            tagged_ptr_t new_tos (newnode, old_tos.get_tag());
            newnode->next.set_ptr(old_tos.get_ptr());

            if (tos.compare_exchange_strong(old_tos, new_tos))
                return true;
        }
    }

    /** Pops object from stack.
     *
     * If pop operation is successful, object is written to memory location denoted by ret.
     *
     * \returns true, if the pop operation is successful, false if stack was empty.
     *
     * \note Thread-safe and non-blocking
     *
     * */
    bool pop(T * ret)
    {
        tagged_ptr_t old_tos = tos.load(detail::memory_order_consume);

        for (;;)
        {
            if (!old_tos.get_ptr())
                return false;

            node * new_tos_ptr = old_tos->next.get_ptr();
            tagged_ptr_t new_tos(new_tos_ptr, old_tos.get_tag() + 1);

            if (tos.compare_exchange_strong(old_tos, new_tos))
            {
                *ret = old_tos->v;
                dealloc_node(old_tos.get_ptr());
                return true;
            }
        }
    }

    /**
     * \return true, if stack is empty.
     *
     * \warning The state of the stack can be modified by other threads
     *
     * */
    bool empty(void) const
    {
        return tos.load().get_ptr() == NULL;
    }

private:
#ifndef BOOST_DOXYGEN_INVOKED
    node * alloc_node(T const & t)
    {
        node * chunk = pool.allocate();
        new(chunk) node(t);
        return chunk;
    }

    void dealloc_node(node * n)
    {
        n->~node();
        pool.deallocate(n);
    }

    detail::atomic<tagged_ptr_t> tos;

    static const int padding_size = BOOST_LOCKFREE_CACHELINE_BYTES - sizeof(tagged_ptr_t);
    char padding[padding_size];

    pool_t pool;
#endif
};


} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_STACK_HPP_INCLUDED */
