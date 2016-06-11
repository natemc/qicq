#ifndef QICQ_SYM
#define QICQ_SYM

#include <cstdint>
#include <cstring>
#include <iostream>

namespace qicq {
  struct sym {
    sym(): i(0) {}
    sym(char c): i(makesym(&c, 1)) {}
    sym(const char* s): i(makesym(s, strlen(s))) {}
    sym(const char* s, size_t n): i(makesym(s, n)) {}
    const char* c_str() const { return base+i; }

  private:
    int32_t i;
    static char* base;
    static int32_t makesym(const char* s, size_t n);
    friend bool operator==(const sym& x, const sym& y);
    friend std::ostream& operator<<(std::ostream& os, const sym& s);
  };
  inline sym operator""_m(const char* s, size_t n) { return sym(s,n); }
  
  inline bool operator==(const sym& x, const sym& y) { return x.i==y.i; }
  inline bool operator!=(const sym& x, const sym& y) { return !(x==y); }
  inline bool operator<(const sym& x, const sym& y){
    return strcmp(x.c_str(), y.c_str()) < 0;
  }
  inline bool operator<=(const sym& x, const sym& y) { return !(y<x); }
  inline bool operator>(const sym& x, const sym& y) { return y<x; }
  inline bool operator>=(const sym& x, const sym& y) { return !(x<y); }

  inline
  std::ostream& operator<<(std::ostream& os, const sym& s) {
    return os << s.c_str();
  }

  namespace detail {
    struct SymBuilder {
      inline sym operator()(char c)        const { return sym(c); }
      inline sym operator()(const char* s) const { return sym(s); }
    };
  } // namespace detail
  extern detail::SymBuilder s;
  
  void debug_syms(std::ostream& os);
} // namespace qicq

#endif
