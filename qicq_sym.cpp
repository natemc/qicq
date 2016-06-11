#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <qicq/qicq_sym.h>
#include <vector>
using namespace std;

namespace {
  char symbuf[1024*1024]; // 1MB > 10,000 100-char symbols
  char* nextsym = symbuf+1;
  vector<const char *> syms;

  template <typename T, size_t N>
  size_t array_len(T(&a)[N]){ return N; }  
}

namespace qicq {
  char* sym::base = symbuf;

  void debug_syms(std::ostream& os) {
    for_each(begin(syms), end(syms),
	     [&](const char* s){
	       const void* p = reinterpret_cast<const void*>(s);
	       os << p << '\t' << s << '\n';
	     });
  }

  int32_t sym::makesym(const char* s, size_t n) {
    auto i = lower_bound(begin(syms), end(syms), s,
			 [=](auto x, auto y){return memcmp(x,y,n)<0;});
    if (i != syms.end() && !strcmp(s,*i))
      return *i - base;

    assert(nextsym+n+1 < symbuf+array_len(symbuf));
    char * const newsym = strncpy(nextsym, s, n);
    *(newsym+n) = '\0';
    nextsym += n+1;
    syms.insert(i, newsym);
    return newsym - base;
  }

  detail::SymBuilder s;
} // namespace qicq
