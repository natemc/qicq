#ifndef QICQ_ADAPT_H
#define QICQ_ADAPT_H

#include <map>
#include <unordered_map>
#include <vector>

namespace qicq {
  namespace detail {
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
  } // namespace detail

  template <class F> // is_function doesn't work ??
    // , std::enable_if_t<std::is_function<std::decay_t<F>>::value>* = nullptr>
  auto f(F&& f) { return detail::make_fun(f); }
  
  template <class T, size_t N>
  auto f(T(&a)[N]) { return [&](const size_t i){return a[i];}; }
  template <class T, size_t N>
  auto f(const T(&a)[N]) { return [&](const size_t i){return a[i];}; }
  
  template <class T, size_t N>
  auto f(std::array<T,N>& a) { return [&](const size_t i){return a[i];}; }
  template <class T, size_t N>
  auto f(const std::array<T,N>& a) { return [&](const size_t i){return a[i];}; }

  template <class K, class V, class C>
  auto f(std::map<K,V,C>& m) { return [&](const K& k){return m[k];}; }
  template <class K, class V, class C>
  auto f(const std::map<K,V,C>& m) { return [&](const K& k){return m[k];}; }
  
  template <class K, class V, class H, class E>
  auto f(std::unordered_map<K,V,H,E>& m) {
    return [&](const K& k){return m[k];};
  }
  template <class K, class V, class H, class E>
  auto f(const std::unordered_map<K,V,H,E>& m) {
    return [&](const K& k){return m[k];};
  }

  template <class T>
  auto f(const std::vector<T>& x) { return [&](size_t i){return x[i];}; }
  template <class T>
  auto f(std::vector<T>& x) { return [&](size_t i){return x[i];}; }
  
} // namespace qicq

#endif
