#ifndef HUNIT_H
#define HUNIT_H

// Based on Kevlin Henney's presentation, Programming with GUTs.
// https://www.infoq.com/presentations/testing-communication

#include <functional>
#include <stdexcept>
#include <string>

namespace hunit {
  struct testcase {
    std::string           spec;
    std::function<void()> f;
  };

  struct testsuite {
    template <size_t N>
    testsuite(const testcase(&suite)[N]): tests(suite), n(N) {}
    const testcase* tests;
    size_t n;
  };

  struct failure: std::logic_error {
    failure(const char* file_, int line_, const char* e):
      std::logic_error(e), file(file_), line(line_)
    {}
    failure(const char* file_, int line_, const std::string& e):
      std::logic_error(e), file(file_), line(line_)
    {}
    const char* file;
    int line;
  };

  struct Runner {
    Runner(): cases(0), errors(0), fails(0) {}
    
    int run(const testsuite& suite);
    int run(const testcase* suite, size_t n);
    template <size_t N>
    int run(const testcase(&suite)[N]) { return run(suite, N); }
    
    int cases;
    int errors;
    int fails;
  };

  int run(const testsuite* body, size_t n);
  template <size_t N>
  int run(const testsuite(&body)[N]) { return run(body, N); }
} // namespace hunit

#define ASSERT(x) do {						\
    if(!(x)) throw hunit::failure(__FILE__, __LINE__, #x);	\
  } while(0);

#define CATCH(x,e) do {							\
  try{(x); throw hunit::failure(__FILE__, __LINE__, #x); }		\
    catch(const e&){}							\
    catch(const std::exception& w){					\
      throw hunit::failure(__FILE__, __LINE__, #x);			\
    }									\
  } while(0);
#endif
