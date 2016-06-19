#ifndef QICQ_H
#define QICQ_H

#include <algorithm>
#include <boost/any.hpp>
#include <boost/hana.hpp>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>
#include <utility>

#include <qicq/qicq_fun.h>

namespace qicq {
  template <class...  T>
  struct tuple: boost::hana::tuple<T...> {
    using boost::hana::tuple<T...>::tuple;
    template <class N> auto operator[](N&& n)       = delete;
    template <class N> auto operator[](N&& n) const = delete;
    template <class N>
    auto operator()(N&& n) {
      return boost::hana::at(*this,std::forward<N>(n));
    }
    template <class N>
    auto operator()(N&& n) const {
      return boost::hana::at(*this,std::forward<N>(n));
    }
  };
  
  template <class K, class V> struct dict;
  
  template <class T>
  struct vec {
    typedef typename std::vector<T>::value_type      value_type;
    typedef typename std::vector<T>::iterator        iterator;
    typedef typename std::vector<T>::const_iterator  const_iterator;
    typedef typename std::vector<T>::reference       reference;
    typedef typename std::vector<T>::const_reference const_reference;
    typedef typename std::vector<T>::size_type       size_type;

    vec() = default;
    vec(std::initializer_list<T> t): v(t) {}
    explicit vec(size_type n): v(n) {}
    explicit vec(const std::string& s): v(std::begin(s), std::end(s)) {}
    vec(const char* s): v(s, s+strlen(s)) {
      static_assert(std::is_same<T,char>::value,
                    "vec(const char*) is only valid for vec<char>");
    }
    vec(size_type n, const T& t): v(n, t) {}
    template <class I,
      std::enable_if_t<!std::is_same<size_t, I>::value>* = nullptr>
    vec(I first, I last): v(first, last) {}

    operator        std::vector<T>&()       { return v; }
    operator const  std::vector<T>&() const { return v; }

    iterator insert(iterator p, const T& t) { return v.insert(p, t); }
    void push_back(const T& t) { v.push_back(t); }
    void reserve(size_type n) { v.reserve(n); }

    size_type       size () const { return v.size(); }
    bool            empty() const { return v.empty(); }
    iterator        begin()       { return v.begin(); }
    const_iterator  begin() const { return v.begin(); }
    iterator        end  ()       { return v.end(); }
    const_iterator  end  () const { return v.end(); }
    reference       front()       { return v.front(); }
    const_reference front() const { return v.front(); }
    reference       back ()       { return v.back(); }
    const_reference back () const { return v.back(); }
    //    reference       operator[](size_type i)       { return v[i]; }
    //    const_reference operator[](size_type i) const { return v[i]; }
    
    reference       operator()(size_type i)       { return v[i]; }
    const_reference operator()(size_type i) const { return v[i]; }
    // TODO non-const vec indexing operator so we can say
    // x(v(1,3,5,7)) = "qicq"; // modify x in place
    template <class I>
    auto operator()(const vec<I>& i) const;
    template <class K, class V>
    auto operator()(const dict<K,V>& i) const;
    template <class I, class J,
      std::enable_if_t<std::is_integral<I>::value>* = nullptr>
    auto operator()(const I& i, const J& j) const {
      return (*this)(i)(j);
    }
    template <class I, class J,
      std::enable_if_t<!std::is_integral<I>::value>* = nullptr>
    auto operator()(const I& i, const J& j) const {
      auto t((*this)(i));
      vec<std::decay_t<decltype((*std::begin(t))(j))>> r(t.size());
      std::transform(std::begin(t), std::end(t), std::begin(r),
                     [&](auto&& u){return u(j);});
      return r;
    }

    template <class C>
    vec<T>& sort(C c) { std::stable_sort(begin(), end(), c); return *this; }

  private:
    std::vector<T> v;
  };

  template <>
  struct vec<bool> {
    typedef bool        value_type;
    typedef bool*       iterator;
    typedef const bool* const_iterator;
    typedef bool&       reference;
    typedef const bool& const_reference;
    typedef size_t      size_type;

    vec() = default;
    explicit vec(size_type n): v(n) {}
    vec(size_type n, bool b): v(n, b) {}
    vec(std::initializer_list<bool> b): v(b.begin(), b.end()) {}
    template <class I,
      std::enable_if_t<!std::is_same<size_t, I>::value>* = nullptr>
    vec(I first, I last): v(first, last) {}

    iterator insert(iterator p, bool t) {
      auto i = v.insert(v.begin() + (p-begin()), t);
      return begin() + (i - v.begin());
    }
    void push_back(bool t) { v.push_back(t); }
    void reserve(size_type n) { v.reserve(n); }

    size_type       size () const { return v.size(); }
    bool            empty() const { return v.empty(); }
    
    iterator        begin()       { return (bool*)v.data(); }
    const_iterator  begin() const { return (const bool*)v.data(); }
    iterator        end  ()       { return begin()+size(); }
    const_iterator  end  () const { return begin()+size(); }
    reference       front()       { return *begin(); }
    const_reference front() const { return *begin(); }
    reference       back ()       { return *(begin()+size()-1); }
    const_reference back () const { return *(begin()+size()-1); }

    reference       operator()(size_type i)       { return *(begin()+i); }
    const_reference operator()(size_type i) const { return *(begin()+i); }
    template <class I>
    auto operator()(const vec<I>& i) const {
      vec<bool> r(i.size());
      std::transform(std::begin(i), std::end(i), std::begin(r), *this);
      return r;
    }
    
    template <class C>
    vec<bool>& sort(C c) { std::stable_sort(begin(), end(), c); return *this; }

  private:
    std::vector<char> v;
  };
  // TODO distinguish 0_b and 1_b as atoms from 00_b etc as vecs
  inline vec<bool> operator""_b(const char* s) {
    const int n = strlen(s);
    vec<bool> r(n);
    std::transform(s, s+n, std::begin(r), [](char c){return '0'!=c;});
    return r;
  }

  template <class K, class V>
  struct dict {
    typedef typename vec<K>::value_type      key_type;
    typedef typename vec<V>::value_type      value_type;
    typedef typename vec<V>::iterator        iterator;
    typedef typename vec<V>::const_iterator  const_iterator;
    typedef typename vec<V>::reference       reference;
    typedef typename vec<V>::const_reference const_reference;

    dict() = default;
    explicit dict(const vec<K>& k_): k(k_), v(k_.size()) {}
    dict(const vec<K>& k_, const vec<V>& v_): k(k_), v(v_) {}
    dict(vec<K>&& k_, vec<V>&& v_): k(k_), v(v_) {}
    
    bool empty() const { return 0 == size(); }
    size_t size() const { return k.size(); }
    bool has(const K& k_) const {
      return k.end() != std::find(k.begin(), k.end(), k_);
    }
    
    iterator        begin()       { return v.begin(); }
    const_iterator  begin() const { return v.begin(); }
    iterator        end  ()       { return v.end  (); }
    const_iterator  end  () const { return v.end  (); }
    reference       front()       { return v.front(); }
    const_reference front() const { return v.front(); }
    reference       back ()       { return v.back (); }
    const_reference back () const { return v.back (); }
    
    const vec<K>& key() const { return k; }
    const vec<V>& val() const { return v; }
    reference operator()(const K& k_) {
      auto i = std::find(std::begin(k), std::end(k), k_);
      if (std::end(k) != i)
        return *(std::begin(v) + (i-std::begin(k)));
      k.push_back(k_);
      v.push_back(V());
      return v.back();
    }
    const_reference operator()(const K& k_) const {
      // TODO: handle not found?
      return v(std::find(std::begin(k), std::end(k), k_) - std::begin(k));
    }
    template <class I>
    auto operator()(const vec<I>& i) const;
    template <class L, class U>
    auto operator()(const dict<L,U>& i) const;
    template <class I, class J,
      std::enable_if_t<std::is_convertible<I,K>::value>* = nullptr>
    auto operator()(const I& i, const J& j) const {
      return (*this)(i)(j);
    }
    template <class I, class J,
      std::enable_if_t<!std::is_convertible<I,K>::value>* = nullptr>
    auto operator()(const I& i, const J& j) const {
      auto t((*this)(i));
      vec<std::decay_t<decltype((*std::begin(t))(j))>> r(t.size());
      std::transform(std::begin(t), std::end(t), std::begin(r),
                     [&](auto&& u){return u(j);});
      return r;
    }

  private:
    vec<K> k;
    vec<V> v;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Traits
  //////////////////////////////////////////////////////////////////////////////
  template <class>   struct is_vec        : std::false_type {};
  template <class T> struct is_vec<vec<T>>: std::true_type  {};
  template <class T> constexpr bool is_vec_v = is_vec<T>::value;

  template <class,class=void> struct is_dict: std::false_type {};
  template <class T>
  struct is_dict<T, std::enable_if_t<std::is_same<T,
      dict<typename T::key_type,typename T::value_type>>::value>>:
    std::true_type
  {};
  template <class T> constexpr bool is_dict_v = is_dict<T>::value;

  //////////////////////////////////////////////////////////////////////////////
  // Stream output
  //////////////////////////////////////////////////////////////////////////////
  template <class... T>
  std::ostream& operator<<(std::ostream& os, const tuple<T...>& t) {
    boost::hana::for_each(t, [&](auto&& x){os << x << '\n';});
    return os;
  }
  
  template <class T, T N>
  std::ostream& operator<<(std::ostream& os,
                           const boost::hana::integral_constant<T,N>& t) {
    return os << static_cast<T>(t);
  }
  
  template <class T, std::enable_if_t<!is_vec_v<T>>* = nullptr>
  std::ostream& operator<<(std::ostream& os, const vec<T>& v) {
    if (v.empty())
      return os;
    else if (1 == v.size())
      return os << ',' << v.back();
    else {
      std::copy(std::begin(v), std::end(v)-1,
                std::ostream_iterator<T>(os, " "));
      return os << v.back();
    }
  }

  inline
  std::ostream& operator<<(std::ostream& os, const vec<bool>& v) {
    if (v.empty())
      return os;
    else if (1 == v.size())
      return os << ',' << v.back() << 'b';
    else {
      std::copy(std::begin(v), std::end(v),
                std::ostream_iterator<bool>(os, ""));
      return os << 'b';
    }
  }

  inline
  std::ostream& operator<<(std::ostream& os, const vec<char>& v) {
    if (1 == v.size())
      os << ",\"" << v.back();
    else {
      os << '\"';
      std::copy(std::begin(v), std::end(v),
                std::ostream_iterator<char>(os, ""));
    }
    return os << '\"';
  }

  template <class T, std::enable_if_t<is_vec_v<T>>* = nullptr>
  std::ostream& operator<<(std::ostream& os, const vec<T>& v) {
    if (v.empty())
      return os;
    else if (1 == v.size())
      return os << ',' << v.back();
    else {
      std::copy(std::begin(v), std::end(v)-1,
                std::ostream_iterator<T>(os, "\n"));
      return os << v.back();
    }
  }

  template <class K, class V>
  std::ostream& operator<<(std::ostream& os, const dict<K,V>& d) {
    if (d.empty())
      return os;
    else {
      auto max_width = [](const vec<std::string>& x){
        vec<size_t> w(x.size());
        std::transform(std::begin(x), std::end(x), std::begin(w),
                       [](const std::string& s){return s.size();});
        return *std::max_element(std::begin(w), std::end(w));
      };

      vec<std::string> k(d.size()), v(d.size());
      std::transform(std::begin(d.key()), std::end(d.key()), std::begin(k),
                     [](const K& x){
                       std::ostringstream os; os<<x; return os.str();});
      const size_t kw = max_width(k);
      os << std::left;
      for (size_t i=0; i<d.size(); ++i)
        os << std::setw(kw) << k(i) << "| " << d.val()(i) << '\n';
      return os;
    }
  }

  namespace detail {
    namespace hana = boost::hana;
    using namespace hana::literals;
    using std::decay_t;
    using std::enable_if_t;
    using std::result_of_t;

    template <class T>
    constexpr bool is_arithmetic_v = std::is_arithmetic<T>::value;
    template <class B, class D>
    constexpr bool is_base_of_v = std::is_base_of<B,D>::value;
    template <class T>
    constexpr bool is_integral_v = std::is_integral<T>::value;
    template <class T, class U>
    constexpr bool is_same_v = std::is_same<T,U>::value;
    template <class T>
    constexpr bool is_void_v = std::is_void<T>::value;
    
    template <class T, class U>
    struct decays_same {
      static const auto value = is_same_v<decay_t<T>,decay_t<U>>;
    };
    template <class T, class U>
    constexpr bool decays_same_v = decays_same<T,U>::value;
    
    template <class T>
    struct is_void_result {
      static const auto value = is_void_v<result_of_t<T>>;
    };
    template <class T>
    constexpr bool is_void_result_v = is_void_result<T>::value;

    template <class T> struct to_void { typedef void type; };
    template <class T> using to_void_t = typename to_void<T>::type;
    template <class T, class U = void>
    struct has_converge_type: std::false_type {};
    template <class T>
    struct has_converge_type<T, to_void_t<typename decay_t<T>::converge_type>>:
      std::true_type {};
    template <class T>
    constexpr bool has_converge_type_v = has_converge_type<T>::value;
    
    // http://talesofcpp.fusionfenix.com/post-11/true-story-call-me-maybe
    template <class, class = void> struct is_callable_imp: std::false_type {};
    template <class F, class... A>
    struct is_callable_imp<F(A...), detail::to_void_t<result_of_t<F(A...)>>>:
      std::true_type {};
    template <class E> struct is_callable: is_callable_imp<E> {};
    template <class E>
    constexpr bool is_callable_v = is_callable<E>::value;
  
    // I think this is impossible.  We want to say, is F callable with
    // one argument of type A, but NOT callable with two arguments
    // where the first argument is type A, and the 2nd argument is of
    // unknown (and therefore arbitrary) type.
    //
    // template <class F, class A, class R = void>
    // struct unary_only: std::false_type {};
    //
    // This would make it possible to write, e.g., L2(2*x)/2 instead
    // of L2(2*x)(2)

    ////////////////////////////////////////////////////////////////////////////
    // vec & dict creation
    ////////////////////////////////////////////////////////////////////////////
    template <class O>
    void vbuild(O o) {}
    template <class O, class T, class... U>
    void vbuild(O o, const T& t, const U&... u) {
      *o = t;
      ++o;
      vbuild(o, u...);
    }

    template <class K, class V>
    dict<K,V> make_dict(const vec<K>& k, const vec<V>& v) {
      return dict<K,V>(k,v);
    }

    template <class... T>
    auto tuple_cast(const boost::hana::tuple<T...>& x) {
      return *reinterpret_cast<const tuple<T...>*>(&x);
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // NonChainArg: things that aren't treated as arguments to
    // functions in / chaining
    ////////////////////////////////////////////////////////////////////////////
    struct NonChainArg {};
    template <class T>
    struct is_non_chain_arg {
      static const bool value = is_base_of_v<NonChainArg, decay_t<T>>;
    };
    template <class T> constexpr bool is_non_chain_arg_v =
      is_non_chain_arg<T>::value;
    
    ////////////////////////////////////////////////////////////////////////////
    // Adverbs
    ////////////////////////////////////////////////////////////////////////////
    struct Adverb: NonChainArg {};
    template <class T>
    struct is_adverb {
      static const bool value = is_base_of_v<Adverb, decay_t<T>>;
    };
    template <class T> constexpr bool is_adverb_v = is_adverb<T>::value;

    template <class F, class... T>
    constexpr auto uniform_result_type(const F& f, const tuple<T...>& t) {
      auto t0 = hana::type_c<decltype(f(t(0_c)))>;
      return hana::all_of(t, [&](auto&& x){
          return hana::type_c<decltype(f(x))> == t0;});
    }

    template <class F>
    struct Fun {
      F f;
      Fun(const F& f_): f(f_) {}
      template <class X>
      auto operator()(X&& x) const { return f(std::forward<X>(x)); }
      template <class X, class Y>
      auto operator()(X&& x, Y&& y) const {
        return f(std::forward<X>(x), std::forward<Y>(y));
      }
      template <class X, class Y, class Z>
      auto operator()(X&& x, Y&& y, Z&& z) const {
        return f(std::forward<X>(x), std::forward<Y>(y), std::forward<Z>(z));
      }
    };
    template <class F>
    Fun<F> make_fun(F&& f) { return Fun<F>(std::forward<F>(f)); }
    
    template <class F, class L>
    struct FunLhs {
      F f;
      const L& lhs;
      FunLhs(const F& f_, const L& l_): f(f_), lhs(l_) {}
      template <class R>
      auto operator()(R&& rhs) const { return f(lhs, std::forward<R>(rhs)); }
      template <class R, enable_if_t<!is_adverb<R>::value>* = nullptr>
      auto operator/(R&& rhs) const { return f(lhs, std::forward<R>(rhs)); }
    };
    template <class F, class L>
    FunLhs<F,L> make_funlhs(F&& f, L&& l_) {
      return FunLhs<F,L>(std::forward<F>(f), std::forward<L>(l_));
    }

    template <class F, class R>
    struct FunRhs {
      F f;
      const R& rhs;
      FunRhs(const F& f_, const R& r_): f(f_), rhs(r_) {}
      template <class L>
      auto operator()(L&& lhs) const { return f(std::forward<L>(lhs), rhs); }
    };
    template <class F, class R>
    FunRhs<F,R> make_funrhs(F&& f, R&& r) {
      return FunRhs<F,R>(std::forward<F>(f), std::forward<R>(r));
    }

    template <class F>
    struct BoundEach {
      F f;
      BoundEach(const F& f_): f(f_) {}
      
      template <class T>
      auto operator()(const T& t) const { return f(t); }
      template <class T, enable_if_t<is_void_result_v<F(T)>>* = nullptr>
      void operator()(const vec<T>& x) const {
        std::for_each(std::begin(x), std::end(x), f);
      }
////////////////////////////////////////////////////////////////////////////////
      template <class... T>
      auto operator()(const tuple<T...>& t) const {
        using namespace hana;
        return if_(type_c<void> == type_c<decltype(f(t(0_c)))>,
                   [&](auto&& x){for_each(x, f);},
                   [&](auto&& u){
                     return if_(!uniform_result_type(f,u),
                                [&](auto&& x){
                                  return tuple_cast(transform(x, f));
                                },
                                [&](auto&& x){ // uniform type so convert to vec
                                  vec<decltype(f(u(0_c)))> r(size(x));
                                  int i=0;
                                  for_each(x, [&](auto&& e){r(i++)=f(e);});
                                  return r;
                                })(u);})(t);
      }

      template<class K, class V, enable_if_t<is_void_result_v<F(V)>>* =nullptr>
      void operator()(const dict<K,V>& x) const { (*this)(x.val()); }
      template <class T, enable_if_t<!is_void_result_v<F(T)>>* = nullptr>
      auto operator()(const vec<T>& x) const {
        vec<decltype(f(std::declval<T>()))> r(x.size());
        std::transform(std::begin(x), std::end(x), std::begin(r), f);
        return r;
      }
      template<class K, class V,enable_if_t<!is_void_result_v<F(V)>>* =nullptr>
      auto operator()(const dict<K,V>& x) const {
        return make_dict(x.key(), (*this)(x.val()));
      }
        
      template <class T>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
    struct Each: Adverb {
      template <class F>
      auto operator()(F&& f) const { return BoundEach<F>(std::forward<F>(f)); }
    };

    template <class F>
    struct BoundEachLeft {
      F f;
      BoundEachLeft(const F& f_): f(f_) {}
      
      template <class L, class R, enable_if_t<!is_vec_v<L>>* = nullptr>
      auto operator()(const L& lhs, const R& rhs) const { return f(lhs, rhs); }
      template <class L, class R>
      auto operator()(const vec<L>& lhs, const R& rhs) const {
        return Each()([&](auto&& x){return f(x,rhs);})(lhs);
      }
      template <class K, class V, class R>
      auto operator()(const dict<K,V>& lhs, const R& rhs) const {
        return Each()([&](auto&& x){return f(x,rhs);})(lhs);
      }
      template <class... L, class R>
      auto operator()(const tuple<L...>& lhs, const R& rhs) const {
        return Each()([&](auto&& x){return f(x,rhs);})(lhs);
      }
    };
    struct EachLeft: Adverb {
      template <class F>
      auto operator()(F&& f) const {
        return BoundEachLeft<F>(std::forward<F>(f));
      }
    };

    template <class F>
    struct BoundEachRight {
      F f;
      BoundEachRight(const F& f_): f(f_) {}
      
      template <class L, class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const L& lhs, const R& rhs) const { return f(lhs,rhs); }
      template <class L, class R>
      auto operator()(const L& lhs, const vec<R>& rhs) const {
        return Each()([&](auto&& y){return f(lhs,y);})(rhs);
      }
      template <class L, class K, class V>
      auto operator()(const L& lhs, const dict<K,V>& rhs) const {
        return Each()([&](auto&& y){return f(lhs,y);})(rhs);
      }
      template <class L, class... R>
      auto operator()(const L& lhs, const tuple<R...>& rhs) const {
        return Each()([&](auto&& x){return f(x,lhs);})(rhs);
      }
    };
    struct EachRight: Adverb {
      template <class F>
        auto operator()(F&& f) const {
        return BoundEachRight<F>(std::forward<F>(f));
      }
    };

    template <class F>
    struct BoundEachBoth {
      F f;
      BoundEachBoth(const F& f_): f(f_) {}

      template <class L, class R,
        enable_if_t<!is_vec_v<L> && !is_vec_v<R>>* = nullptr>
      auto operator()(const L& lhs, const R& rhs) const { return f(lhs, rhs); }
      template <class L, class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const vec<L>& lhs, const R& rhs) const {
        return EachLeft()(f)(lhs, rhs);
      }
      template <class K, class V, class R,
        enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const dict<K,V>& lhs, const R& rhs) const {
        return EachLeft()(f)(lhs, rhs);
      }
      template <class... L, class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const tuple<L...>& lhs, const R& rhs) const {
        return EachLeft()(f)(lhs, rhs);
      }
        
      template <class L, class R, enable_if_t<!is_vec_v<L>>* = nullptr>
      auto operator()(const L& lhs, const vec<R>& rhs) const {
        return EachRight()(f)(lhs, rhs);
      }
      template <class L, class K, class V,
        enable_if_t<!is_vec_v<L>>* = nullptr>
      auto operator()(const L& lhs, const dict<K,V>& rhs) const {
        return EachRight()(f)(lhs, rhs);
      }
      template <class L, class... R, enable_if_t<!is_vec_v<L>>* = nullptr>
      auto operator()(const L& lhs, const tuple<R...>& rhs) const {
        return EachRight()(f)(lhs, rhs);
      }
        
      template<class L,class R,enable_if_t<is_void_result_v<F(L,R)>>* =nullptr>
      void operator()(const vec<L>& lhs, const vec<R>& rhs) const {
        assert(lhs.size() == rhs.size());
        for (size_t i=0; i<lhs.size(); ++i) f(lhs[i], rhs[i]);
      }
      template <class K, class V, class R>
      auto operator()(const dict<K,V>& lhs, const vec<R>& rhs) const {
        return (*this)(lhs.val(), rhs);
      }
      template<class L,class R,enable_if_t<!is_void_result_v<F(L,R)>>* =nullptr>
      auto operator()(const vec<L>& lhs, const vec<R>& rhs) const {
        assert(lhs.size() == rhs.size());
        vec<decltype(f(*std::begin(lhs), *std::begin(rhs)))> r(lhs.size());
        std::transform(std::begin(lhs), std::end(lhs), std::begin(rhs),
                       std::begin(r), f);
        return r;
      }
      template <class L, class K, class V>
      auto operator()(const vec<L>& lhs, const dict<K,V>& rhs) const {
        return (*this)(lhs, rhs.val());
      }
      
      template <class K, class V, class L, class U>
      auto operator()(const dict<K,V>& lhs, const dict<L,U>& rhs) const {
        typedef decltype(f(std::declval<V>(), std::declval<U>())) R;
        dict<K,R> r;
        for (const K& k: lhs.key())
          if (rhs.has(k))
            r[k] = f(lhs[k],rhs[k]);
        return r;
      }
    };
    struct EachBoth: Adverb {
      template <class F>
      auto operator()(F&& f) const {
        return BoundEachBoth<F>(std::forward<F>(f));
      }
    };

    template <class F>
    struct BoundEachMany {
      F f;
      BoundEachMany(const F& f_): f(f_) {}

      template <class T, class... U>
      auto operator()(const vec<T>& x, const vec<U>&... y) const {
        vec<int64_t> i(x.size());
        std::iota(std::begin(i), std::end(i), 0);
        return Each()([&](int64_t j){return f(x[j], y[j]...);})(i);
      }
    };
    struct EachMany: Adverb {
      template <class F>
      auto operator()(F&& f) const {
        return BoundEachMany<F>(std::forward<F>(f));
      }
    };
    
    template <class F>
    struct BoundEachPrior {
      F f;
      BoundEachPrior(const F& f_): f(f_) {}
      
      template <class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const R& rhs) const { return rhs; }
      template <class L, class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const L& lhs, const R& rhs) const { return f(lhs, rhs); }

      template <class R>
      auto operator()(const vec<R>& rhs) const {
        vec<decltype(f(*std::begin(rhs),*std::begin(rhs)))> r(rhs.size());
        if (rhs.size()) {
          r(0) = rhs(0);
          for (size_t i=1; i<rhs.size(); ++i)
            r(i) = f(rhs(i), rhs(i-1));
        }
        return r;
      }      
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return make_dict(x.key(), (*this)(x.val()));
      }
      template <class... T>
      auto operator()(const tuple<T...>& t) const {
        vec<decltype(f(t(0_c),t(0_c)))> r(hana::size(t));
        r(0) = t(0_c);
        if (1 < r.size()) {
          int i=1;
          hana::fold(t, [&](auto&& g,auto&& h){r(i++)=f(h,g);return h;});
        }
        return r;
      }
      
      template <class L, class R>
      auto operator()(const L& lhs, const vec<R>& rhs) const {
        vec<decltype(f(lhs,*std::begin(rhs)))> r(rhs.size());
        if (rhs.size()) {
          r(0) = f(rhs(0), lhs);
          for (size_t i=1; i<rhs.size(); ++i)
            r(i) = f(rhs(i), rhs(i-1));
        }
        return r;
      }
      template <class L, class K, class V>
      auto operator()(const L& lhs, const dict<K,V>& rhs) const {
        return make_dict(rhs.key(), (*this)(lhs, rhs.val()));
      }
      template <class L, class... T>
        auto operator()(const L& lhs, const tuple<T...>& rhs) const {
        vec<decltype(f(rhs(0_c),lhs))> r(hana::size(rhs));
        int i=0;
        hana::fold(rhs, lhs, [&](auto&& g,auto&& h){r(i++)=f(h,g);return h;});
        return r;
      }
            
      template <class R>
      auto operator/(R&& rhs) const { return (*this)(std::forward<R>(rhs)); }
    };
    struct EachPrior: Adverb {
      template <class F>
      auto operator()(F&& f) const {
        return BoundEachPrior<F>(std::forward<F>(f));
      }
    };

    template <class F>
    struct BoundOver {
      F f;
      BoundOver(const F& f_): f(f_) {}

      template <class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const R& rhs) const { return rhs; }
      template <class L, class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const L& lhs, const R& rhs) const { return f(lhs, rhs); }

      template <class T, result_of_t<F(T,T)>* = nullptr>
      auto operator()(const vec<T>& rhs) const {
        assert(rhs.size());
        return 1 == rhs.size()? static_cast<result_of_t<F(T,T)>>(rhs(0)) :
          std::accumulate(rhs.begin()+2, rhs.end(), f(rhs(0),rhs(1)), f);
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& rhs) const {
        return (*this)(rhs.val());
      }
      template <class... T>
      auto operator()(const tuple<T...>& x) const { return hana::fold(x, f); }
      
      template <class L, class R>
      auto operator()(const L& lhs, const vec<R>& rhs) const {
        return rhs.empty()? static_cast<result_of_t<F(L,R)>>(lhs) :
          std::accumulate(rhs.begin()+1, rhs.end(), f(lhs,rhs(0)), f);
      }
      template <class L, class K, class V>
      auto operator()(const L& lhs, const dict<K,V>& rhs) const {
        return (*this)(lhs, rhs.val());
      }
      template <class L, class... R>
      auto operator()(const L& lhs, const tuple<R...>& rhs) const {
        return hana::fold(rhs, lhs, f);
      }

      template <class R>
      auto operator/(R&& rhs) const { return (*this)(std::forward<R>(rhs)); }
    };
    struct Over: Adverb {
      template <class F>
      auto operator()(F&& f) const { return BoundOver<F>(std::forward<F>(f)); }
    };
  
    template <class F>
    struct BoundScan {
      F f;
      BoundScan(const F& f_): f(f_) {}

      template <class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const R& rhs) const { return rhs; }
      template <class R>
      auto operator()(const vec<R>& rhs) const {
        vec<decltype(f(*std::begin(rhs),*std::begin(rhs)))> r(rhs.size());
        std::partial_sum(std::begin(rhs), std::end(rhs), std::begin(r), f);
        return r;
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& rhs) const {
        return make_dict(rhs.key(), (*this)(rhs.val()));
      }
      template <class... T>
      auto operator()(const tuple<T...>& t) const {
        vec<decltype(f(t(0_c),t(0_c)))> r(hana::size(t));
        r(0) = t(0_c);
        int i = 1;
        hana::fold(t, [&](auto&& x, auto&& y){return r(i++)=f(x,y);});
        return r;
      }
      
      template <class L, class R, enable_if_t<!is_vec_v<R>>* = nullptr>
      auto operator()(const L& lhs, const R& rhs) const { return f(lhs, rhs); }
      template <class L, class R>
      auto operator()(const L& lhs, const vec<R>& rhs) const {
        vec<decltype(f(lhs,*std::begin(rhs)))> r(rhs.size());
        if (rhs.size()) {
          r(0) = f(lhs, rhs(0));
          for (size_t i=1; i<rhs.size(); ++i)
            r(i) = f(r(i-1), rhs(i));
        }
        return r;
      }
      template <class L, class K, class V>
      auto operator()(const L& lhs, const dict<K,V>& rhs) const {
        return make_dict(rhs.key(), (*this)(lhs, rhs.val()));
      }
      template <class L, class... T>
      auto operator()(const L& lhs, const tuple<T...>& rhs) const {
        vec<decltype(f(lhs,rhs(0_c)))> r(hana::size(rhs));
        int i = 0;
        hana::fold(rhs, lhs, [&](auto&& x, auto&& y){return r(i++)=f(x,y);});
        return r;
      }      
      
      template <class R>
      auto operator/(R&& rhs) const { return (*this)(std::forward<R>(rhs)); }
    };
    struct Scan: Adverb {
      template <class F>
      auto operator()(F&& f) const { return BoundScan<F>(std::forward<F>(f)); }
    };

    // Converge comes later, because it depends on Match.
  } // namespace detail

  //////////////////////////////////////////////////////////////////////////////
  // Atomic ops
  //////////////////////////////////////////////////////////////////////////////
#ifdef QICQ_ATOMIC_OP
#error "QIC_ATOMIC_OP macro conflict"
#elif defined QIC_DICT_MERGE_OP
#error "QIC_DICT_MERGE_OP macro conflict"
#elif defined QIC_DICT_MERGE_REL_OP
#error "QIC_DICT_MERGE_REL_OP macro conflict"
#else
#define QICQ_ATOMIC_OP(op)                                              \
  template <class T, class U,                                           \
    std::enable_if_t<std::is_arithmetic<U>::value>* = nullptr>          \
  auto operator op(const vec<T>& x, const U& y) {                       \
    return detail::Each()([&](const T& t){return t op y;})(x);          \
  }                                                                     \
  template <class T, class U,                                           \
    std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>          \
  auto operator op(const T& x, const vec<U>& y) {                       \
    return detail::Each()([&](const U& u){return x op u;})(y);          \
  }                                                                     \
  template <class T, class U>                                           \
  auto operator op(const vec<T>& x, const vec<U>& y) {                  \
    assert(x.size() == y.size());                                       \
    vec<decltype(std::declval<T>() op std::declval<U>())> r(x.size());  \
    std::transform(std::begin(x), std::end(x), std::begin(y), std::begin(r), \
                   [](const T& t, const U& u){return t op u;});         \
    return r;                                                           \
  }                                                                     \
  template <class K, class V, class U,                                  \
    std::enable_if_t<std::is_arithmetic<U>::value>* = nullptr>          \
  auto operator op(const dict<K,V>& x, const U& y) {                    \
    return detail::Each()([&](const V& v){return v op y;})(x);          \
  }                                                                     \
  template <class T, class K, class V,                                  \
    std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>          \
  auto operator op(const T& x, const dict<K,V>& y) {                    \
    return detail::Each()([&](const V& v){return x op v;})(y);          \
  }                                                                     \
  template <class K, class V, class U>                                  \
  auto operator op(const dict<K,V>& x, const vec<U>& y) {               \
    typedef decltype(std::declval<V>() op std::declval<U>()) R;         \
    return dict<K,R>(x.key(), x.val() op y);                            \
  }                                                                     \
  template <class T, class K, class V>                                  \
    auto operator op(const vec<T>& x, const dict<K,V>& y) {             \
    typedef decltype(std::declval<T>() op std::declval<V>()) R;         \
    return dict<K,R>(y.key(), x op y.val());                            \
  }
  //    return detail::EachBoth()(f)(x,y); // why doesn't this work?

#define QICQ_DICT_MERGE_OP(op, id)                                      \
  QICQ_ATOMIC_OP(op)                                                    \
  template <class K, class V, class U>                                  \
  auto operator op(const dict<K,V>& x, const dict<K,U>& y) {            \
    typedef decltype(std::declval<V>() op std::declval<U>()) R;         \
    dict<K,R> r;                                                        \
    for (const K& k: x.key()) {                                         \
      if (y.has(k))                                                     \
        r(k) = x(k) op y(k);                                            \
      else                                                              \
        r(k) = x(k) op U(id);                                           \
    }                                                                   \
    for (const K& k: y.key()) {                                         \
      if (!r.has(k))                                                    \
        r(k) = V(id) op y(k);                                           \
    }                                                                   \
    return r;                                                           \
  }
  
#define QICQ_DICT_MERGE_REL_OP(op,no_lhs,no_rhs)                        \
  QICQ_ATOMIC_OP(op)                                                    \
  template <class K, class V, class U>                                  \
  auto operator op(const dict<K,V>& x, const dict<K,U>& y) {            \
    typedef decltype(std::declval<V>() op std::declval<U>()) R;         \
    dict<K,R> r;                                                        \
    for (auto k: x.key()) {                                             \
      if (y.has(k))                                                     \
        r(k) = x(k) op y(k);                                            \
      else                                                              \
        r(k) = no_rhs;                                                  \
    }                                                                   \
    for (auto k: y.key()) {                                             \
      if (!r.has(k))                                                    \
        r(k) = no_lhs;                                                  \
    }                                                                   \
    return r;                                                           \
  }

  QICQ_DICT_MERGE_OP(+,0)
  QICQ_DICT_MERGE_OP(-,0)
  QICQ_DICT_MERGE_OP(*,1)
  QICQ_DICT_MERGE_OP(/,1) // questionable
  QICQ_ATOMIC_OP(%)       // no identity
  QICQ_DICT_MERGE_REL_OP(==,false,false)
  QICQ_DICT_MERGE_REL_OP(!=,true ,true )
  QICQ_DICT_MERGE_REL_OP(< ,true ,false)
  QICQ_DICT_MERGE_REL_OP(<=,true ,false)
  QICQ_DICT_MERGE_REL_OP(>=,false,true )
  QICQ_DICT_MERGE_REL_OP(> ,false,true )
#undef QIC_DICT_MERGE_REL_OP
#undef QIC_DICT_MERGE_OP
#undef QICQ_ATOMIC_OP
#endif

  template <class T>
  auto operator!(const vec<T>& x) {
    return detail::Each()([](const T& t){return !t;})(x);
  }
  template <class K, class V>
  auto operator!(const dict<K,V>& x) {
    return detail::Each()([](const V& t){return !t;})(x);
  }
  
  namespace detail {
    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    struct Til {
      vec<int64_t> operator()(int64_t n) const {
        vec<int64_t> r(n);
        std::iota(std::begin(r), std::end(r), 0);
        return r;
      }
      vec<int64_t> operator/(int64_t n) const { return (*this)(n); }
      vec<int64_t> operator/=(int64_t n) const { return (*this)(n); }
    };

    struct Amend {
      template <class T, class U, class F>
      auto operator()(vec<T> x, const U& i, const F& f) const {
        x[i] = f(x[i]);
        return x;
      }
      template <class T, class U, class F>
      auto operator()(vec<T>&& x, const U& i, const F& f) const {
        x[i] = f(x[i]);
        return x;
      }
      template <class K, class V, class U, class F>
      auto operator()(dict<K,V> x, const U& i, const F& f) const {
        x[i] = f(x[i]);
        return x;
      }
      template <class K, class V, class U, class F>
      auto operator()(dict<K,V>&& x, const U& i, const F& f) const {
        x[i] = f(x[i]);
        return x;
      }
    };

    struct At {
      template <class F, class I>
      auto operator()(const F& f, const I& i) const { return f(i); }

      // TODO add slice and figure out how it works
      template <class... T, class U>
      auto operator()(const tuple<T...>& x, const U& i) const {
        return hana::at(x, i);
      }
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& i) const {
        return EachRight()(*this)(x, i);
      }
      template <class K, class V, class U>
      auto operator()(const dict<K,V>& x, const vec<U>& i) const {
        return EachRight()(*this)(x, i);
      }
      template <class T, class K, class V>
      auto operator()(const vec<T>& x, const dict<K,V>& i) const {
        return make_dict(i.key(), (*this)(x, i.val()));
      }
      template <class K, class V, class L, class U>
      auto operator()(const dict<K,V>& x, const dict<L,U>& i) const {
        return make_dict(i.key(), (*this)(x, i.val()));
      }

      // TODO tuple index
    };
  } // namespace detail
  
  template <class T> template <class I>
  auto vec<T>::operator()(const vec<I>& i) const {
    return detail::At()(*this, i);
  }
  template <class T> template <class K, class V>
  auto vec<T>::operator()(const dict<K,V>& i) const {
    return detail::At()(*this, i);
  }
  template <class K, class V> template <class I>
  auto dict<K,V>::operator()(const vec<I>& i) const {
    return detail::At()(*this, i);
  }
  template <class K, class V> template <class L, class U>
  auto dict<K,V>::operator()(const dict<L,U>& i) const {
    return detail::At()(*this, i);
  }
  
  namespace detail {
    struct Avg {
      typedef double converge_type;
      
      template <class T>
      auto operator()(const vec<T>& x) const {
        assert(x.size());
        return Over()(std::plus<T>())(x) / static_cast<double>(x.size());
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Bool {
      template <class T>
      bool operator()(const T& t) const { return t; }
      template <class T>
      auto operator()(const vec<T>& x) const { return Each()(*this)(x); }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const { return Each()(*this)(x); }
      template <class... T>
      auto operator()(const tuple<T...>& x) const {
        vec<bool> r(hana::size(x));
        int i=0;
        hana::for_each(x, [&](auto&& e){r(i++)=e;});
        return r;
      }
      template <class T>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Compare {
      template <class T,
        enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
      bool operator()(const T& x, const T& y) const {return x<y;}
      template <class T>
      bool operator()(const vec<T>& x, const vec<T>& y) const {
        return std::lexicographical_compare(std::begin(x), std::end(x),
                                            std::begin(y), std::end(y),
                                            *this);
      }
    };

    struct Cut {
      template <class T, class U>
        auto operator()(const vec<T>& p, const vec<U>& x) const {
        static_assert(std::is_integral<T>::value,
                      "cut points must be integers");
        assert(p.size());
        assert(0<=p.front());
        assert(std::is_sorted(std::begin(p), std::end(p)));
        assert(p.back() <= x.size());
        /* return At()(x, */
        /*          p+Each()(Til())(Drop()(1,Deltas()(Join()(p, */
        /*                                                    x.size())))); */
        vec<vec<U>> r(p.size());
        auto xi = p.front();
        auto ri = std::begin(r);
        for (auto pi = ++std::begin(p); pi!=std::end(p); ++pi, ++ri)
          for (ri->reserve(*pi-xi); xi != *pi; ++xi)
            ri->push_back(x(xi));
        std::copy(std::begin(x)+xi, std::end(x), std::back_inserter(*ri));
        return r;
      }
    };

    struct Deltas {
      template <class T>
      auto operator()(const vec<T>& x) const {
        return EachPrior()(std::minus<T>())(x);
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return make_dict(x.key(), (*this)(x.val()));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Distinct {
      template <class T>
      vec<T> operator()(const vec<T>& x) const {
        vec<T> r;
        for (auto&& t: x)
          if (std::end(r) == std::find(std::begin(r), std::end(r), t))
            r.push_back(t);
        return r;
      }
      // TODO tuple
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Dot: NonChainArg {
      template <class F, class T,
        enable_if_t<is_callable_v<F(T)> && !is_callable_v<F(T,T)> &&
          !is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return f(x(0));
      }
      template <class F, class T,
        enable_if_t<!is_callable_v<F(T)> && is_callable_v<F(T,T)> &&
          !is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return f(x(0),x(1));
      }
      template <class F, class T,
        enable_if_t<is_callable_v<F(T)> && is_callable_v<F(T,T)> &&
          !is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return 2==x.size()? f(x(0),x(1)) : f(x(0));
      }
      template <class F, class T,
        enable_if_t<!is_callable_v<F(T)> && !is_callable_v<F(T,T)> &&
        is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return f(x(0),x(1),x(2));
      }
      template <class F, class T,
        enable_if_t<is_callable_v<F(T)> && !is_callable_v<F(T,T)> &&
        is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return 3==x.size()? f(x(0),x(1),x(2)) : f(x(0));
      }
      template <class F, class T,
        enable_if_t<!is_callable_v<F(T)> && is_callable_v<F(T,T)> &&
        is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return 3==x.size()? f(x(0),x(1),x(2)) : f(x(0),x(1));
      }
      template <class F, class T,
        enable_if_t<is_callable_v<F(T)> && is_callable_v<F(T,T)> &&
        is_callable_v<F(T,T,T)>>* = nullptr>
      auto operator()(const F& f, const vec<T>& x) const {
        return
          3==x.size()? f(x(0),x(1),x(2)) :
          2==x.size()? f(x(0),x(1))      :
          f(x(0));
      }

      template <class F, class T, class U>
      auto operator()(const F& f, const std::pair<T,U>& x) const {
        return f(x.first, x.second);
      }
      template <class F, class... T>
      auto operator()(F&& f, const tuple<T...>& x) const {
        return hana::unpack(x, std::forward<F>(f));
      }
    };
    
    struct Drop {
      template <class T>
      vec<T> operator()(int64_t n, const vec<T>& v) const {
        return
          v.size() <= std::abs(n)? vec<T>() :
          0 <= n                 ? vec<T>(std::begin(v)+n, std::end(v)) :
          vec<T>(std::begin(v), std::end(v)+n);
      }
      template <class K, class V>
      dict<K,V> operator()(int64_t n, const dict<K,V>& x) const {
        return make_dict((*this)(n,x.key()), (*this)(n,x.val()));
      }
      // TODO tuple
    };
  
    struct Enlist {
      template <class T>
      vec<T> operator()(const T& x) const { return vec<T>(1, x); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Find {
      template <class T, class U>
      int64_t operator()(const vec<T>& x, const U& y) const {
        return std::find(std::begin(x), std::end(x), y) - std::begin(x);
      }
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return EachRight()(*this)(x, y);
      }
      template <class T, class K, class V>
      auto operator()(const vec<T>& x, const dict<K,V>& y) const {
        return make_dict(y.key(), EachRight()(*this)(x, y.val()));
      }
      
      template <class K, class V, class U>
      K operator()(const dict<K,V>& x, const U& y) const {
        const int64_t i = (*this)(x.val(), y);
        // TODO we need a better answer than K() for not found in dict
        return i<x.size()? x.key()(i) : K();
      }
      template <class K, class V, class U>
      auto operator()(const dict<K,V>& x, const vec<U>& y) const {
        return EachRight()(*this)(x, y);
      }
      template <class K, class V, class L, class U>
      auto operator()(const dict<K,V>& x, const dict<L,U>& y) const {
        return make_dict(y.key(), EachRight()(*this)(x, y.val()));
      }

      // TODO tuple.  hana::find doesn't do what we want
    };

    struct First {
      template <class T>
      T operator()(const vec<T>& x) const {
        assert(x.size());
        return x.front();
      }
      template <class K, class V>
      V operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
      template <class... T>
      auto operator()(const tuple<T...>& x) const { return x(0_c); }
      
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Group {
      template <class T>
      auto operator()(const vec<T>& x) const {
        const vec<T> k(Distinct()(x));
        vec<vec<int64_t>> v(k.size());
        Find f;
        for (size_t i=0; i<x.size(); ++i)
          v(f(k,x(i))).push_back(i);
        return make_dict(k, v);
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        auto g = (*this)(x.val());
        return make_dict(g.key(), At()(x.key(), g.val()));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
    
    struct Iasc {
      template <class T>
      vec<int64_t> operator()(const vec<T>& x) const {
        Compare c;
        return Til()(x.size()).sort([&](int64_t i,int64_t j){
            return c(x(i),x(j));});
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return At()(x.key(), (*this)(x.val()));
      }
      
      // TODO use hana::sort.by for tuples
      
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
  
    struct Asc {
      template <class T>
      auto operator()(vec<T> x) const { return x.sort(Compare()); }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        auto i = Iasc()(x.val());
        return make_dict(At()(x.key(),i), At()(x.val(),i));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Idesc {
      template <class T>
      vec<int64_t> operator()(const vec<T>& x) const {
        Compare c;
        return Til()(x.size()).sort([&](int64_t i,int64_t j){
            return c(x(j),x(i));});
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return At()(x.key(), (*this)(x.val()));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Desc {
      template <class T>
      auto operator()(vec<T> x) const {
        Compare c;
        return x.sort([&](const T& a, const T& b){return c(b,a);});
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        auto i = Idesc()(x.val());
        return make_dict(At()(x.key(),i), At()(x.val(),i));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct In {
      template <class T, class U>
      bool operator()(const T& x, const vec<U>& y) const {
        return std::find(std::begin(y), std::end(y), x) != std::end(y);
      }
      template <class T, class K, class V>
      bool operator()(const T& x, const dict<K,V>& y) const {
        return (*this)(x, y.val());
      }
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return EachLeft()(*this)(x, y);
      }
      template <class T, class K, class V>
      auto operator()(const vec<T>& x, const dict<K,V>& y) const {
        return (*this)(x, y.val());
      }
      template <class K, class V, class L, class U>
      auto operator()(const dict<K,V>& x, const dict<L,U>& y) const {
        return make_dict(x.key(), (*this)(x.val(), y.val()));
      }
    };
  
    struct Join {
      template <class T, class U>
      auto operator()(const T& x, const U& y) const {
        // TODO something like boost::any
        vec<std::common_type_t<T,U>> r(2);
        r.front() = x;
        r.back() = y;
        return r;
      }
      template <class T, class U>
      auto operator()(const T& x, const vec<U>& y) const {
        vec<std::common_type_t<T,U>> r(y.size()+1);
        r.front() = x;
        std::copy(std::begin(y), std::end(y), std::begin(r)+1);
        return r;
      }
      template <class T, class U>
      auto operator()(const vec<T>& x, const U& y) const {
        vec<std::common_type_t<T,U>> r(x.size()+1);
        std::copy(std::begin(x), std::end(x), std::begin(r));
        r.back() = y;
        return r;
      }
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        vec<std::common_type_t<T,U>> r(x.size()+y.size());
        std::copy(std::begin(x), std::end(x), std::begin(r));
        std::copy(std::begin(y), std::end(y), std::begin(r)+x.size());
        return r;
      }
      template <class K, class V, class U>
      auto operator()(const dict<K,V>& x, const dict<K,U>& y) const {
        dict<K,V> r(x);
        Each()([&](int64_t i){r(y.key()(i))=y.val()(i);})(Til()(y.size()));
        return r;
      }
    };

    struct Key {
      template <class T>
      vec<int64_t> operator()(const vec<T>& x) const { return Til()(x.size()); }
      template <class K, class V>
      const vec<K>& operator()(const dict<K,V>& x) const { return x.key(); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
    
    struct Last {
      template <class T>
      T operator()(const vec<T>& x) const {
        assert(x.size());
        return x.back();
      }
      template <class K, class V>
      V operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
      template <class... T>
      auto operator()(const tuple<T...>& x) const { return hana::back(x); }

      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Max {
      template <class T, class U>
      auto operator()(const T& x, const U& y) const { return x<y? y:x; }

      template <class T>
      auto operator()(const vec<T>& x) const {
        assert(x.size());
        return *std::max_element(std::begin(x), std::end(x));
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return (*this)(x.val());
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }

      template <class T, class U>
      auto operator()(const T& x, const vec<U>& y) const {
        return EachRight()(*this)(x,y);
      }
      template <class T, class K, class V>
      auto operator()(const T& x, const dict<K,V>& y) const {
        return make_dict(y.key(), (*this)(x, y.val()));
      }

      template <class T, class U>
      auto operator()(const vec<T>& x, const U& y) const {
        return EachLeft()(*this)(x,y);
      }
      template <class K, class V, class U>
      auto operator()(const dict<K,V>& x, const U& y) const {
        return make_dict(x.key(), (*this)(x.val(), y));
      }

      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return EachBoth()(*this)(x,y);
      }
    };

    struct Med {
      // TODO med should work like avg on matrices
      template <class T>
      double operator()(vec<T> x) const {
        Compare c;
        std::sort(std::begin(x), std::end(x), c);
        const size_t p = x.size()/2;
        return 1 == x.size()%2? x(p) : (x(p-1)+x(p)) / 2.0;
      }
      template <class K, class V>
      double operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
      
      template <class T>
      auto operator/(T&& x) { return (*this)(std::forward<T>(x)); }
      template <class T>
      auto operator/=(T&& x) { return (*this)(std::forward<T>(x)); }
    };
  
    struct Min {
      template <class T, class U>
      auto operator()(const T& x, const U& y) const { return x<y? x:y; }

      template <class T>
      auto operator()(const vec<T>& x) const {
        assert(x.size());
        return *std::min_element(std::begin(x), std::end(x));
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return (*this)(x.val());
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }

      template <class T, class U>
      auto operator()(const T& x, const vec<U>& y) const {
        return EachRight()(*this)(x,y);
      }
      template <class T, class K, class V>
      auto operator()(const T& x, const dict<K,V>& y) const {
        return make_dict(y.key(), (*this)(x, y.val()));
      }

      template <class T, class U>
      auto operator()(const vec<T>& x, const U& y) const {
        return EachLeft()(*this)(x,y);
      }
      template <class K, class V, class U>
      auto operator()(const dict<K,V>& x, const U& y) const {
        return make_dict(x.key(), (*this)(x.val(), y));
      }

      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return EachBoth()(*this)(x,y);
      }
    };

    struct All {
      typedef bool converge_type;

      template <class T, enable_if_t<!is_vec_v<T>>* = nullptr>
      bool operator()(const T& x) const { return Bool()(x); }

      template <class T, enable_if_t<!is_vec_v<T>>* = nullptr>
      bool operator()(const vec<T>& x) const {
        return Over()(std::logical_and<bool>())
          (true, Each()([](const T& t){return t!=0;})(x));
      }
      template <class T, enable_if_t<is_vec_v<T>>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return Over()(Min())(Bool()(x));
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const { return (*this)(x.val()); }

      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Any {
      typedef bool converge_type;
      
      template <class T, enable_if_t<!is_vec_v<T>>* = nullptr>
      bool operator()(const vec<T>& x) const {
        return Over()(std::logical_or<bool>())
          (false, Each()([](const T& t){return t!=0;})(x));
      }
      template <class T, enable_if_t<is_vec_v<T>>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return Over()(Max())(Bool()(x));
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Match {      
      template <class T, enable_if_t<is_arithmetic_v<T>>* = nullptr>
      bool operator()(const T& x, const T& y) const { return Eq()(x,y); }
      template <class T>
      bool operator()(const vec<T>& x, const vec<T>& y) const {
        return x.size() == y.size() && All()(EachBoth()(*this)(x,y));
      }
      template <class K, class V>
      bool operator()(const dict<K,V>& x, const dict<K,V>& y) const {
        return (*this)(x.key(), y.key()) && (*this)(x.val(), y.val());
      }
      template <class T, class U>
      bool operator()(const T& x, const U& y) const { return false; }
    };

    // TODO can we share more code btwn BoundConvergeAny and
    // BoundConverge?
    template <class F>
    struct BoundConvergeAny {
      F f;
      BoundConvergeAny(const F& f_): f(f_) {}
      
      template <class T>
      auto operator()(const T& x) const { return keep_going(x, x); }
      template <class T>
      auto operator/(T&& x) const { return (*this)(x); }
      template <class T>
      auto operator/=(T&& x) const { return (*this)(x); }
      
    private:
      template <class T, class U,
        enable_if_t<is_same_v<U,result_of_t<F(U)>>>* = nullptr>
      U keep_going(const T& orig, U p) const {
        U r = f(p);
        for (; !Match()(r,p) && !Match()(r,orig); r = f(p))
          p = std::move(r);
        return r;
      }
      template <class T, class U,
        enable_if_t<!is_same_v<U,result_of_t<F(U)>>>* = nullptr>
      boost::any keep_going(const T& orig, const U& x) const {
        auto r = f(x);
        return Match()(r,x) || Match()(r,orig)? r :keep_going(orig, r);
      }
    };
    template <class F>
    struct BoundConverge {
      typedef typename decay_t<F>::converge_type R; // result type
      F f;
      BoundConverge(const F& f_): f(f_) {}
      
      template <class T,
        enable_if_t<is_same_v<T,result_of_t<F(T)>>>* = nullptr>
      auto operator()(const T& x) const { return keep_going(x, x); }
      template <class T,
        enable_if_t<!is_same_v<T,result_of_t<F(T)>>>* = nullptr>
      auto operator()(const T& x) const {
        return boost::any_cast<R>(keep_going(x, x));
      }
      
      template <class T>
      auto operator/(T&& x) const { return (*this)(x); }
      template <class T>
      auto operator/=(T&& x) const { return (*this)(x); }
      
    private:
      template <class T, class U,
        enable_if_t<is_same_v<U,result_of_t<F(U)>>>* = nullptr>
      U keep_going(const T& orig, U p) const {
        U r = f(p);
        for (; !Match()(r,p) && !Match()(r,orig); r = f(p))
          p = std::move(r);
        return r;
      }
      template <class T, class U,
        enable_if_t<!is_same_v<U,result_of_t<F(U)>>>* = nullptr>
      boost::any keep_going(const T& orig, const U& x) const {
        auto r = f(x);
        return Match()(r,x) || Match()(r,orig)? r :keep_going(orig, r);
      }
    };
    struct Converge: Adverb {
      // TODO use BoundConverge when we can determine F's return type
      // in advance.
      template <class F, enable_if_t<has_converge_type_v<F>
        /*|| F has fixed return type or F(T) returns T for all T*/>* = nullptr>
      auto operator()(F&& f) const {
        return BoundConverge<F>(std::forward<F>(f));
      }
      template <class F, enable_if_t<!has_converge_type_v<F>>* = nullptr>
      auto operator()(F&& f) {
        return BoundConvergeAny<F>(std::forward<F>(f));
      }
    };

    struct Differ {
      template <class T>
      auto operator()(const vec<T>& x) const {
        auto r = !EachPrior()(Match())(x);
        r(0) = true;
        return r;
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return make_dict(x.key(), (*this)(x.val()));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
    
    struct Rank {
      template <class T>
      vec<int64_t> operator()(const vec<T>& x) const {
        return Iasc()(Iasc()(x));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Raze {
      template <class T, enable_if_t<is_vec_v<T>>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return Over()(Join())(x);
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Reverse {
      template <class T>
      vec<T> operator()(const vec<T>& x) const {
        vec<T> r(x);
        std::reverse(r.begin(), r.end());
        return r;
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        // k&q reverse both key and value
        return make_dict((*this)(x.key()), (*this)(x.val()));
      }
      
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
  
    struct Rotate {
      template <class T>
      vec<T> operator()(int64_t n, const vec<T>& x) const {
        const int64_t m = 0<=n? n%x.size() : x.size() - (-n)%x.size();
        vec<T> r(x);
        std::rotate(std::begin(r), std::begin(r)+m, std::end(r));
        return r;
      }
    };

    struct Signum {
      template <class T>
      int operator()(const T& x) const { return (0<x)-(x<0); }
      template <class T>
      auto operator()(const vec<T>& x) const { return Each()(*this)(x); }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const {
        return make_dict(x.key(), (*this)(x.val()));
      }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
  
    struct Sum {
      template <class T, enable_if_t<is_arithmetic_v<T>>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return Over()(std::plus<std::common_type_t<int,T>>())(0, x);
      }
      // For a matrix
      template <class T, enable_if_t<!is_arithmetic_v<T>>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return Over()(std::plus<T>())(x);
      }
      template <class K, class V>
      auto operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Take {
      template <class T>
      vec<T> operator()(int64_t n, const T& x) const {
        return vec<T>(std::abs(n), x);
      }
    
      template <class T>
      vec<T> operator()(int64_t n, const vec<T>& x) const {
        return
          n <  0       ? from_back(-n, x)                       :
          n <= x.size()? vec<T>(std::begin(x), std::begin(x)+n) :
          Each()([&](int64_t j){return x(j%x.size());})(Til()(n));
      }
      template <class K, class V>
      dict<K,V> operator()(int64_t n, const dict<K,V>& x) const {
        return make_dict((*this)(x.key()), (*this)(x.val()));
      }

      template <class T, class U>
      vec<vec<U>> operator()(const vec<T>& rc, const vec<U>& x) const {
        static_assert(std::is_integral<T>::value,
                      "reshape shape must be 2 integers");
        assert(2 == rc.size());
        assert(0<rc.front() || 0<rc.back());
        return
          rc.front() <= 0? fixed_cols_flex_rows(rc.back() , x) :
          rc.back()  <= 0? fixed_rows_flex_cols(rc.front(), x) :
          matrix_from_vector(rc.front(), rc.back(), x);
      }

    private:
      template <class U>
      vec<vec<U>> fixed_cols_flex_rows(int64_t n, const vec<U>& x) const {
        if (x.size() <= n)
          return Enlist()(x);
        const double r = x.size() / static_cast<double>(n);
        const int64_t t = std::ceil(r);  // # rows
        const int64_t u = std::floor(r); // # full rows
        return t==u? matrix_from_vector(t, n, x) :
          Join()(matrix_from_vector(u, n, x),
                 matrix_from_vector(1, x.size() - u*n, Drop()(u*n, x)));
      }

      template <class U>
      vec<vec<U>> fixed_rows_flex_cols(int64_t m, const vec<U>& x) const {
        if (x.size() <= m)
          return Each()(Enlist())(x);
        const double c = x.size() / static_cast<double>(m);
        const int64_t t = std::ceil(c);  // # cols in big rows
        const int64_t u = std::floor(c); // # cols in small rows
        if (t==u)
          return matrix_from_vector(m, t, x);
        // Guassian elimination w/2 equations and 2 unknowns, b and s:
        // let b = # big rows, s = # small rows
        //          b*t + s*u = x.size()
        // b+s=m => b*u + s*u = m*u
        //       => b         = x.size() - m*u (since t-u=1)
        const int64_t b = x.size() - m*u; // # big rows
        const int64_t s = m-b;            // # small rows
        return Join()(matrix_from_vector(b, t, x),
                      matrix_from_vector(s, u, Drop()(b*t, x)));
      }

      template <class T>
      vec<T> from_back(int64_t n, const vec<T>& v) const {
        return n <= v.size()? vec<T>(std::end(v)-n, std::end(v)) :
          EachRight()([&](int64_t o,int64_t j){return v((o+j)%v.size());})
            (v.size() - n%v.size(), Til()(n));
      }
    
      template <class U>
      vec<vec<U>>
      matrix_from_vector(int64_t m, int64_t n, const vec<U>& x) const {
        return EachLeft()(EachRight()([&](int64_t r,int64_t c){
              return x((c+r*n)%x.size());}))(Til()(m),Til()(n));
      }
    };
  
    struct Sublist {
      template <class T>
      auto operator()(int64_t n, const vec<T>& x) const {
        return Take()(Signum()(n)*std::min(std::abs(n),
                                           static_cast<int64_t>(x.size())),
                      x);
      }
      template <class K, class V>
      auto operator()(int64_t n, const dict<K,V>& x) const {
        return make_dict((*this)(n, x.key()), (*this)(n, x.val()));
      }
      template <class T, class U>
      auto operator()(const vec<T>& bn, const vec<U>& x) const {
        static_assert(std::is_integral<T>::value,
                      "sublist range must be 2 integers");
        assert(2 == bn.size());
        assert(0 <= bn.front() && 0 <= bn.back());
        auto r = Drop()(bn.front(), x);
        return r.size() <= bn.back()? r : Take()(bn.back(), r);
      }
      template <class T, class K, class V>
      auto operator()(const vec<T>& bn, const dict<K,V>& x) const {
        return make_dict((*this)(bn, x.key()), (*this)(bn, x.val()));
      }
    };

    struct Union {
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return Distinct()(Join()(x,y));
      }
    };

    struct Value {
      template <class T>
      const vec<T>& operator()(const vec<T>& x) const { return x; }
      template <class K, class V>
      const vec<V>& operator()(const dict<K,V>& x) const { return x.val(); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };
    
    struct Where {
      template <class T>
      vec<int64_t> operator()(const vec<T>& x) const {
        assert(Over()(Min())(true,0<=x));
        // compiler doesn't like this :-(
        //      return Raze()(EachBoth()(Take())(x,Til()(x.size())));
        const int64_t n = Sum()(x);
        vec<int64_t> r(n);
        int64_t ri = 0;
        for (int64_t i=0; i<x.size(); ++i)
          for (int64_t j=0; j<x(i); ++j)
            r(ri++) = i;
        return r;
      }

      template <class K, class V>
      vec<K> operator()(const dict<K,V>& x) const {
        return At()(x.key(), (*this)(x.val()));
      }
          
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/(T&& x) const { return (*this)(std::forward<T>(x)); }
      template <class T, enable_if_t<!is_non_chain_arg_v<T>>* = nullptr>
      auto operator/=(T&& x) const { return (*this)(std::forward<T>(x)); }
    };

    struct Except {
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return At()(x, Where()(!(In()(x,y))));
      }
      template <class K, class V>
      vec<V> operator()(const dict<K,V>& x) const { return (*this)(x.val()); }
    };
  
    struct Inter {
      template <class T, class U>
      auto operator()(const vec<T>& x, const vec<U>& y) const {
        return At()(x, Where()(In()(x,y)));
      }
    };
  
    struct Xbar {
      template <class T, class U,
        enable_if_t<is_integral_v<T> && is_integral_v<U>>* = nullptr>
      auto operator()(const T& x, const vec<U>& y) const {
        assert(0<x);
        return x*(y/x);
      }
      template <class T, class K, class V>
      auto operator()(const T& x, const dict<K,V>& y) const {
        return make_dict(y.key(), (*this)(x, y.val()));
      }

      // Does this make any sense?
      /* template <class T, class U, */
      /*   enable_if_t<is_integral_v<T> && is_integral_v<U>>* = nullptr> */
      /* auto operator()(vec<T>& x, const vec<U>& y) { */
      /*   assert(Over()(Min())(true,0<x)); */
      /*   return x*(y/x); */
      /* } */
    };
  } // namespace detail
  
  //////////////////////////////////////////////////////////////////////////////
  // op/ as chain builder
  //////////////////////////////////////////////////////////////////////////////
  template <class F, class L,
    std::enable_if_t<!std::is_same<L,detail::Til>::value>* = nullptr>
  auto operator/(const L& x, const F& f) {
    return detail::make_funlhs(f, x);
  }
  
  /* template <class R, class A1, class A2, class L, */
  /*   std::enable_if_t<!std::is_same<L,detail::Til>::value>* = nullptr> */
  /* auto operator/(const L& x, R(*f)(A1,A2)) { */
  /*   return detail::make_funlhs(f, x); */
  /* } */
  
  /* template <class R, class A1, class A2, class L, */
  /*   std::enable_if_t<!std::is_same<L,detail::Til>::value>* = nullptr> */
  /* auto operator/(const L& x, R(&f)(A1,A2)) { */
  /*   return detail::make_funlhs(f, x); */
  /* } */
  
  /* template <class F, class T, */
  /*   std::enable_if_t<!detail::is_non_chain_arg_v<T> && */
  /*   detail::is_same_v<std::result_of_t<F(T)>,std::result_of_t<F(T)>>>* =nullptr> */
  /* auto operator/(const F& f, const T& x) { */
  /*   static_assert(!detail::is_non_chain_arg_v<T>, "wtf"); */
  /*   return f(x); */
  /* } */
  
  // F&& or L&& w/std::forward makes the compiler go nuts :-(
  /* template <class F, class L, */
  /*   std::enable_if_t<!decays_same<L,detail::Til>::value>* = nullptr> */
  /* auto operator/(L&& x, F&& f) { */
  /*   return detail::make_funlhs(std::forward<F>(f), std::forward<L>(x)); */
  /* } */

  template <class F>
  auto operator/(F&& f, const detail::Dot& d) {
    return detail::make_funlhs(d, std::forward<F>(f));
  }
  
  template <class F>
  auto operator/(F&& f, const detail::Converge& c) {
    return c(std::forward<F>(f));
  }

  template <class F>
  auto operator/(F&& f, const detail::Each& e) {
    return e(std::forward<F>(f));
  }

  template <class F>
  auto operator/(F&& f, const detail::EachLeft& e) {
    return e(std::forward<F>(f));
  }
  template <class F, class L>
  auto operator/(const detail::FunLhs<F,L>& fl, const detail::EachLeft& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }
  template <class F, class L>
  auto operator/(detail::FunLhs<F,L>&& fl, const detail::EachLeft& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }

  template <class F>
  auto operator/(F&& f, const detail::EachRight& e) {
    return e(std::forward<F>(f));
  }
  template <class F, class L>
  auto operator/(const detail::FunLhs<F,L>& fl, const detail::EachRight& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }
  template <class F, class L>
  auto operator/(detail::FunLhs<F,L>&& fl, const detail::EachRight& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }

  // TODO extend this logic to all dyadic Bound adverbs
  template <class F, class R>
  auto operator/(const detail::BoundEachBoth<F>& e, R&& x) {
    return detail::make_funrhs(e, std::forward<R>(x));
  }
  template <class F>
  auto operator/(F&& f, const detail::EachBoth& e) {
    return e(std::forward<F>(f));
  }
  template <class F, class L>
  auto operator/(const detail::FunLhs<F,L>& fl, const detail::EachBoth& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }
  template <class F, class L>
  auto operator/(detail::FunLhs<F,L>&& fl, const detail::EachBoth& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }

  template <class F>
  auto operator/(F&& f, const detail::EachPrior& e) {
    return e(std::forward<F>(f));
  }
  template <class F, class L>
  auto operator/(const detail::FunLhs<F,L>& fl, const detail::EachPrior& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }
  template <class F, class L>
  auto operator/(detail::FunLhs<F,L>&& fl, const detail::EachPrior& e) {
    return detail::make_funlhs(e(fl.f), fl.lhs);
  }

  template <class F>
  auto operator/(F&& f, const detail::Over& o) {
    return o(std::forward<F>(f));
  }
  template <class F, class L>
  auto operator/(const detail::FunLhs<F,L>& fl, const detail::Over& o) {
    return detail::make_funlhs(o(fl.f), fl.lhs);
  }
  template <class F, class L>
  auto operator/(detail::FunLhs<F,L>&& fl, const detail::Over& o) {
    return detail::make_funlhs(o(fl.f), fl.lhs);
  }

  template <class F>
  auto operator/(F&& f, const detail::Scan& s) {
    return s(std::forward<F>(f));
  }
  template <class F, class L>
  auto operator/(const detail::FunLhs<F,L>& fl, const detail::Scan& s) {
    return detail::make_funlhs(s(fl.f), fl.lhs);
  }
  template <class F, class L>
  auto operator/(detail::FunLhs<F,L>&& fl, const detail::Scan& s) {
    return detail::make_funlhs(s(fl.f), fl.lhs);
  }

  //////////////////////////////////////////////////////////////////////////////
  // op/= as R->L chain builder
  //////////////////////////////////////////////////////////////////////////////
  template <class F, class R,
    std::enable_if_t<!std::is_same<F,detail::Til>::value>* = nullptr>
  auto operator/=(const F& f, const R& x) {
    return detail::make_funrhs(f, x);
  }
  
  template <class L, class F, class R>
  auto operator/=(const detail::FunLhs<F,L>& f, const R& r) { return f(r); }

  template <class L, class F, class R>
  auto operator/(const L& l_, const detail::FunRhs<F,R>& x) { return x(l_); }
  template <class L, class F, class R>
  auto operator/=(const L& l_, const detail::FunRhs<F,R>& x) { return x(l_); }
  
  template <class F, class R>
  auto operator/=(const detail::BoundEach<F>& e, R&& x) {
    return e(std::forward<R>(x));
  }
  /* template <class F, class R> */
  /* auto operator/=(const detail::BoundEachBoth<F>& e, R&& x) { */
  /*   return detail::make_funrhs(e, std::forward<R>(x)); */
  /* } */
  template <class F, class R>
  auto operator/=(const detail::BoundEachRight<F>& e, R&& x) {
    return e(std::forward<R>(x));
  }
  template <class F, class R>
  auto operator/=(const detail::BoundOver<F>& o, R&& x) {
    return o(std::forward<R>(x));
  }
  template <class F, class R>
  auto operator/=(const detail::BoundScan<F>& s, R&& x) {
    return s(std::forward<R>(x));
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Creation convenience functions
  //////////////////////////////////////////////////////////////////////////////
  template <class... T>
  auto v(const T&... a) {
    vec<std::common_type_t<T...>> r(sizeof...(a));
    detail::vbuild(std::begin(r), a...);
    return r;
  }

  template <class T>
  auto v(std::initializer_list<T> i) { return vec<T>(i); }
  inline auto v(const char* s) { return vec<char>(s); }
  inline auto v(const std::string& s) { return vec<char>(s); }

  template <class K, class V>
  auto d(const vec<K>& k, const vec<V>& v) { return dict<K,V>(k,v); }
  template <class K, class V>
  auto d(vec<K>&& k, vec<V>&& v) { return dict<K,V>(k,v); }

  template <class... T>
  auto t(T&&... a) { return tuple<T...>(std::forward<T>(a)...); }

  template <class F>
  auto f(F&& f) { return detail::make_fun(f); }
  
  //////////////////////////////////////////////////////////////////////////////
  // Adverbs 
  //////////////////////////////////////////////////////////////////////////////
  extern detail::Converge  conv;
  extern detail::Each      each;
  extern detail::EachLeft  left;
  extern detail::EachRight right;
  extern detail::EachBoth  both;
  extern detail::EachMany  many;
  extern detail::EachPrior prior;
  extern detail::Over      over;
  extern detail::Scan      scan;

  //////////////////////////////////////////////////////////////////////////////
  // Functions
  //////////////////////////////////////////////////////////////////////////////
  extern detail::All all;
  extern detail::Amend amend;
  extern detail::Any any;
  extern detail::Asc asc;
  extern detail::At at;
  extern detail::Avg avg;
  extern detail::Bool cbool;
  extern detail::Cut cut;
  extern detail::Deltas deltas;
  extern detail::Desc desc;
  extern detail::Differ differ;
  extern detail::Distinct distinct;
  extern detail::Dot dot;
  extern detail::Drop drop;
  extern detail::Enlist enlist;
  extern detail::Except except;
  extern detail::Find find;
  extern detail::First first;
  extern detail::Group group;
  extern detail::Iasc iasc;
  extern detail::Idesc idesc;
  extern detail::In in;
  extern detail::Inter inter;
  extern detail::Join join;
  extern detail::Key key;
  extern detail::Last last;
  extern detail::Match match;
  extern detail::Max max;
  extern detail::Med med;
  extern detail::Min min;
  //extern detail::Next next_; TODO: implies nulls
  //extern detail::Prev prev_; TODO: implies nulls
  extern detail::Rank rank;
  extern detail::Raze raze;
  extern detail::Reverse rev;
  extern detail::Rotate rot;
  extern detail::Signum signum;
  extern detail::Sublist sublist;
  extern detail::Sum sum;
  //extern detail::Sv sv TODO
  extern detail::Take take;
  extern detail::Til til;
  extern detail::Union union_;
  extern detail::Value val;
  //extern detail::Vs vs; TODO
  extern detail::Where where;
  extern detail::Xbar xbar;
} // namespace qicq

#endif
