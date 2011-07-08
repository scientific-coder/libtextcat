#ifndef TEXTCAT_HXX
#define TEXTCAT_HXX
#include <iterator>
#include <iostream>
#include <fstream>
#include <tuple>
#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>

extern "C" {
  //:TODO: adjust configure
#include <unicode/uchar.h>
}
#include "utf_codecs.hxx"
#include "ranks.hxx"
/*
  various data structures :
ngrams : compact array of bitfields :TODO: make a generic reusable template: how to specialize std::rotate for it ?
hash_table of ngrams and counts
vector of ngrams and counts
vector or ngrams and ranks : fingerprints
vector of fingerprints and names : smart pointers ? iterators ? how to avoid copy ?
ony one needed per app, maybe no copyes at all
 */
namespace textcat {

  char32_t constexpr placeholder_char= U'_'; 
  char32_t to_normal(char32_t c) { return u_isalpha(c) ? u_toupper(c) : placeholder_char; } //u_isUAlphabetic

  // storing ngrams for 1 <= n <= N  
  template<unsigned char N>
  struct ngram {
    static unsigned char const size=N;
    ngram(char32_t init=0):data(static_cast<data_type>(init)){}
    char32_t operator()(char32_t to_push) {
      char32_t const to_pop( data >> (21*(N-1)));
      data= ((data << 21) & ( (static_cast<data_type>(1)<< (21*N))-1)) 
        | static_cast<data_type>(to_push);
      return to_pop;
    }
    std::size_t hash() const {
      return static_cast<std::size_t>(data) 
        ^ static_cast<std::size_t>(data>>(8*sizeof(std::size_t)));
    }
    template<unsigned char NN> friend  bool operator<(ngram<NN> const& n1, ngram<NN> const& n2);
    template<unsigned char NN> friend bool operator==(ngram<NN> const& n1, ngram<NN> const& n2);
    template<typename Os, unsigned char NN> friend Os& operator<<(Os& os, ngram<NN> const& n);
    typedef __int128 data_type;
    data_type data;
  };
  template<unsigned char N>
  bool operator<(ngram<N> const& n1, ngram<N> const& n2)
  { return n1.data < n2.data; }
  template<unsigned char N>
  bool operator==(ngram<N> const& n1, ngram<N> const& n2)
  { return n1.data == n2.data; }
  
  template<typename Os, unsigned char N> 
  Os& operator<<(Os& os, ngram<N> const& n) { 
    for(unsigned char i(0); i != N; ++i){
      char32_t const c (static_cast<char32_t>(n.data >> 21*i) & ((1 << 21)-1));
      if(c) { to_utf8(c, os);}
      else {
        os << '\0';
        break;
      }
    }
    return os;
  }
}

namespace std {
  template<unsigned char N> struct hash<textcat::ngram<N> > {
    std::size_t operator()(textcat::ngram<N> const& ng) const 
    { return ng.hash() ; }
  };
}

namespace textcat {

  template<unsigned char N, typename SizeType= std::size_t>
  struct counts {
    typedef ngram<N> ngram_type;
    typedef SizeType size_type;
    typedef std::unordered_map<ngram_type, size_type> map_type;
    typedef typename std::pair<ngram_type, size_type> value_type;
    typedef std::vector<value_type> fingerprint_type;
  };
  
  template<unsigned char N, typename In> 
  typename counts<N>::map_type make_counts(In b, In e, std::size_t max_ngrams){
    typename counts<N>::map_type counters;
    typedef typename counts<N>::ngram_type ngram_type;
    ngram_type ng;
    std::size_t n;
    bool was_invalid, invalid;
    for( n=0, ng(placeholder_char), was_invalid=true; (b != e) && (n != max_ngrams)
             ; was_invalid= invalid) {
        char32_t const new_codepoint(to_normal(from_utf8(b, e)));
        invalid= (new_codepoint == placeholder_char);
        if(! (invalid && was_invalid)) {
          ng(new_codepoint);
          ++counters[ng];
          ++n;
        }
        if(invalid && !was_invalid)
          { ng= ngram_type(placeholder_char);}
      }
      return counters;
  }
  
  template<unsigned char N> 
  typename counts<N>::fingerprint_type
  to_vector(typename counts<N>::map_type const& c, std::size_t maxngrams){
    typedef typename counts<N>::value_type value_type;
    typename counts<N>::fingerprint_type result(c.begin(), c.end());
    typedef typename value_type::second_type size_type;
    if(maxngrams > result.size()) { maxngrams= result.size(); }
    std::partial_sort(result.begin(), result.begin()+maxngrams, result.end()
                      ,[](value_type const& v1, value_type const& v2){ return v1.second > v2.second; });
    result.resize(maxngrams);
    return result;
  }
  
  template<typename InOut> InOut count_to_rank(InOut b, InOut e){
    if(b!= e){
      for(typename std::iterator_traits<InOut>::value_type::second_type last((*b).second), rnk(0)
            ; b != e; ++b) {
        if((*b).second != last){ last= (*b).second; ++rnk;}
        (*b).second= rnk;
      }
    }
    return b;
  }

// Input Stream data is supposed to be already sorted
// imbue maxngrams and >> ?
  template<unsigned char N, typename Is>   typename counts<N>::fingerprint_type read(Is& is, std::size_t maxngrams= std::numeric_limits<std::size_t>::max()) {
    typename counts<N>::fingerprint_type result;
    std::string tmp;
    std::size_t count;
    while(std::getline(is, tmp, '\t') && (maxngrams--)){
      is >> count;
      if(!is) { break; }
      ngram<N> ng;
      result.push_back(std::make_pair(std::for_each(tmp.begin(), tmp.end(), ng)
                                      , count));
    }
    count_to_rank(result.begin(), result.end());
    return result;
  }

  template<unsigned char N, typename Os>   Os& operator<<(Os& os, typename counts<N>::fingerprint_type fp) {
    std::for_each(fp.begin(), fp.end()
                  , [&os](typename counts<N>::value_type const& v)
                  {os << v.first << '\t' << v.second << '\n';});
    // for(typename counts<N>::fingerprint_type::const_iterator it(fp.begin()); it != fp.end(); ++it){
    //   os<<(*it).first << '\t' << (*it).second << '\n';
    // }
    return os;
  }

  struct scorer {
    scorer(float ratio=1.03, std::size_t oop= 400
           , std::size_t max_s= std::numeric_limits<std::size_t>::max()
           ): 
      out_of_place(oop), max_score(max_s)
      , cutoff(max_s), treshold_ratio(ratio){}
    template<unsigned char N>
    std::size_t operator()(typename counts<N>::fingerprint_type const& f1
                           , typename counts<N>::fingerprint_type const& f2){
      typedef typename counts<N>::fingerprint_type fp_type;
      std::size_t result(0);
      for(typename fp_type::const_iterator it1(f1.begin()), it2(f2.begin())
            ; (it1 != f1.end()) && (it2 != f2.end()) && (result != max_score)
            ; ) {
        if( (*it1).first < (*it2).first) { ++it1; }
        else {
          result += ((*it1).first == (*it2).first)
            ? std::abs((*(it1++)).second - (*it2).second)
          : out_of_place;
          if(result > cutoff) { result= max_score; }
          ++it2;
        }
      }
      cutoff= std::min(cutoff, static_cast<std::size_t>(result* treshold_ratio));// check no overflow
      return result;
    }
    std::size_t const out_of_place, max_score;
    std::size_t cutoff;
    float const treshold_ratio;
  };

  

  template<unsigned char N> struct classifier {
    typedef typename counts<N>::fingerprint_type fingerprint_type;
    typedef std::tuple<std::string, fingerprint_type> language_type;
    std::vector<language_type> languages;

    // for now, taking iterators to std::pairs of languagename, filename
    template<typename In> classifier(In b, In e) {
      std::transform(b, e, std::back_inserter(languages)
                     , [](typename std::iterator_traits<In>::value_type const& v)
                     -> language_type { 
                       std::ifstream ifs(std::get<1>(v));
                       return language_type(std::get<0>(v)
                                            , read<N>(ifs)); });
    }

    template<typename In, typename Out> Out operator()(In b, In e, Out o
                                                       , std::size_t max_read=std::numeric_limits<std::size_t>::max()) const {
      float const treshold_ratio (1.03);
      //:TODO: LRU caching of detected languages
      typename counts<N>::fingerprint_type fp(to_vector<N>(make_counts<N>(b, e, max_read), max_read));
      count_to_rank(fp.begin(), fp.end());
      typename counts<N>::fingerprint_type const& cfp(fp);
      std::vector<std::size_t> scores(languages.size()), idx(languages.size());
      scorer s(treshold_ratio);
      std::transform(languages.begin(), languages.end(), scores.begin()
                     ,[&s, &fp](language_type const& lang){ return s.operator()<N>(fp, std::get<1>(lang));});
      indexes(scores.begin(), scores.end(), idx.begin());
      std::size_t const best(scores[idx.front()]);
      for(auto it(idx.begin()); (it != idx.end()) || (scores[*it] <= best*treshold_ratio); ++it, ++o) 
        { *o= std::get<0>(languages[*it]); }
      return o;
    }
  };
}

  
#endif
