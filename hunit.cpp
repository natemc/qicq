#include "hunit.h"

#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <sys/ioctl.h>

namespace {
  const char green  [] = "\033[32m";
  const char yellow [] = "\033[33m";
  const char red    [] = "\033[31m";
  const char igreen [] = "\033[32m\033[7;1m";
  const char iyellow[] = "\033[33m\033[7;1m";
  const char ired   [] = "\033[31m\033[7;1m";
  const char erase  [] = "\033[1K\033[0E";
  const char reset  [] = "\033[0m";
  
  int terminal_width() {
    winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return w.ws_col;
  }

  template <class... T>
  std::string bar(double frac, T&&... x) {
    std::ostringstream o, p;
    int dummy[] = { (o<<x,0)... };
    p << std::left << std::setw(terminal_width()*frac) << o.str();
    return p.str();
  }
} // namespace

namespace hunit {
  int Runner::run(const testsuite& suite) { return run(suite.tests, suite.n); }
  
  int Runner::run(const testcase* suite, size_t N) {
    cases += N;
    for (size_t i=0; i<N; ++i) {
      const testcase& c = suite[i];
      std::cout << "RUNNING " << c.spec << "..." << std::flush;
      try {
	c.f();
	std::cout << erase << green << "OK    " << reset << c.spec << '\n';
      }
      catch (const failure& e) {
	std::cout << erase << red << "FAIL  " << reset << c.spec
		  << ' ' << red << e.file << ':' << e.line
		  << ' ' << e.what() << reset << '\n';
	++fails;
      }
      catch (const std::exception& e) {
	std::cout << erase << yellow << "ERROR " << reset << c.spec
		  << ' ' << yellow << e.what() << reset << '\n';
	++errors;
      }
    }

    return errors + fails;
  }

  int run(const testsuite* body, size_t n) {
    Runner r;
    for (size_t i=0; i<n; ++i)
      r.run(body[i]);

    const int N = r.cases;
    if (0 == r.errors + r.fails) {
      std::cout << igreen << bar(1, "ALL ", N, " tests PASSED")
		<< reset << '\n';
      return EXIT_SUCCESS;
    }

    if (r.fails)
      std::cout << ired << bar(r.fails/double(N), r.fails, ' ') << reset;
    if (r.errors)
      std::cout << iyellow << bar(r.errors/double(N), r.errors, " ERRORS ")
		<< reset;
    int passes = N - r.errors - r.fails;
    if (passes)
      std::cout << igreen << bar(passes/double(N), ' ', passes, " PASSES")
		<< reset;
    std::cout << '\n';
    return EXIT_FAILURE;
  }
} // namespace hunit
