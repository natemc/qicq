# qicq
C++ library inspired by q.  qicq depends on boost (any and hana), and the Makefile is currently mac-specific.

## Lambdas

qicq_lambda.h has a few macros for creating lambdas in a qish way:

```
#include <iostream>
#include <qicq_lambda.h>
using std::cout;

auto f = L2(x+y);       // L2 is a 2-arg lambda with args x and y
cout << f(2,3) << '\n'; // 5
```

## Infix

Dyadic (aka binary) function objects can be used infix by placing a forward slash before and behind:

```
#include <iostream>
#include <qicq.h>
#include <qicq_lambda.h>
using std::cout;

cout << 2/L2(x+y)/3 << '\n'; // 5
```

Support for pointers to functions, etc may be added soon if I need it.  Adding support for monadic (unary) functions in general requires c++17's is_callable trait, but the qicq function objects all support similar syntax:

```
cout << til/5 << '\n'; // 0 1 2 3 4
```

Beware, however: C++ precedence rules still apply, and you will often have to use () even for qicq's monadic functions.

## Atomic operations

qicq's containers respond to arithmetic and relational operators by automatically vectorizing:

```
#include <iostream>
#include <qicq.h>
using std::cout; // advise against "using namespace std" due to a few name conflicts
using namespace qicq;

cout << 1+til/5 << '\n';            // 1 2 3 4 5
cout << where/(0==til/5%2) << '\n'; // 0 2 4
```

## Indexing

qicq's containers can be indexed by containers using the at function:

```
cout << v("abcdefghij")/at/v((1,3,5),v(2,4,6)) << '\n'; // bdf ceg
```

## Adverbs

qicq supports the 7 adverbs of k and q:

```
cout << L1(2*x)/each/til(5) << '\n';                     // 0 2 4 6 8
cout << raze(v("abc")/join/right/left/v("def")) << '\n'; // ad ae af bd be bf cd ce cf
cout << v("abc")/join/both/v("def") << '\n';             // ad be cf
cout << L2(x+y)/over/til(10) << '\n';                    // 45
cout << L2(x+y)/scan/til(5) << '\n';                     // 0 1 3 6 10
cout << 4/L2(x-y)/prior/v(8,3,5) << '\n';                // 4 -5 2
```

The plan for extended overloads is to name them differently.  So far, qicq only supports converge:

```
cout << all/conv/(til(3)/L2(x*y)/left/til(3) << '\n'; // 0
```

The match function (which conv uses) does fuzzy matching for doubles.

## Containers

auto a = v(1,2,3);                 // vec<int>
auto b = d(v("a"_m,"b"_m),v(1,2)); // dict<sym,int>
auto c = t(1,"abc");               // tuple<int,const char*>

Indexing, arithmetic, and relational operators support dicts.  Tuples are mostly experimental at this point, but you can apply a function to a tuple of arguments:

```
auto p = [](const char* s, double x){
  std::ostringstream os; os << s << ':' << x; return os.str();};
cout << apply(p, t("ack", 3.14)) << '\n'; // ack 3.14
```

Further support throughout the library for these grows as I need it.

## Which functions are implemented?

The quickest way to see a list of functions implemented so far is to open up qicq.cpp.
