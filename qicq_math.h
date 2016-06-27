#ifndef QICQ_MATH_H
#define QICQ_MATH_H

#include <cmath>
#include <qicq/qicq.h>

namespace qicq {
  namespace detail {
    struct Abs {
      static const size_t arity = 1;
      template <class T>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<T(*)(T)>(std::abs))(x);
      }
    };
    
    struct Cos {
      static const size_t arity = 1;
      template <class T,
        std::enable_if_t<!std::is_integral<T>::value>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<T(*)(T)>(std::cos))(x);
      }
      template <class T,
        std::enable_if_t<std::is_integral<T>::value>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<double(*)(double)>(std::cos))(x);
      }
    };
    
    struct Exp {
      static const size_t arity = 1;
      template <class T,
        std::enable_if_t<!std::is_integral<T>::value>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<T(*)(T)>(std::exp))(x);
      }
      template <class T,
        std::enable_if_t<std::is_integral<T>::value>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<double(*)(double)>(std::exp))(x);
      }
    };
    
    struct Sin {
      static const size_t arity = 1;
      template <class T,
        std::enable_if_t<!std::is_integral<T>::value>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<T(*)(T)>(std::sin))(x);
      }
      template <class T,
        std::enable_if_t<std::is_integral<T>::value>* = nullptr>
      auto operator()(const vec<T>& x) const {
        return each(static_cast<double(*)(double)>(std::sin))(x);
      }
    };
  } // namespace detail
  
  template <class T> auto abs(const vec<T>& x) { return detail::Abs()(x); }
  template <class T> auto cos(const vec<T>& x) { return detail::Cos()(x); }
  template <class T> auto exp(const vec<T>& x) { return detail::Exp()(x); }
  template <class T> auto sin(const vec<T>& x) { return detail::Sin()(x); }

  // Even if you don't using namespace std, this will be ambiguous :-(
  // extern detail::Abs abs;
  extern detail::Abs qabs;
  extern detail::Cos qcos;
  extern detail::Exp qexp;
  extern detail::Sin qsin;
} // namespace qicq

#endif
