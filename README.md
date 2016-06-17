# qicq
qicq ("kick") is a C++ library inspired by q.  qicq depends on boost (any and hana), and the Makefile is currently mac-specific.

## Lambdas

qicq_lambda.h has a few macros for creating lambdas in a qish way:

```
#include <iostream>
#include <qicq_lambda.h>
using std::cout;

auto f = L2(x+y);       // L2 creates a 2-arg lambda with args x and y
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

Adding support for monadic (unary) function objects in general requires c++17's is_callable trait, but the qicq function objects all support similar syntax:

```
cout << til/5 << '\n'; // 0 1 2 3 4
```

Beware, however: C++ precedence rules still apply, and you will often have to use ().

## Right-to-left expression evaluation

With C++ precedence and associativity rules, you can force right-to-left evaluation with compound assignment; qicq supports this with the /= operator:

```
auto x = sum/=7/take/=1.0/7; // x is not quite 1.0
```

Keep in mind that compound assignment has lower precedence than all other operators except the comma operator.  You will sometimes need () around the whole expression using /= operator:

```
cout << (sum/=7/take/=1.0/7) << '\n';
```

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

Containers are functions, so you can index a vec or dict using operator():

```
cout << v("abcdefghij")(4) << '\n';                   // e
cout << v("abcdefghij")(v((1,3,5),v(2,4,6))) << '\n'; // bdf ceg
cout << v(v(0,1,2),v(3,4,5))(1,1) << '\n';            // 4
cout << v(v(0,1,2),v(3,4,5))(v(0,1),v(0,2)) << '\n';  // (0 2) (3 5)
```

You cannot use / for indexing in this way, because / in this context means atomic division.  However, you can apply an adverb to a container using / e.g.

```
auto x = v(3,3)/take/v("abcdefghi");          // abc def ghi
assert(v(2,0)/(x/both)/v(0,2)/match/v("gc"));
```

qicq's containers can also be indexed using the at function:

```
cout << v("abcdefghij")/at/v((1,3,5),v(2,4,6)) << '\n'; // bdf ceg
```

## Adverbs

qicq supports the 7 adverbs of q:

```
cout << L1(2*x)/each/til(5) << '\n';                     // 0 2 4 6 8
cout << raze(v("abc")/join/right/left/v("def")) << '\n'; // ad ae af bd be bf cd ce cf
cout << v("abc")/join/both/v("def") << '\n';             // ad be cf
cout << L2(x+y)/over/til(10) << '\n';                    // 45
cout << L2(x+y)/scan/til(5) << '\n';                     // 0 1 3 6 10
cout << 4/L2(x-y)/prior/v(8,3,5) << '\n';                // 4 -5 2
```

For looping over more than two vectors in parallel, qicq has ```many```:

```
auto f = many(L3(x+y+z));
cout << f(v(1,2,3),v(10,20,30),v(100,200,300)) << '\n'; // 111 222 333
```

The plan for extended overloads is to name them differently.  So far, qicq only supports converge:

```
cout << all/conv/(til(3)/L2(x*y)/left/til(3) << '\n'; // 0
```

The match function (which conv uses) does fuzzy matching for doubles.

## Containers

```
auto a = v(1,2,3);                 // vec<int>
auto b = d(v("a"_m,"b"_m),v(1,2)); // dict<sym,int>
auto c = t(1,"abc");               // tuple<int,const char*>
```

Indexing, arithmetic, and relational operators support dicts.  Most of the functions implemented so far also support dicts:

```
cout << group/d(s/each/v("abcdefghijklmnopqrst"),
                v(4,0,2,1,2,1,2,3,2,4,1,0,2,4,1,2,0,1,1,2));
// 4| a j n
// 0| b l q
// 2| c e g i m p t
// 1| d f k o r s
// 3| ,h
```

Tuples are mostly experimental at this point and have limited support.  However, while you can't apply a function to a vec (yet), you can apply a function to a tuple:

```
auto p = [](const sym& s, double x){
  std::ostringstream os; os << s << ':' << x; return os.str();};
cout << p/apply/t(s("pi"),3.14) << '\n'; // pi:3.14
```

## Literals

You can create a symbol via either the s function or the _s literal:

```
assert(s("xyzzy") == "xyzzy"_s);
```

You can create a vec<bool> using _b:

```
assert(v(true,false,true) == 101_b);
```

## Which functions are implemented?

The quickest way to see a list of functions implemented so far is to open up qicq.cpp.
