#include <cstdlib>
#include <hunit.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <qicq/qicq.h>
#include <qicq/qicq_lambda.h>
#include <qicq/qicq_math.h>
#include <qicq/qicq_sym.h>

namespace hana = boost::hana;
using namespace qicq;
using std::cout;
using std::ostringstream;
using std::string;

namespace {
#define SAME_TYPE(x,y) std::is_same<decltype(x),decltype(y)>::value
#define ASSERT_MATCH(x,y) do {						\
    auto p = (x);							\
    auto q = (y);							\
    if(!match(p,q)) {							\
      std::ostringstream os;						\
      os << #x " => " << p << "\n !!!!! " << #y << " => " << q;		\
      throw hunit::failure(__FILE__, __LINE__, os.str());		\
    }									\
  } while(0);
  
////////////////////////////////////////////////////////////////////////////////
  hunit::testcase adverb_tests[] = {
  // cout << raze(v("abc")/join/right/left/v("def")) << '\n';
  // cout << typeid(v("abc")/join/right/left/v("def")).name() << '\n';
    "each calls its lhs once for every element of its rhs", []{
      ASSERT(all/(v(0,2,4,6,8) == L1(2*x)/each/(til(5))));},
    "each works with all", []{
      ASSERT(all/(10_b == all/each/v(11_b,10_b)));},
    "right calls its lhs once for every element of its rhs", []{
      ASSERT(all/conv/(v(v(0,0,0),v(0,1,2),v(0,2,4)) ==
		       til/3/L2(x*y)/right/til(3)));},
    "left&right stack to make cross", []{
      ASSERT_MATCH(v(v(.12,.22),v(.22,.32)),
      		   L1(x/L2(x+y)/left/right/x)(v(.1,.2)-.04));},
    "both iterates over the lhs and rhs in parallel", []{
      ASSERT_MATCH(v(v(1,4),v(2,5),v(3,6)), v(1,2,3)/join/both/v(4,5,6));},
    "f/over/atom returns atom", []{
      ASSERT_MATCH(42, std::plus<int>()/over/42);},
    "f/over/vec reduces f over vec", []{
      ASSERT_MATCH(45LL, L2(x+y)/over/til(10))},
    "atom/f/over/vec reduces f over vec with atom as the initial value", []{
      ASSERT_MATCH(47, 2/std::plus<int>()/over/til(10))},
    "f/prior/vec computes f(vec[i],vec[i-1])", []{
      ASSERT_MATCH(v(8,-5,2), std::minus<int>()/prior/v(8,3,5));},
    "atom/f/prior/vec computes f(vec[i],vec[i-1]) w/initial value atom", []{
      ASSERT_MATCH(v(4,-5,2), 4/std::minus<int>()/prior/v(8,3,5));},
    "f/scan/atom returns atom", []{
      ASSERT_MATCH(42, std::plus<int>()/scan/42);},
    "f/scan/vec returns the prefix (f) over vec", []{
      // TODO investigate why the result is not v(0LL,...)
      ASSERT_MATCH(v(0,1,3,6,10), std::plus<int>()/scan/til(5));},
    "atom/f/scan/vec returns the prefix (f) over vec w/initial value atom", []{
      // TODO investigate why the result is not v(2LL,...)
      ASSERT_MATCH(v(2,3,5,8,12), 2/std::plus<int>()/scan/til(5));},
  };

  hunit::testcase all_tests[] = {
    "all is atomic", []{
      ASSERT_MATCH(010_b, all/v(111_b,010_b));},
    "all can be applied monadically", []{
      ASSERT(all/111_b);
      ASSERT(!(all/101_b));
    },
  };
  
  hunit::testcase asc_tests[] = {
    "asc/vec sorts vec", []{
      ASSERT_MATCH(v(0,1,2,3,4,6,8), asc/v(3,8,4,6,2,0,1));},
    "asc/vec<vec> sorts lexicographically", []{
      ASSERT_MATCH(v(v(1,1),v(3,8),v(6,2)), asc/v(v(3,8),v(6,2),v(1,1)));
    },
  };
  
  hunit::testcase at_tests[] = {
    "vec/at/atom returns vec[atom]", []{
      ASSERT_MATCH('z', v("xyzzy")/at/2);},
    "vec/at/ivec returns all elements of vec per ivec", []{
      ASSERT_MATCH(v("zy"), v("xyzzy")/at/v(2,4));},
    "at's result conforms to the index", []{
      ASSERT_MATCH(v(v("acegi"),v("bdfhj")),
		   v("abcdefghij")/at/v(2*til(5),1+2*til(5)));},
  };
  
  hunit::testcase cut_tests[] = {
    "lhs/cut/rhs breaks rhs per lhs", []{
      ASSERT_MATCH(v(v(0LL,1,2,3),v(4LL,5,6,7),v(8LL,9)),
		   v(0,4,8)/cut/til(10));
      ASSERT_MATCH(v(v(0LL,1,2,3),til/0,v(4LL,5,6,7),v(8LL,9)),
		   v(0,4,4,8)/cut/til(10));
    },
  };
  
  hunit::testcase desc_tests[] = {
    "desc/vec sorts vec desc", []{
      ASSERT_MATCH(v(8,6,4,3,2,1,0), desc/v(3,8,4,6,2,0,1));},
  };

  hunit::testcase dict_tests[] = {
    "arithmetic on dict w/atom or vec is atomic", []{
      ASSERT_MATCH(d(v("abc"),v(0,2,4)), 2*d(v("abc"),v(0,1,2)));
      ASSERT_MATCH(d(v("abc"),v(0,3,6)), d(v("abc"),v(0,1,2))*3);
      ASSERT_MATCH(d(v("abc"),v(3,6,9)), v(3,5,7)+d(v("abc"),v(0,1,2)));
      ASSERT_MATCH(d(v("abc"),v(2,5,8)), d(v("abc"),v(0,1,2))+v(2,4,6));
    },
    "relop on dict w/atom or vec is atomic", []{
      ASSERT_MATCH(d(v("abc"),001_b), 2==d(v("abc"),v(0,1,2)));
      ASSERT_MATCH(d(v("abc"),110_b), d(v("abc"),v(0,1,2))!=2);
      ASSERT_MATCH(d(v("abc"),100_b), v(3,5,7)<d(v("abc"),v(4,5,6)));
      ASSERT_MATCH(d(v("abc"),001_b), d(v("abc"),v(3,6,9))>v(8,8,8));
    },
    "arithmetic on dict/dict merges keys", []{
      ASSERT_MATCH(d(v("abcd"),v(1,3,5,7)),
		   d(v("abc"),v(1,2,3)) + d(v("bcd"),v(1,2,7)));
    },
    "dicts are functions", []{
      ASSERT_MATCH(3LL, d(v("abcde"),til/5)('d'));
      ASSERT_MATCH(v(1LL,3), d(v("abcde"),til/5)(v("bd")));
      ASSERT_MATCH(1LL, d(v("abcde"),til/each/til(5))('d',1));
      ASSERT_MATCH(v(0LL,2), d(v("abcde"),til/each/til(5))('d',v(0,2)));
      auto e = d(v("abcde"),til/5/rot/left/til(5));
      ASSERT_MATCH(v(v(1LL,3),v(3LL,0)), e(v("bd"),v(0,2)));
    },
  };
  
  hunit::testcase distinct_tests[] = {
    "distinct/vec preserves order", []{
      ASSERT_MATCH(v(1,3,5,6,4,7,9), distinct/v(1,1,3,5,6,3,4,7,5,9));},
  };
  
  hunit::testcase drop_tests[] = {
    "atom/drop/vec drops the first atom elements from vec", []{
      ASSERT_MATCH(v(2,3), 1/drop/v(1,2,3));},
    "-atom/drop/vec drops the last atom elements from vec", []{
      ASSERT_MATCH(v(1,2), -1/drop/v(1,2,3));},
  };

  hunit::testcase enlist_tests[] = {
    "enlist(atom) creates a vec with 1 element", []{
      ASSERT_MATCH(v(1), enlist/1);},
  };

  hunit::testcase except_tests[] = {
    "lhs/except/rhs returns (set diff) lhs-rhs", []{
      ASSERT_MATCH(v(1,2), v(1,2,3,4,5)/except/v(3,4,5,6,7));},
  };
  
  hunit::testcase find_tests[] = {
    "vec/find/atom returns the index of atom in vec", []{
      ASSERT_MATCH(2LL, v(1,3,5,7,9)/find/5);},
    "vec/find/atom returns vec.size() when atom is not in vec", []{
      ASSERT_MATCH(5LL, v(1,3,5,7,9)/find/0);},
    "vec/find/evec returns the indices of evec's elements in vec", []{
      ASSERT_MATCH(v(2LL,1), v(1,3,5,7,9)/find/v(5,3));},
  };

  hunit::testcase group_tests[] = {
    "group/vec creates a dict mapping vec's values to their indices", []{
      ASSERT_MATCH(d(v(4,0,2,1,3),
		     v(v(0LL,9,13),
		       v(1LL,11,16),
		       v(2LL,4,6,8,12,15,19),
		       v(3LL,5,10,14,17,18),
		       v(7LL))),
		   group/v(4,0,2,1,2,1,2,3,2,4,1,0,2,4,1,2,0,1,1,2));
    },
  };
  
  hunit::testcase iasc_tests[] = {
    "iasc/vec returns the index that will sort vec", []{
      ASSERT_MATCH(v(5LL,6,4,0,2,3,1), iasc/v(3,8,4,6,2,0,1));
      ASSERT_MATCH(v(0,1,2,3,4,6,8), L1(x/at/iasc(x))(v(3,8,4,6,2,0,1)));
    },
  };
  
  hunit::testcase idesc_tests[] = {
    "idesc/vec returns the index that will sort vec desc", []{
      ASSERT_MATCH(v(8,6,4,3,2,1,0), L1(x/at/idesc(x))(v(3,8,4,6,2,0,1)));},
  };
  
  hunit::testcase in_tests[] = {
    "atom/in/vec returns true iff atom is in vec", []{
      ASSERT(3/in/til(5));
      ASSERT(!(8/in/til(5)));
    },
    "lhs/in/vec follows the shape of lhs", []{
      ASSERT_MATCH(v(10_b,01_b), v(v(3,8),v(5,2))/in/til(5));
    },
  };
  
  hunit::testcase inter_tests[] = {
    "lhs/inter/rhs returns the set intersection of lhs and rhs", []{
      ASSERT_MATCH(v(3,4,5), v(1,2,3,4,5)/inter/v(3,4,5,6,7));},
  };
  
  hunit::testcase join_tests[] = {
    "join(atom,atom) creates a vec with 2 elements", []{
      ASSERT_MATCH(v(1,4.2), 1/join/4.2);},
    "join(vec,atom) appends atom to vec", []{
      ASSERT_MATCH(v(1,2,3,4.2), v(1,2,3)/join/4.2);},
    "join(vec,vec) concats the vecs", []{
      ASSERT_MATCH(1+til/6, v(1LL,2,3)/join/v(4,5,6));},
  };

  hunit::testcase match_tests[] = {
    "int/match/int is ==", []{
      ASSERT(4/match/4);
      ASSERT(!(4/match/3));
    },
    "match requires exact type equality", []{
	ASSERT(!(4/match/4.0));},
    "match is not atomic; it always returns bool", []{
      ASSERT(v(4,5)/match/v(4,5));
      ASSERT(!(v(4,5)/match/4));
      ASSERT(v(v(1,2,3),v(4,5,6))/match/v(v(1,2,3),v(4,5,6)));
    },
    "match accepts arguments whose shapes do not agree", []{
      ASSERT(!(v(4,5)/match/v(4)));
      ASSERT(!(v(v(1,2,3),v(4,5,6))/match/v(4,5)));
      ASSERT(!(v(v(1,2,3),v(4,5,6))/match/v(v(1,2,3),v(4,5,7))));
    },
    "match on doubles is fuzzy", []{
      ASSERT(1.0/0/match/(1.0/0));
      ASSERT(-1.0/0/match/(-1.0/0));
      ASSERT(!(1.0/0/match/(-1.0/0)));
      ASSERT(std::numeric_limits<double>::quiet_NaN()/match/
	     std::numeric_limits<double>::quiet_NaN());
      ASSERT((1.0!=sum(7/take/(1.0/7))) && 1.0/match/sum(7/take/(1.0/7)));
    },
  };
  
  hunit::testcase max_tests[] = {
    "max is atomic", []{
      ASSERT_MATCH(v(v(4,8,4,5),v(10,5,6,5)),
		   v(4,5)/max/v(v(3,8,3,5),v(10,1,6,3)));
      ASSERT_MATCH(v(v(4,8,4,5),v(10,5,6,5)),
		   v(v(3,8,3,5),v(10,1,6,3))/max/v(4,5));
    },
    "max can be applied monadically", []{
      ASSERT_MATCH(10, max(v(10,3,8,1,5)));},
  };
  
  hunit::testcase med_tests[] = {
    "med/vec, when vec.size() is odd, returns (double)vec's middle element", []{
      ASSERT_MATCH(3.0, med/v(1,4,5,3,2));},
    "med/vec, when vec.size() is even, returns avg(vec's middle elements)", []{
      ASSERT_MATCH(2.5, med/v(1,4,3,2));},
  };
  
  hunit::testcase min_tests[] = {
    "min is atomic", []{
      ASSERT_MATCH(v(v(3,4,3,4),v(5,1,5,3)),
		   v(4,5)/min/v(v(3,8,3,5),v(10,1,6,3)));
      ASSERT_MATCH(v(v(3,4,3,4),v(5,1,5,3)),
		   v(v(3,8,3,5),v(10,1,6,3))/min/v(4,5));
    },
    "min can be applied monadically", []{
      ASSERT_MATCH(1, min(v(10,3,8,1,5)));},
  };

  hunit::testcase not_tests[] = {
    "! is atomic", []{
      ASSERT_MATCH(01_b, !10_b);
      ASSERT_MATCH(v(01_b,10_b), !v(10_b,01_b));
    },
  };
  
  hunit::testcase rank_tests[] = {
    "rank/vec returns the relative rank of each element of vec", []{
      ASSERT_MATCH(v(3LL,6,4,5,2,0,1), rank/v(3,8,4,6,2,0,1));},
  };
  
  hunit::testcase raze_tests[] = {
    "raze/vec<vec> flattens one level", []{
      ASSERT_MATCH(v(1,2,3,4,5,6), raze/v(v(1,2,3),v(4,5,6)));
      ASSERT_MATCH(v(v(1,2),v(3,4),v(5,6),v(7,8)),
		   raze/v(v(v(1,2),v(3,4)),v(v(5,6),v(7,8))));
    },
  };
  
  hunit::testcase rev_tests[] = {
    "rev/vec reverses vec", []{
      ASSERT_MATCH(v("abc"), rev(v("cba")));},
  };
  
  hunit::testcase rot_tests[] = {
    "atom/rot/vec rotates vec left atom positions", []{
      ASSERT_MATCH(v("defghijabc"), 3/rot/v("abcdefghij"));},
    "-atom/rot/vec rotates vec right atom positions", []{
      ASSERT_MATCH(v("hijabcdefg"), -3/rot/v("abcdefghij"));},
    "atom/rot/vec over-rotates when atom > vec.size()", []{
      ASSERT_MATCH(v("cdefghijab"), 12/rot/v("abcdefghij"));
      ASSERT_MATCH(v("ijabcdefgh"), -12/rot/v("abcdefghij"));
    },
  };
  
  hunit::testcase signum_tests[] = {
    "signum/0 is zero", []{ ASSERT_MATCH(0, signum/0); },
    "signum/+ is one", []{ ASSERT_MATCH(1, signum/5); },
    "signum/- is -1", []{ ASSERT_MATCH(-1, signum/-3); },
    "signum/vec maps signum over vec's elements", []{
      ASSERT_MATCH(v(-1,-1,0,1,1), signum/v(-5,-1,0,1,5));},
  };

  hunit::testcase sublist_tests[] = {
    "atom/sublist/vec is take without the overtake", []{
      ASSERT_MATCH(v(0LL,1,2), 3/sublist/til(5));
      ASSERT_MATCH(v(2LL,3,4), -3/sublist/til(5));
      ASSERT_MATCH(til(5), 15/sublist/til(5));
      ASSERT_MATCH(til(5), -12/sublist/til(5));
    },
    "pair/sublist/vec takes pair[1] elements from starting from pair[0]", []{
      ASSERT_MATCH(v(1LL,2,3), v(1,3)/sublist/til(5));
    },
  };
  
  hunit::testcase sum_tests[] = {
    "sum/vec sums vec", []{ASSERT_MATCH(16, sum/v(8,3,5));},
    "sum can sum bools", []{ASSERT_MATCH(2, sum/11_b);},
    "sum is atomic", []{ASSERT_MATCH(v(3,7,11), sum/v(v(1,3,5),v(2,4,6)));},
  };
  
  hunit::testcase take_tests[] = {
    "int/take/int replicates the rhs", []{
      ASSERT_MATCH(v(47,47,47), 3/take/47);},
    "+int/take/vec takes the first int elements from vec", []{
      ASSERT_MATCH(v(10,20,30), 3/take/v(10,20,30,40));},
    "-int/take/vec takes the last int elements from vec", []{
      ASSERT_MATCH(v(20,30,40), -3/take/v(10,20,30,40));},
    "int/take/vec overtakes when int>vec.size()", []{
      ASSERT_MATCH(v(1,2,3,1,2,3,1,2,3,1), 10/take/v(1,2,3));},
    "-int/take/vec overtakes when int>vec.size()", []{
      ASSERT_MATCH(v(20,30,10,20,30), -5/take/v(10,20,30));},
    "pair/take/vec reshapes", []{
      ASSERT_MATCH(v(v(0LL,1),v(2LL,3)), v(2,2)/take/til(10));
      ASSERT_MATCH(v(v(0LL,1,2,3,4,5),v(6LL,7,8,9,0,1)), v(2,6)/take/til(10));
    },
    "v(r,0)/take/vec takes as many cols as needed to make r rows", []{
      ASSERT_MATCH(v(v(0LL,1,2,3),v(4LL,5,6,7)), v(2,0)/take/til(8));
      ASSERT_MATCH(v(v(0LL,1,2),v(3LL,4,5),v(6LL,7)), v(3,0)/take/til(8));
      ASSERT_MATCH(v(v(0LL,1),v(2LL,3),v(4LL,5),v(6LL),v(7LL),v(8LL),v(9LL)),
		   v(7,0)/take/til(10));
    },
    "v(0,c)/take/vec takes as many rows as needed to make c cols", []{
      ASSERT_MATCH(v(v(0LL,1,2,3,4,5,6),v(7LL,8,9)), v(0,7)/take/til(10));},
  };
  
  hunit::testcase vec_tests[] = {
    "v(int,double) yields vec<double>", []{
      ASSERT(SAME_TYPE(vec<double>(), v(1,1.5)));},
    "v(...,double) yields vec<double>", []{
      ASSERT_MATCH((vec<double>{0.0,1.0,2.0}), v(0, 1L, 2.0));},
    "atom*vec multiplies each element of vec by atom", []{
      ASSERT(all((v(0,2,4,6,8) == 2*til(5))));},
    "vec*atom multiplies each element of vec by atom", []{
      ASSERT(all((v(0,3,6) == til/3*3)));},
    "double*vec<int> promotes & multiplies each element of vec by atom", []{
      ASSERT(all((v(0,1.5,3,4.5,6) == 1.5*til(5))));},
    "vecs are functions", []{
      ASSERT_MATCH('d', v("abcdef")(3));
      ASSERT_MATCH(v("bdf"), v("abcdef")(v(1,3,5)));
      ASSERT_MATCH(3, v(v(0,1,2),v(3,4,5))(1,0));
      ASSERT_MATCH(v(3,5), v(v(0,1,2),v(3,4,5))(1,v(0,2)));
      ASSERT_MATCH(v(0,3), v(v(0,1,2),v(3,4,5))(v(0,1),0));
      ASSERT_MATCH(v(v(0,2),v(3,5)), v(v(0,1,2),v(3,4,5))(v(0,1),v(0,2)));
      ASSERT_MATCH(v("gc"), v(2,0)/(v(3,3)/take/v("abcdefghi")/both)/v(0,2));
    },
  };

  hunit::testcase union_tests[] = {
    "lhs/union_/rhs returns the set union of lhs and rhs", []{
      ASSERT_MATCH(v(1,2,3,4,5,6,7), v(1,2,3,4,5)/union_/v(3,4,5,6,7));},
  };
  
  hunit::testcase where_tests[] = {
    "where/vec<bool> returns the indices in vec that are true", []{
      ASSERT_MATCH(v(0LL,2), where/101_b);
      ASSERT_MATCH(v(0LL,2,4), where/(0==til/5%2));
    },
    "where/vec<integral> is x#'!#x", []{
      ASSERT_MATCH(v(1LL,2,2,3,3,3,4,4,4,4), where/til(5));},
  };
  
  hunit::testcase xbar_tests[] = {
    "atom/xbar/vec rounds vec's elements down to multiples of atom", []{
      ASSERT_MATCH(v(0LL,0,0,3,3,3,6,6,6,9), 3/xbar/til(10));},
  };

  int run_tests() {
    const hunit::testsuite suites[] = {
      adverb_tests,
      all_tests,
      asc_tests,
      at_tests,
      cut_tests,
      desc_tests,
      dict_tests,
      distinct_tests,
      drop_tests,
      enlist_tests,
      except_tests,
      find_tests,
      group_tests,
      iasc_tests,
      idesc_tests,
      in_tests,
      inter_tests,
      join_tests,
      match_tests,
      max_tests,
      med_tests,
      min_tests,
      not_tests,
      rank_tests,
      raze_tests,
      rev_tests,
      rot_tests,
      signum_tests,
      sublist_tests,
      sum_tests,
      take_tests,
      union_tests,
      vec_tests,
      where_tests,
      xbar_tests,
    };
    return hunit::run(suites);
  }
} // namespace

int main (int argc, const char* argv[]) {  
  // TODO automate these and write a bunch more
  /*
  //    V1(cout<<x<<' ')/each/til(3); cout << '\n';
  cout << deltas/v(8,3,5) << '\n';
  cout << 0/std::plus<int>()/over/00_b << '\n';
  cout << 0/L2(x+y)/over/11_b << '\n';
  cout << avg/v(8,3,5) << '\n';
  cout << differ/v(3,3,3) << '\n';
  sym foo("foo");
  sym bar("bar");
  cout << foo << '\t' << bar << '\n';
  debug_syms(cout);
  cout << (sym("foo") == foo) << (sym("bar") == bar) << '\n';
  cout << ("foo"_s == sym("foo")) << '\n';
  cout << d(v("a"_s,"bb"_s,"c"_s),v(1,2,3));
  cout << d(v("a"_s,"b"_s,"c"_s),v(1,2,3))+10;
  cout << d(v("a"_s,"b"_s,"c"_s),v(1,2,3))+v(10,20,30);
  cout << (d(v("a"_s,"b"_s,"c"_s),v(1,2,3))<
  d(v("d"_s,"b"_s,"c"_s),v(10,20,30)));
  cout << at(d(v("a"_s,"b"_s,"c"_s),v(1,2,3)), "b"_s) << '\n';
  V1(cout<<x<<'*')/each/d(v("a"_s,"bb"_s,"c"_s),v(1,2,3)); cout<<'\n';
  cout << asc/d(v("a"_s,"b"_s,"c"_s),v(3,2,1));
  cout << d(v("a"_s,"b"_s,"c"_s),v(3,2,1))/join/d(v("b"_s,"d"_s),v(10,20));
  cout << "++++++++++++++++++++++++++++++++++++++++\n";
  V1(cout<<x<<'*')/each/t(v(1,2,3),string("abc")); cout << '\n';
  cout << L1(x.size())/each/t(v(1,2,3),string("abc")) << '\n';
  cout << 25/L2(x+y)/over/t(v(1,2,3),v(40,50,60)) << '\n';
  cout << L2(x+y)/scan/t(v(1,2,3),v(40,50,60)) << '\n';
  cout << 25/L2(x+y)/scan/t(v(1,2,3),v(40,50,60)) << '\n';
  cout << L2(x-y)/prior/t(1,1,2,3,5,8) << '\n';
  cout << -5/L2(x-y)/prior/t(1,1,2,3,5,8) << '\n';
  cout << last/t(3.14,"foo") << '\n';

  cout << L2(x+y)/apply/t(3,4) << '\n';
  cout << max/apply/std::make_pair(3,4.5) << '\n';

  cout << cos(til/10) << '\n';
  */
  
  // run_tests();
  // cout << group/d(s/each/v("abcdefghijklmnopqrst"),
  // 		  v(4,0,2,1,2,1,2,3,2,4,1,0,2,4,1,2,0,1,1,2));
  // auto p = [](const char* s, double x){
  //   ostringstream os; os << s << ':' << x; return os.str();};
  // cout << p/apply/t("ack", 3.14) << '\n';
  // cout << d(v("a"_s,"b"_s,"c"_s),v(1,2,3)) +
  //   d(v("d"_s,"b"_s,"c"_s),v(10,20,30));
  //  return EXIT_SUCCESS;

  cout << 1_b << 0_b << '\n';
  return run_tests();
}
