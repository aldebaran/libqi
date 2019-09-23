#ifndef KA_OPT_HPP
#define KA_OPT_HPP
#pragma once
#include <algorithm>
#include <iterator>
#include <boost/optional/optional.hpp>
#include "functional.hpp"
#include "macro.hpp"
#include "macroregular.hpp"
#include "scoped.hpp"
#include "src.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

namespace ka
{
  /// Contains a value or nothing.
  ///
  /// This type is similar to `std::optional` but has two distinctive features:
  ///
  /// 1) It has a `void` specialization that strives to make generic programming
  ///    easier.
  ///
  /// 2) It is a standard sequence (containing at most one element)
  ///
  /// Due to the exceptional nature of `void` amongst C++ types, the API
  /// of this type is slightly different than the one of `std::optional` (see
  /// below).
  ///
  ///
  /// # Basic usage
  ///
  /// Example: Setting a value in a non-`void` `opt_t`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// opt_t<int> o;
  /// o.set(1);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Setting a value in a `void` `opt_t`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// opt_t<void> o;
  /// o.set();
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// In the above examples, `set` is used instead of `=` because there is no
  /// way to assign a void value (except by calling `operator=()` directly,
  /// which defeat the readability purpose of operators...).
  ///
  ///
  /// # Complex usage: generic code
  ///
  /// ## Problem
  ///
  /// In generic code, we often want to call an arbitrary procedure and put its
  /// result in an optional. The problem is the procedure might return `void`.
  /// Consider the following example:
  ///
  /// Example: Conditionally calling a procedure and returning an `opt_t`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // KO: THIS WON'T COMPILE (SEE BELOW)
  ///
  /// // Procedure<Ret (Args...)> Proc
  /// template<typename Proc, typename... Args>
  /// auto exec_conditionally(Proc proc, Args const&... args)
  ///   -> decltype(opt_t<decltype(proc(args...))>) // simplification: should decay
  /// {
  ///   opt_t<decltype(proc(args...))> opt;
  ///
  ///   // Some pre-processing...
  ///
  ///   if (/* Some condition */) {
  ///     opt.set(proc(args...)); // KO if `proc(args...)` returns `void`
  ///   }
  ///
  ///   // Some post-processing...
  ///   return opt;
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// In the above example, if the procedure returns `void` the code won't
  /// compile. This is because the following code is unfortunately illegal:
  ///
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void f(int) { // returns `void`
  ///   // ...
  /// }
  ///
  /// bool g() { // takes `void`
  ///   // ...
  /// }
  ///
  /// // On call site.
  /// g(f(1)); // KO: doesn't compile.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// ## Solution
  ///
  /// Consequently, a dedicated method is provided for this case.
  ///
  /// Example: Conditionally calling a procedure and returning an `opt_t`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // OK: THIS COMPILES
  ///
  /// // Procedure<Ret (Args...)> Proc
  /// template<typename Proc, typename... Args>
  /// auto exec_conditionally(Proc proc, Args const&... args)
  ///   -> decltype(opt_t<decltype(proc(args...))>) // simplification: should decay
  /// {
  ///   opt_t<decltype(proc(args...))> opt;
  ///
  ///   // Some pre-processing...
  ///
  ///   if (/* Some condition */) {
  ///     opt.call_set(proc, args...); // ok: handles the `void` case
  ///   }
  ///
  ///   // Some post-processing...
  ///   return opt;
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  ///
  /// # Sequence
  ///
  /// An optional can be viewed as a sequence whose maximum size is one.
  /// Consequently sequence operations that would make the size exceed one will
  /// throw an exception.
  /// `opt_t` models the standard concept `SequenceContainer` and this notably
  /// allows the use of all standard algorithms.
  ///
  /// `Container` methods include:
  /// - iterator methods: begin, end, cbegin...
  /// - size methods: empty, size...
  /// - swap
  ///
  /// `SequenceContainer` methods include:
  /// - insert methods: constructor taking ranges, insert, emplace, push_back...
  /// - erase methods: erase, clear, pop_back...
  /// - accessors: [], at, front, back...
  ///
  /// TODO: Simplify the implementation by using a `boost::static_vector` when
  /// it is stable (see https://svn.boost.org/trac10/ticket/11957 and
  /// https://github.com/boostorg/container/issues/78).
  template<typename T>
  class opt_t {
    boost::optional<T> opt;

    template<typename I>
    using EnableIfIterator = EnableIf<std::is_base_of<
      std::input_iterator_tag, typename std::iterator_traits<I>::iterator_category
    >::value>;
  public:
  // Regular:
    opt_t() = default;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(opt_t, opt)

    // TODO: remove this when get rid of VS2013.
#if KA_COMPILER_VS2013_OR_BELOW
    opt_t(opt_t const& x) : opt(x.opt) {
    }
    opt_t& operator=(opt_t const& x) {
      opt = x.opt;
      return *this;
    }
    opt_t(opt_t&& x) : opt(std::move(x.opt)) {
    }
    opt_t& operator=(opt_t&& x) {
      opt = std::move(x.opt);
      return *this;
    }
#else
    opt_t(opt_t const&) = default;
    opt_t& operator=(opt_t const&) = default;
    opt_t(opt_t&&) = default;
    opt_t& operator=(opt_t&&) = default;
#endif

#if KA_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // Readable:
    T const& operator*() const & {
      return *opt;
    }

  // Mutable:
    T& operator*() & {
      return *opt;
    }

  // ...:
    T operator*() && {
      return std::move(*opt);
    }
#else
  // Readable:
    T const& operator*() const {
      return *opt;
    }

  // Mutable:
    T& operator*() {
      return *opt;
    }

  // ...:
#endif

    template<typename U>
    opt_t& set(U&& x) {
      opt = fwd<U>(x);
      return *this;
    }

    /// Procedure<T (Args...)> Proc
    template<typename Proc, typename... Args>
    opt_t& call_set(Proc&& p, Args&&... args) {
      return set(fwd<Proc>(p)(fwd<Args>(args)...));
    }

    T const* get_ptr() const KA_NOEXCEPT_EXPR(opt.get_ptr()) {
      return opt.get_ptr();
    }

    T* get_ptr() KA_NOEXCEPT_EXPR(opt.get_ptr()) {
      return opt.get_ptr();
    }

  // ContiguousContainer:
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using iterator = T*;
    using const_iterator = T const*;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using size_type = std::size_t;

    // Default constructor, copy constructor, assignment and destruction defined
    // above.

    iterator begin() KA_NOEXCEPT(true) {
      return opt.get_ptr();
    }

    const_iterator begin() const KA_NOEXCEPT(true) {
      return const_cast<opt_t&>(*this).begin();
    }

    iterator end() KA_NOEXCEPT(true) {
      return begin() + size();
    }

    const_iterator end() const KA_NOEXCEPT(true) {
      return const_cast<opt_t&>(*this).end();
    }

    const_iterator cbegin() const KA_NOEXCEPT(true) {
      return begin();
    }

    const_iterator cend() const KA_NOEXCEPT(true) {
      return end();
    }

    // Equality and difference defined above.

    void swap(opt_t& x) KA_NOEXCEPT_EXPR(std::swap(x, x)) {
      std::swap(*this, x);
    }

    friend
    void swap(opt_t& x, opt_t& y) KA_NOEXCEPT_EXPR(x.swap(y)) {
      x.swap(y);
    }

    KA_CONSTEXPR
    size_type size() const KA_NOEXCEPT(true) {
      return empty() ? 0 : 1;
    }

    KA_CONSTEXPR
    size_type max_size() const KA_NOEXCEPT(true) {
      return 1;
    }

    KA_CONSTEXPR
    bool empty() const KA_NOEXCEPT_EXPR(!static_cast<bool>(opt)) {
      return !static_cast<bool>(opt);
    }

  // ContainerSequence:
    /// Throws: `std::length_error` if `n > 1`
    ///
    /// T constructible from U
    template<typename U>
    opt_t(size_type n, U&& u) {
      if (n == 0) {
        return;
      }
      if (n > 1) {
        throw std::length_error{"opt_t(n, u): n greater than 1."};
      }
      opt = fwd<U>(u);
    }

    /// Precondition: bounded_range(i, j)
    /// Throws: `std::length_error` if `std::distance(i, j) > 1`
    ///
    /// InputIterator<T> I
    template<typename I, typename = EnableIfInputIterator<I>>
    opt_t(I i, I const& j) {
      if (i == j) {
        return;
      }
      opt = src(i);
      ++i;
      if (! (i == j)) {
        throw std::length_error{"opt_t(i, j): range [i, j) greater than 1."};
      }
    }

    /// Throws: `std::length_error` if `l.size() > 1`
    opt_t(std::initializer_list<T> l) : opt_t(l.begin(), l.end()) {
    }

    /// Throws: `std::length_error` if `l.size() > 1`
    opt_t& operator=(std::initializer_list<T> l) {
      *this = opt_t(l);
      return *this;
    }

    /// Throws: `std::length_error` if `!empty()`
    /// Throws: `std::runtime_error` if `p != end()`
    template<typename... Args>
    iterator emplace(const_iterator p, Args&&... args) {
      if (!empty()) {
        throw std::length_error{"opt_t::emplace(p, args): opt not empty."};
      }
      if (p != end()) {
        throw std::runtime_error{"opt_t::emplace(p, args): invalid iterator."};
      }
      opt.emplace(fwd<Args>(args)...);
      return begin();
    }

    /// Throws: `std::length_error` if `!empty()`
    /// Throws: `std::runtime_error` if `p != end()`
    template<typename U>
    iterator insert(const_iterator p, U&& u) {
      return emplace(p, fwd<U>(u));
    }

    /// Throws: `std::length_error` if `n > 1 || !empty()`
    /// Throws: `std::runtime_error` if `p != end()`
    template<typename U>
    iterator insert(const_iterator p, size_type n, U&& u) {
      if (n == 0) {
        return const_cast<iterator>(p);
      }
      if (n > 1) {
        throw std::length_error{"opt_t::insert(p, n, u): n greater than 1."};
      }
      return insert(p, fwd<U>(u));
    }

    /// Precondition: bounded_range(i, j)
    /// Throws: `std::length_error` if `std::distance(i, j) > 1 || !empty()`
    /// Throws: `std::runtime_error` if `p != end()`
    ///
    /// InputIterator<T> I
    template<typename I>
    iterator insert(const_iterator p, I i, I const& j) {
      if (i == j) {
        return const_cast<iterator>(p);
      }
      bool must_erase = true;
      auto _ = scoped(insert(p, src(i)), [&](const_iterator q) {
        if (must_erase) {
          erase(q);
        }
      });
      ++i;
      if (i == j) {
        must_erase = false;
        return begin();
      }
      // Will raise an exception because too many elements.
      return insert(p, max_size() + 1, src(i));
    }

    /// Throws: `std::length_error` if `l.size() > 1 || !empty()`
    /// Throws: `std::runtime_error` if `p != end()`
    iterator insert(const_iterator p, std::initializer_list<T> l) {
      return insert(p, l.begin(), l.end());
    }

    /// Throws: `std::length_error` if `empty()`
    /// Throws: `std::runtime_error` if `q != begin()`
    iterator erase(const_iterator const& q) {
      if (empty()) {
        throw std::length_error{"opt_t::erase(q): opt empty."};
      }
      if (q != begin()) {
        throw std::runtime_error{"opt_t::erase(q): q != begin()."};
      }
      clear();
      return const_cast<iterator>(std::next(q));
    }

    /// Throws: `std::length_error` if `std::distance(q1, q2) > 1 || empty()`
    /// Throws: `std::runtime_error` if `q1 != begin()`
    iterator erase(const_iterator const& q1, const_iterator const& q2) {
      if (q1 == q2) {
        return const_cast<iterator>(q2);
      }
      if (std::next(q1) != q2) {
        throw std::length_error{"opt_t::erase(q1, q2): range [q1, q2) greater than 1."};
      }
      return erase(q1);
    }

    void clear() {
      opt = boost::none;
    }

    // VS2013 is confused with the 'iterator' assign overload.
    // TODO: Remove this when VS2013 is not supported anymore.

    /// Throws: `std::length_error` if `n > 1`
#if KA_COMPILER_VS2013_OR_BELOW
    void assign(size_type n, value_type const& u) {
#else
    template<typename U>
    void assign(size_type n, U&& u) {
#endif
      assign(&u, &u + n); // ok: only 0 or 1 is accepted.
    }

    /// Precondition: bounded_range(i, j)
    /// Throws: `std::length_error` if `std::distance(i, j) > 1`
    ///
    /// InputIterator<T> I
    template<typename I, typename = EnableIfInputIterator<I>>
    void assign(I i, I const& j) {
      if (i == j) {
        clear();
        return;
      }
      bool must_restore = true;
      auto _ = scoped(exchange(opt, src(i)), [&](boost::optional<T> const& old) {
        if (must_restore) {
          opt = old;
        }
      });
      ++i;
      if (i != j) {
        throw std::length_error{"opt_t::assign(i, j): range greater than 1."};
      }
      must_restore = false;
    }

    /// Throws: `std::length_error` if `l.size() > 1`
    void assign(std::initializer_list<T> l) {
      assign(l.begin(), l.end());
    }

    /// Precondition: !empty()
    reference front() {
      return src(begin());
    }

    /// Precondition: !empty()
    const_reference front() const {
      return src(begin());
    }

    /// Precondition: !empty()
    reference back() {
      return front();
    }

    /// Precondition: !empty()
    const_reference back() const {
      return front();
    }

    /// Throws: `std::length_error` if `!empty()`
    template<typename... Args>
    void emplace_front(Args&&... args) {
      emplace(begin(), fwd<Args>(args)...);
    }

    /// Throws: `std::length_error` if `!empty()`
    template<typename... Args>
    void emplace_back(Args&&... args) {
      emplace_front(fwd<Args>(args)...);
    }

    /// Throws: `std::length_error` if `!empty()`
    template<typename U>
    void push_front(U&& u) {
      insert(begin(), fwd<U>(u));
    }

    /// Throws: `std::length_error` if `!empty()`
    template<typename U>
    void push_back(U&& u) {
      push_front(fwd<U>(u));
    }

    /// Throws: `std::length_error` if `empty()`
    void pop_front() {
      erase(begin());
    }

    /// Throws: `std::length_error` if `empty()`
    void pop_back() {
      pop_front();
    }

    /// Precondition: n < size()
    reference operator[](size_type n) {
      return src(begin() + n);
    }

    /// Precondition: n < size()
    const_reference operator[](size_type n) const {
      return const_cast<opt_t&>(*this)[n];
    }

    /// Throws: `std::out_of_range` if `n >= size()`
    reference at(size_type n) {
      if (n >= size()) {
        throw std::out_of_range{"opt_t::at: n out of range."};
      }
      return operator[](n);
    }

    /// Throws: `std::out_of_range` if `n >= size()`
    const_reference at(size_type n) const {
      return const_cast<opt_t&>(*this).at(n);
    }
  // Functor:
    /// Function<U (T)> F
    template<typename F>
    auto fmap(F&& f) -> opt_t<CodomainFor<F, T>> {
      using U = CodomainFor<F, T>;
      if (empty()) {
        return opt_t<U>{};
      } else {
        return opt_t<U>{}.call_set(fwd<F>(f), **this);
      }
    }
  };

  /// Constructs an optional set with the given parameter.
  ///
  /// The returned optional is therefore not empty.
  ///
  /// Example: Constructing optionals from values
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto o0 = opt(1);   // `o0` has type `opt_t<int>` and is not empty.
  /// auto o1 = opt('a'); // `o1` has type `opt_t<char>` and is not empty.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// See `opt()` for constructing a `void` optional that is set.
  template<typename T>
  opt_t<Decay<T>> opt(T&& t) {
    return opt_t<Decay<T>>{}.set(fwd<T>(t));
  }

  /// This `void` specialization strives to make `void` handling easier.
  ///
  /// It cannot circumvent all the problems with `void` though. Therefore,
  /// it does not make irrelevant the alternative of using a 'unit type' as a
  /// replacement to `void`.
  ///
  /// Also note that this specialization does _not_ model the standard concept
  /// `Container`. The main reason is it's impossible to iterator over `void`
  /// "values".
  template<>
  class opt_t<void> {
    bool empty_ = true;
  public:
  // Regular:
    opt_t() = default;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(opt_t, empty_)

  // ...
    opt_t& set() KA_NOEXCEPT(true) {
      empty_ = false;
      return *this;
    }

    /// Procedure<void (Args...)> Proc
    template<typename Proc, typename... Args>
    opt_t& call_set(Proc&& p, Args&&... args) {
      fwd<Proc>(p)(fwd<Args>(args)...);
      return set();
    }

    void operator*() const {
      BOOST_ASSERT(!empty_); // Same behavior as `boost::optional`.
    }

    void const* get_ptr() const KA_NOEXCEPT(true) {
      return empty_ ? nullptr : this;
    }

    void* get_ptr() KA_NOEXCEPT(true) {
      return empty_ ? nullptr : this;
    }

  // "ProtoContainer":
    // (`void` makes impossible to model `Container`, in particular there is no
    // iteration and no dereferencement)
    using value_type = void;
    using size_type = std::size_t;

    // Default constructor, copy constructor, assignment and destruction defined
    // above.

    // Equality and difference defined above.

    void swap(opt_t& x) KA_NOEXCEPT(true) {
      std::swap(empty_, x.empty_);
    }

    friend
    void swap(opt_t& x, opt_t& y) KA_NOEXCEPT(true) {
      x.swap(y);
    }

    KA_CONSTEXPR
    size_type size() const KA_NOEXCEPT(true) {
      return empty() ? 0 : 1;
    }

    KA_CONSTEXPR
    size_type max_size() const KA_NOEXCEPT(true) {
      return 1;
    }

    KA_CONSTEXPR
    bool empty() const KA_NOEXCEPT_EXPR(true) {
      return empty_;
    }
  // Functor:
    /// Procedure<_ ()> F
    template<typename F>
    opt_t<CodomainFor<F>> fmap(F&& f) {
      return fmap_dispatch(Equal<void, CodomainFor<F>>{}, fwd<F>(f));
    }
  private:
    template<typename F>
    opt_t<void> fmap_dispatch(true_t /* VoidCodomain */, F&& f) {
      if (!empty()) {
        fwd<F>(f)();
      }
      return *this;
    }

    template<typename F>
    opt_t<CodomainFor<F>> fmap_dispatch(false_t /* VoidCodomain */, F&& f) {
      opt_t<CodomainFor<F>> o;
      if (!empty()) {
        o.call_set(fwd<F>(f));
      }
      return o;
    }
  };

  /// Constructs a `void` optional that is set.
  ///
  /// The optional is set because this function is the counterpart the version
  /// taking an argument (that effectively returns a set optional).
  ///
  /// Example: Constructing optionals (`void` and non-`void`)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto o0 = opt();     // `o0` has type `opt_t<void>` and is not empty.
  /// auto o1 = opt(true); // `o1` has type `opt_t<bool>` and is not empty.
  ///
  /// opt_t<void> o2;      // `o2` is empty.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  inline opt_t<void> opt() {
    return opt_t<void>{}.set();
  }

  namespace detail {
  // model EmptyMutable boost::optional<T>:
    template<typename T> KA_CONSTEXPR
    bool empty(boost::optional<T> const& t) {
      return !static_cast<bool>(t);
    }
  } // namespace detail

  namespace fmap_ns {
  // model FunctorApp opt:
    /// meaning(fmap(f, ka_opts...)) =
    ///     all_are_set
    ///       ? opt(f(src(ka_opts)...)) // set (void case handled)
    ///       : opt_t<U>()              // empty
    ///   where
    ///     all_are_set = !ka_opts.empty() && ...
    ///     U = typeof(f(ka_opts...))
    ///
    /// Function<U (T...)> F
    template<typename F, typename T, typename... O>
    auto fmap(F&& f, opt_t<T> const& x, O&&... o)
        -> opt_t<CodomainFor<F, T, typename Decay<O>::value_type...>> {
      // The pattern
      //  ```
      //  type res;
      //  if (condition) res = value;
      //  return res;
      //  ```
      //  sadly tends to generate more efficient code on current compilers
      //  (gcc9, clang9), even with optimizations activated, than
      //  ```
      //  return condition
      //    ? type(value)
      //    : type();
      //  ```
      //  .
      using U = CodomainFor<F, T, typename Decay<O>::value_type...>;
      opt_t<U> res;

      // TODO: Use fold expression when available.
      std::array<bool, 1 + sizeof...(O)> empties = {x.empty(), o.empty()...};
      if (std::none_of(empties.begin(), empties.end(), id_transfo_t{})) {
        res.call_set(fwd<F>(f), src(x), src(fwd<O>(o))...);
      }
      return res;
    }

  // model FunctorApp boost::optional:
    /// meaning(fmap(f, boost_opts...)) = meaning(fmap(f, as_ka_opt(boost_opts)...))
    ///   where
    ///     meaning(as_ka_opt(boost_opt)) = meaning(boost_opt)
    ///
    /// Function<U (T...)> F
    template<typename F, typename T, typename... O>
    auto fmap(F&& f, boost::optional<T> const& x, O&&... o)
        -> boost::optional<CodomainFor<F, T, typename Decay<O>::value_type...>> {
      using U = CodomainFor<F, T, typename Decay<O>::value_type...>;
      boost::optional<U> res;

      // TODO: Use fold expression when available.
      std::array<bool, 1 + sizeof...(O)> empties = {!x, (!o)...};
      if (std::none_of(empties.begin(), empties.end(), id_transfo_t{})) {
        res = boost::optional<U>(fwd<F>(f)(x.value(), fwd<O>(o).value()...));
      }
      return res;
    }
  } // namespace fmap_ns
} // namespace ka

#endif // KA_OPT_HPP
