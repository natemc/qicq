#ifndef QICQ_MATH_H
#define QICQ_MATH_H

#include <cmath>
#include <qicq/qicq.h>

namespace qicq {
  template <class T>
  auto abs(const vec<T>& x) {
    return each(static_cast<T(*)(T)>(std::abs))(x);
  }
  
  template <class T, std::enable_if_t<!std::is_integral<T>::value>* = nullptr>
  auto cos(const vec<T>& x) {
    return each(static_cast<T(*)(T)>(std::cos))(x);
  }
  template <class T, std::enable_if_t<std::is_integral<T>::value>* = nullptr>
  auto cos(const vec<T>& x) {
    return each(static_cast<double(*)(double)>(std::cos))(x);
  }
  
  template <class T, std::enable_if_t<!std::is_integral<T>::value>* = nullptr>
  auto sin(const vec<T>& x) {
    return each(static_cast<T(*)(T)>(std::sin))(x);
  }
  template <class T, std::enable_if_t<std::is_integral<T>::value>* = nullptr>
  auto sin(const vec<T>& x) {
    return each(static_cast<double(*)(double)>(std::sin))(x);
  }
} // namespace qicq

#endif
