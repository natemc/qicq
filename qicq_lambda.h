#ifndef QICQ_LAMBDA_H
#define QICQ_LAMBDA_H

////////////////////////////////////////////////////////////////////////////////
// Lambda-generating macros
////////////////////////////////////////////////////////////////////////////////

#ifdef L0
#error "L0 macro conflict"
#elif defined L1
#error "L1 macro conflict"
#elif defined L2
#error "L2 macro conflict"
#elif defined L3
#error "L3 macro conflict"
#elif defined V0
#error "V0 macro conflict"
#elif defined V1
#error "V1 macro conflict"
#elif defined V2
#error "V2 macro conflict"
#elif defined V3
#error "V3 macro conflict"
#else

namespace qicq {
  namespace detail {
    template <class F>
    struct L1S {
      static const size_t arity = 1;
      F f;
      L1S(const F& f_): f(f_) {}
      template <class X>
      auto operator()(X&& x) const { return f(std::forward<X>(x)); }
    };
    template <class F>
    auto make_l1s(F&& f) { return L1S<F>(std::forward<F>(f)); }

    // YAGNI
    /* template <class F> */
    /* struct L2S { */
    /*   static const size_t arity = 2; */
    /*   F f; */
    /*   L2S(const F& f_): f(f_) {} */
    /*   template <class X, class Y> */
    /*   auto operator()(X&& x, Y&& y) const { */
    /*     return f(std::forward<X>(x), std::forward<Y>(y)); */
    /*   } */
    /* }; */
    /* template <class F> */
    /* auto make_l2s(F&& f) { return L2S<F>(std::forward<F>(f)); } */
  } // namespace detail
} // namespace qicq

#define L0(expr) [&](){return expr;}
#define L1(expr) qicq::detail::make_l1s([&](auto&& x){return expr;})
#define L2(expr) [&](auto&& x,auto&& y){return expr;}
#define L3(expr) [&](auto&& x,auto&& y,auto&& z){return expr;}
#define V0(expr) [&](){expr;}
#define V1(expr) qicq::detail::make_l1s([&](auto&& x){expr;})
#define V2(expr) [&](auto&& x,auto&& y){expr;}
#define V3(expr) [&](auto&& x,auto&& y,auto&& z){expr;}
#endif

#endif
