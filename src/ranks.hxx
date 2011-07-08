#ifndef RANKS_HXX
#define RANKS_HXX

#include <iterator>
#include <functional>
#include <algorithm>
#include <vector>

template<typename In, typename Comp, typename SizeType= std::size_t> 
struct indexed_binary_function 
: std::binary_function<typename Comp::result_type, SizeType, SizeType> {
  indexed_binary_function(In b, Comp c):begin(b), comp(c){}
  typename Comp::result_type operator()(SizeType i1, SizeType i2) const 
  { return comp(*(begin+i1), *(begin+i2));}
  In begin;
  Comp comp;
};

template<typename In, typename Out, typename Comp>
Out indexes(In b, In e, Out o, Comp comp) {
  std::vector<std::size_t> idx(e-b);
  for(std::size_t i(0); i != idx.size(); ++i)
    { idx[i]= i;}
  std::sort(idx.begin(), idx.end(), indexed_binary_function<In, Comp, std::size_t>(b, comp));
  return std::copy(idx.begin(), idx.end(), o);
}

template<typename In, typename Out> Out indexes(In b, In e, Out o) {
  return indexes(b, e, o, std::less<typename std::iterator_traits<In>::value_type>()); 
}


template<typename In, typename Out, typename Comp>
Out ranks(In b, In e, Out o, Comp comp) {
  std::vector<std::size_t> idx(e-b), rnks(e-b);
  indexes(b, e, idx.begin(), comp);
  for(std::size_t i(0); i != rnks.size(); ++i)
    { rnks[idx[i]]= i; }
  return std::copy(rnks.begin(), rnks.end(), o);
}

template<typename In, typename Out> Out ranks(In b, In e, Out o) {
  return ranks(b, e, o, std::less<typename std::iterator_traits<In>::value_type>()); 
}

#endif
