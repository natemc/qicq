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
#elif defined NLL1
#error "NLL1 macro conflict"
#elif defined NLL1
#error "NLL1 macro conflict"
#elif defined NLL2
#error "NLL2 macro conflict"
#elif defined NLL3
#error "NLL3 macro conflict"
#elif defined NLV0
#error "NLV0 macro conflict"
#elif defined NLV1
#error "NLV1 macro conflict"
#elif defined NLV2
#error "NLV2 macro conflict"
#elif defined NLV3
#error "NLV3 macro conflict"
#elif defined LA_PICK
#error "LA_PICK macro conflict"
#elif defined L1A
#error "L1A macro conflict"
#elif defined L2A
#error "L2A macro conflict"
#elif defined L3A
#error "L3A macro conflict"
#elif defined L4A
#error "L4A macro conflict"
#elif defined LA
#error "LA macro conflict"
#elif defined NLL1A
#error "NLL1A macro conflict"
#elif defined NLL2A
#error "NLL2A macro conflict"
#elif defined NLL3A
#error "NLL3A macro conflict"
#elif defined NLL4A
#error "NLL4A macro conflict"
#elif defined NLLA
#error "NLLA macro conflict"
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

// Non-local (i.e., global) versions: no capture spec
#define NLL0(expr) [](){return expr;}
#define NLL1(expr) qicq::detail::make_l1s([](auto&& x){return expr;})
#define NLL2(expr) [](auto&& x,auto&& y){return expr;}
#define NLL3(expr) [](auto&& x,auto&& y,auto&& z){return expr;}
#define NLV0(expr) [](){expr;}
#define NLV1(expr) qicq::detail::make_l1s([](auto&& x){expr;})
#define NLV2(expr) [](auto&& x,auto&& y){expr;}
#define NLV3(expr) [](auto&& x,auto&& y,auto&& z){expr;}

// user-defined args versions; http://stackoverflow.com/questions/11761703
#define LA_PICK(_1,_2,_3,_4,_5,NAME,...) NAME

#define L1A(a,expr)       qicq::detail::make_l1s([&](auto&& a){return expr;})
#define L2A(a,b,expr)     [&](auto&& a,auto&& b){return expr;}
#define L3A(a,b,c,expr)   [&](auto&& a,auto&& b,auto&& c){return expr;}
#define L4A(a,b,c,d,expr) [&](auto&& a,auto&& b,auto&& c,auto&& d){return expr;}
#define LA(...)           LA_PICK(__VA_ARGS__,L4A,L3A,L2A,L1A)(__VA_ARGS__)

#define NLL1A(a,expr)       qicq::detail::make_l1s([&](auto&& a){return expr;})
#define NLL2A(a,b,expr)     [&](auto&& a,auto&& b){return expr;}
#define NLL3A(a,b,c,expr)   [&](auto&& a,auto&& b,auto&& c){return expr;}
#define NLL4A(a,b,c,d,expr) [&](auto&&a,auto&&b,auto&&c,auto&&d){return expr;}
#define NLLA(...) LA_PICK(__VA_ARGS__,NLL4A,NLL3A,NLL2A,NLL1A)(__VA_ARGS__)
#endif

#endif
