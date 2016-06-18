#ifndef QICQ_FUN_H
#define QICQ_FUN_H

#include <algorithm>
#include <cmath>

namespace qicq {
  namespace detail {
    struct Div {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x/y; }
    };

    struct Eq {
      // TODO precompute these and put literals here
      // static constexpr double dt = std::exp2(-43);
      // static constexpr float  ft = std::exp2(-19);
      
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x==y; }
      bool operator()(float x, float y) const {
        return fpeq(x, y, float(std::exp2(-19)));
      }
      bool operator()(double x, double y) const {
        return fpeq(x, y, std::exp2(-43));
      }

    private:
      // fuzzy match float and double
      // 0n~0n inf~inf -inf~-inf
      // 0 must ~ 0 exactly
      // else tolerance match
      template <class T>
      bool fpeq(T x, T y, T t) const {
        return
	  std::isnan(x)    ? std::isnan(y)     :
	  std::isnan(y)    ? false             :
	  (x<0) != (y<0)   ? false             :
	  !std::isfinite(x)? !std::isfinite(y) :
	  !std::isfinite(y)? false             :
	  std::abs(x-y) <= t*std::max(std::abs(x),std::abs(y));
      }
    };
    
    struct Ne {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x!=y; }
      bool operator()(float x, float y) const { return !(Eq()(x,y)); }
      bool operator()(double x, double y) const { return !(Eq()(x,y)); }
    };
    
    struct Lt {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x<y; }
      bool operator()(float x, float y) const { return x<y && Ne()(x,y); }
      bool operator()(double x, double y) const { return x<y && Ne()(x,y); }
    };
    
    struct Ge {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x>=y; }
      bool operator()(float x, float y) const { return !(Lt()(x,y)); }
      bool operator()(double x, double y) const { return !(Lt()(x,y)); }
    };
    
    struct Gt {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x>y; }
      bool operator()(float x, float y) const { return (Lt()(y,x)); }
      bool operator()(double x, double y) const { return (Lt()(y,x)); }
    };
    
    struct Le {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x<=y; }
      bool operator()(float x, float y) const { return !(Lt()(y,x)); }
      bool operator()(double x, double y) const { return !(Lt()(y,x)); }
    };
    
    struct Minus {
      template <class X, class Y>
      auto operator()(const X& x, const Y& y) const { return x-y; }
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
