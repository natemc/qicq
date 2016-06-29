# qicq
qicq ("kick") is a q-inspired C++ library.

### Features

* [Lambdas](#lambdas)
* [Infix](#infix)
* [Right-to-left expression evaluation](#r2l)
* [Uniform Application & Indexing](#uniform)
* [Atomic operations](#atomic)
* [Adverbs](#adverbs)
* [Containers](#containers)
* [Literals](#lit)
* [To do](#todo)

### Motivation

While [q](http://code.kx.com) was the inspiration for qicq, the graphics library [Cinder](https://libcinder.org) was the impetus.  Exploring Cinder, I repeatedly wished to be able to use vector operations to make the code simpler.  For example, using qicq along with a few wrappers to reduce the Cinder boilerplate slightly, [the easing example](https://libcinder.org/docs/guides/opengl/part3.html) (scroll down to the last example - the one that looks like a stack of rotating post-it notes) on the Cinder site can be made somewhat shorter, and the if and loops removed:

``` C++
struct BasicApp: app::App {
  BasicApp(int n_): n(n_) {}
  
  void setup() {
    using namespace ci::geom;
    auto shader  = lambertColor();
    auto slice   = Cube().size(1, 1.0f/n, 1);
    auto batches = L2(batch(slice>>Translate(x)>>Constant(COLOR,y), shader));
    auto rel     = til/n/float(n);
    slices       = L1(vec3(0,x,0))/each/rel/batches/both/=hue/each/rel;
    cam.look(vec3(2,3,2), vec3(0,.5f,0));
  }
  
  void draw() {
    constexpr float delay     = 0.25f;
    constexpr float rotTime   = 1.5f;
    constexpr float rotOffset = 0.1f;
    float totalTime = delay + rotTime + n*rotOffset;
    auto  start     = til/n*rotOffset;
    auto  end       = start+rotTime;
    float time      = fmod(app::getElapsedFrames() / 30.0f, totalTime);
    auto  rotation  = time/within/v(start,end)*(time-start)/rotTime;
    auto  angle     = f(ci::easeInOutQuint)/each/rotation*float(M_PI)/2.0f;
    g.clear().zbuf().matrices(cam);
    angleY/each/angle/V2(PMM(rot(x).draw(y)))/both/slices;
  }
    
private:
  const int n;
  Cam cam;
  vec<gl::BatchRef> slices;
};
```

It's still fairly verbose, but the code is straightforward, and we can think in terms of the whole problem rather than one piece at a time.

### Dependencies

qicq depends on boost (any and hana), and the Makefile is currently mac-specific.

<a id='lambdas'></a>
## Lambdas

qicq_lambda.h has a few macros for creating lambdas in a q-ish way:

``` C++
#include <iostream>
#include <qicq_lambda.h>
using std::cout; // advise against "using namespace std" due to a few name conflicts

auto f = L2(x+y);       // L2 creates a 2-arg lambda with args x and y
auto g = LA(p,q,p*q);   // LA creates a lambda w/user specified args (up to 4 as of this writing)
cout << f(2,3) << '\t' << g(4,5) << '\n'; // 5 20
```

<a id='infix'></a>
## Infix

Dyadic (aka binary) function objects can be used infix by placing a forward slash before and behind:

``` C++
#include <iostream>
#include <qicq.h>
#include <qicq_lambda.h>
using namespace qicq;
using std::cout;

cout << 2/L2(x+y)/3 << '\n'; // 5
```

This also works for monadic (unary) lambdas defined using the qicq lambda macros as well as function objects that inherit `qicq::Unary`, which includes all the functions in qicq.h:

``` C++
cout << til/5 << '\n';        // 0 1 2 3 4
cout << where/1001_b << '\n'; // 0 3
```

Function pointers are a builtin type, and it is impossible to redefine the behavior of C++ operators for builtin types (even when that behavior is illegal or undefined).  So, if you want to use a regular function infix, wrap it in a call to `f`, which will make a function object out of it:

``` C++
int add(int x, int y) { return x+y; }
..
cout << 3/f(add)/4 << '\n';           // 7
```

Beware: C++ precedence rules still apply, and you will sometimes find it easier to use ().

<a id='r2l'></a>
## Right-to-left expression evaluation

With C++ precedence and associativity rules, you can force right-to-left evaluation with compound assignment; qicq supports this via the `/=` operator:

``` C++
auto x = sum/=7/take/=1.0/7; // x is not quite 1.0
```

Keep in mind that the only opereator with lower precedence than the assignment operators is the comma operator.  So, when using `/=` in a subexpression you will usually need () around the subexpression:

``` C++
cout << (sum/=7/take/=1.0/7) << '\n';
```

<a id='atomic'></a>
## Atomic operations

qicq's vecs and dicts respond to arithmetic and relational operators by automatically vectorizing:

``` C++
#include <iostream>
#include <qicq.h>
using std::cout;
using namespace qicq;

cout << 1+til/5 << '\n';           // 1 2 3 4 5
cout << where(0==til/5%2) << '\n'; // 0 2 4
```

<a id='uniform'></a>
## Uniform Application & Indexing

Containers are functions, so you index them using `operator()`:

``` C++
cout << v("abcdefghij")(4) << '\n';                   // e
cout << v("abcdefghij")(v((1,3,5),v(2,4,6))) << '\n'; // bdf ceg
cout << v(v(0,1,2),v(3,4,5))(1,1) << '\n';            // 4
cout << v(v(0,1,2),v(3,4,5))(v(0,1),v(0,2)) << '\n';  // (0 2) (3 5)
cout << d(v("abcde"),til/5)('d') << '\n';             // 3
cout << t(v(1,2,3),string("abc"))(0_c) << '\n';       // 1 2 3
```

At the moment, you can only index one or two levels deep.

You cannot use `/` for indexing in this way, because `/` in this context means atomic division.  However, you can apply an adverb to a container using `/` e.g.

``` C++
auto x = v(3,3)/take/v("abcdefghi");          // abc def ghi
assert(v(2,0)/(x/both)/v(0,2)/match/v("gc"));
```

qicq's containers can also be indexed using the `at` function:

``` C++
cout << v("abcdefghij")/at/v((1,3,5),v(2,4,6)) << '\n'; // bdf ceg
```

qicq's `dot` function needs a tuple on the rhs when used to index a container, because the return type is different depending on the size of the rhs.

``` C++
cout << v(v("ace"),v("bdf"))/dot/t(v(0,1),v(2,1)) << '\n'; // ec fd
```

<a id='adverbs'></a>
## Adverbs

qicq supports the 7 adverbs of q:

``` C++
cout << L1(2*x)/each/til(5) << '\n';                     // 0 2 4 6 8
cout << raze(v("abc")/join/right/left/v("def")) << '\n'; // ad ae af bd be bf cd ce cf
cout << v("abc")/join/both/v("def") << '\n';             // ad be cf
cout << L2(x+y)/over/til(10) << '\n';                    // 45
cout << L2(x+y)/scan/til(5) << '\n';                     // 0 1 3 6 10
cout << 4/L2(x-y)/prior/v(8,3,5) << '\n';                // 4 -5 2
```

For looping over more than two vectors in parallel, qicq has `many`:

``` C++
auto f = many(L3(x+y+z));
cout << f(v(1,2,3),v(10,20,30),v(100,200,300)) << '\n'; // 111 222 333
```

I'm considering merging `both` and `many` into `each`; the reason to separate them is it reduces the number of overloads considered during lookup which makes compiler errors  shorter.

Meanwhile, `cross` is an adverb in qicq:

``` C++
cout << v(0,1)/join/cross/v(0,1,2) << '\n'; // (0 0) (0 1) (0 2) (1 0) (1 1) (1 2)
cout << L2(v(x,1,y))/cross/v(0,1) << '\n';  // (0 1 0) (0 1 1) (1 1 0) (1 1 1)
```

Functions are generally all different types, so you have to put them in a tuple (not a vec) if you intend to loop over them:

``` C++
cout << (t(min,max)/at/left/right/=v(3,3)/take/=til/9) << '\n'; // (0 2) (3 5) (6 8)
```

The plan for extended overloads is to name them differently.  So far, qicq only supports (a limited form of) converge:

``` C++
cout << all/conv/(til(3)/L2(x*y)/left/til(3) << '\n'; // 0
```

The `match` function (which `conv` uses) does fuzzy matching for doubles.

<a id='containers'></a>
## Containers

``` C++
auto a = v(1,2,3);                 // vec<int>
auto b = d(v("a"_m,"b"_m),v(1,2)); // dict<sym,int>
auto c = t(1,"abc");               // tuple<int,const char*>
```

Currently, vecs are supported wherever it makes sense.  Indexing, arithmetic, and relational operators support dicts.  Most of the functions implemented so far also support dicts:

``` C++
cout << group/d(s/each/v("abcdefghijklmnopqrst"),
                v(4,0,2,1,2,1,2,3,2,4,1,0,2,4,1,2,0,1,1,2));
// 4| a j n
// 0| b l q
// 2| c e g i m p t
// 1| d f k o r s
// 3| ,h
```

Tuples have limited support so far.  You can use `dot` to apply a function to a tuple:

``` C++
auto p = [](const sym& s, double x){
  std::ostringstream os; os << s << ':' << x; return os.str();};
cout << p/dot/t(s("pi"),3.14) << '\n'; // pi:3.14
```

You can also flip a `vec<tuple<T...>>` or a `tuple<vec<T>...>`:

``` C++
assert(v(t(1,'a'),t(2,'b'),t(3,'c'))/match/=flip/t(v(1,2,3),v("abc")));
```

<a id='lit'></a>
## Literals

You can create a symbol via either the `s` function or the `_s` literal:

``` C++
assert(s("xyzzy") == "xyzzy"_s);
```

You can create a `vec<bool>` using `_b`:

``` C++
assert(v(true,false,true) == 101_b);
```

<a id='todo'></a>
## To do

* Nulls
* Tables
* Improve compiler error messages (usually they are several pages)
* Completeness

One idea that may be worth exploring is wrapper types for all the builtin types (e.g., qicq::Int etc).  That would allow overloading operators in more interesting ways (in particular, it would be nice if & and | were dyadic min and max rather than the corresponding bit operations).  With user-defined literals (`_j`, `_f`, etc) it might not be as painful as it seems.

Also, no effort has been made to make qicq fast or to memory efficient.

#### Which functions are implemented?

The quickest way to see a list of functions implemented so far is to open up qicq.cpp.
