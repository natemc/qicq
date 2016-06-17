#ifndef QICQ_FUN_H
#define QICQ_FUN_H

namespace qicq {
  namespace detail {
    struct Div {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x/y; }
    };

    // TODO fuzzy match on float & double for all 6 relops
    struct Eq {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x==y; }
    };
    
    struct Ge {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x>=y; }
    };
    
    struct Gt {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x>y; }
    };
    
    struct Le {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x<=y; }
    };
    
    struct Lt {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x<y; }
    };
    
    struct Minus {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x-y; }
    };
    
    struct Ne {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x!=y; }
    };
    
    struct Plus {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x+y; }
    };

    struct Times {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x*y; }
    };
  } // namespace detail
  
  extern detail::Div   div;
  extern detail::Eq    eq;
  extern detail::Ge    ge;
  extern detail::Gt    gt;
  extern detail::Le    le;
  extern detail::Lt    lt;
  extern detail::Minus minus;
  extern detail::Ne    ne;
  extern detail::Plus  plus;
  extern detail::Times times;
} // namespace qicq

#endif
