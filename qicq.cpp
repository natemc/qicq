#include <qicq/qicq.h>

namespace qicq {
  detail::Hole hole;
  
  detail::Converge  conv;
  detail::Cross     cross;
  detail::Each      each;
  detail::EachLeft  left;
  detail::EachRight right;
  detail::EachBoth  both;
  detail::EachMany  many;
  detail::EachPrior prior;
  detail::Over      over;
  detail::Scan      scan;

  detail::All      all;
  detail::Amend    amend;
  detail::Any      any;
  detail::Asc      asc;
  detail::At       at;
  detail::Avg      avg;
  detail::Bin      bin;
  detail::Bool     bool_;
  detail::Cut      cut;
  detail::Deltas   deltas;
  detail::Desc     desc;
  detail::Differ   differ;
  detail::Distinct distinct;
  detail::Dot      dot;
  detail::Drop     drop;
  detail::Enlist   enlist;
  detail::Except   except;
  detail::Find     find;
  detail::First    first;
  detail::Flip     flip;
  detail::Gen      gen;
  detail::Group    group;
  detail::Iasc     iasc;
  detail::Idesc    idesc;
  detail::In       in;
  detail::Inter    inter;
  detail::Join     join;
  detail::Key      key;
  detail::Last     last;
  detail::Match    match;
  detail::Max      max;
  detail::Med      med;
  detail::Min      min;
  //detail::Next next_; TODO: implies nulls
  //detail::Prev prev_; TODO: implies nulls
  detail::Rank     rank;
  detail::Raze     raze;
  detail::Reverse  rev;
  detail::Rotate   rot;
  detail::Signum   signum;
  detail::Sublist  sublist;
  detail::Sum      sum;
  detail::Sums     sums;
  //detail::Sv sv TODO
  detail::Take     take;
  detail::Til      til;
  detail::Union    union_;
  detail::Value    val;
  //detail::Vs vs; TODO
  detail::Where    where;
  detail::Within   within;
  detail::Xbar     xbar;
} // namespace qicq
