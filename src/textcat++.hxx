#ifndef TEXTCAT_HXX
#define TEXTCAT_HXX
#include <iterator>

namespace textcat {
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
    friend bool operator<(ngram<N> const& n1, ngram<N> const& n2);
    friend bool operator==(ngram<N> const& n1, ngram<N> const& n2);
    template<typename Os> Os& operator<<(Os& os, ngram<N> const& n);
    typedef __int128 data_type;
    data_type data;
  };
  bool operator<(ngram<N> const& n1, ngram<N> const& n2)
  { return n1.data < n2.data; }
  bool operator==(ngram<N> const& n1, ngram<N> const& n2)
  { return n1.data == n2.data; }
  
  template<typename Os> Os& operator<<(Os& os, ngram<N> const& n) { 
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
namespace std {
  namespace tr1 {
    template<unsigned char N> hash<textcat::ngram<N> > {
      std::size_t operator()(textcat::ngram<N> const& ng) const 
      { return ng.hash() ; }
    };
  }
}
namespace textcat {

  template<unsigned char N, typedef SizeType= std::size_t>
  struct counts {
    typedef ngram<N> ngram_type;
    typedef SizeType size_type;
    typedef std::tr1::map<ngram_type, size_type> map_type;
    typedef typename map_type::value_type value_type;
    typedef std::vector<value_type> fingerprint_type;
  };
  
  template<unsigned char N, typename In> 
  typename counts<N>::map_type make_counts(In b, In e, std::size_t max_n){
    typename counts<N>::map_type counts;
    typedef typename counts<N>::ngram_type ngram_type;
      ngram_type ng;
      std::size_t n;
      for( n=0, ng(U'_'), was_invalid=true; (b != e) && (n != max_ngrams)
             ; was_invalid= invalid) {
        char32_t const new_codepoint(to_normal(from_utf8(b, e)));
        invalid= (new_char == U'_');
        if(! (invalid && was_invalid)) {
          ng(new_codepoint);
          ++counts[ng];
          ++n;
        }
        if(invalid && !was_invalid)
          { ng= ngram_type(U'_');}
      }
      return counts;
  }
  
  template<unsigned char N> 
  typename counts<N>::fingerprint_type
  to_vector(typename counts<N>::map_type const& c, std::size_t maxngrams){
    typedef typename counts<N>::value_type value_type;
    typename counts<N>::fingerprint_type result(c.begin(), c.end());
    typedef typename value_type::second_type size_type;
    if(maxngrams > tmp.size()) { maxngrams= tmp.size(); }
    std::partial_sort(tmp.begin(), tmp.begin()+maxngrams, tmp.end()
                      , std::tr1::bind(std::greater<size_type>()
                                       , std::tr1::bind<size_type>(&pair_type::second
                                                                   ,std::tr1::placeholders::_1)
                                       , std::tr1::bind<size_type>(&pair_type::second
                                                                   ,std::tr1::placeholders::_2)));
    result.resize(maxngrams);
    return result;
  }
  
  template<typename InOut> InOut count_to_rank(InOut b, InOut e){
    if(b!= e){
      for(std::iterator_traits<InOut>::value_type last((*b).second), rnk(0)
            ; b != e; ++b) {
        if((*b).second != last){ last= (*b).second; ++rnk;}
        (*b).second= rnk;
      }
    }
    return b;
  }

// Input Stream data is supposed to be already sorted
// imbue maxngrams and >> ?
  template<unsigned char N, typename Is>   typename counts<N>::fingerprint_type read(Is& is, std::size_t maxngrams) {
    typename counts<N>::fingerprint_type result;
    std::string tmp;
    std::size_t count;
    while(std::getline(is, tmp, '\t') && (maxngrams--)){
      is >> count;
      if(!is) { break; }
      ngram<N> ng;
      result.push_back(std::for_each(tmp.begin(), tmp.end(), ng), count);
    }
    count_to_rank(result.begin(), result.end());
    return result;
  }

  template<unsigned char N, typename Os>   Os& operator<<(Os& os, typename counts<N>::fingerprint_type fp) {
    for(typename counts<N>::fingerprint_type::const_iterator it(fp.begin()); it != fp.end(); ++it){
      os<<(*it).first << '\t' << (*it).second << '\n';
    }
    return os;
  }

  template<unsigned char N >
  struct scorer {
    scorer(std::size_t out_of_place= 400
           , std::size_t max_score= std::numeric_limits<std::size_t>::max()
           , float treshold_ratio=1.03): 
      out_of_place(out_of_place), max_score(max_score)
      , cutoff(max_score), treshold_ratio(treshold_ratio){}
    
    std::size_t operator()(typename score<N>::fingerprint_type const& f1
                           ,typename score<N>::fingerprint_type const& f2){
      typedef typename score<N>::fingerprint_type fp_type;
      std::size_t result(0);
      for(typename fp_type::const_iterator it1(f1.begin()), it2(f2.begin())
            ; (it1 != f1.end()) && (it2 != f2.end()) && (result != max_score)
            ; ) {
        if( (*it1).first < (*it2).first) { ++it1; }
        else {
          result += ((*it1).first == (*it2).first)
            ? std::abs((*it1++).second - (*it2).second);
          : out_of_place;
          if(result > cutoff) { result= max_score; }
          ++it2;
        }
      }
      cutoff= std::min(cutoff, result* treshold_ratio);// check no overflow
      return result;
    }
    std::size_t const out_of_place, max_score;
    std::size_t cutoff;
    float treshold_ratio;
  };

  template<unsigned char N> struct classifier {
    template<typename In> classifier(In b, In e)
      std::size_t score( typename counts<N>::fingerprint_type const& 
                         std::array<CountsType::mapped_type::size * 4> string_type;

  struct fingerprint {
    template<typename In>
    fingerprint(In b, In e, std::size_t max_ngrams){
      bool invalid, was_invalid;
      typedef ngram<5> ngram_type;
      ngram_type ng;
      std::size_t n;
      for( n=0, ng(U'_'), was_invalid=true; (b != e) && (n != max_ngrams); was_invalid= invalid) {
        char32_t const new_codepoint(to_normal(from_utf8(b, e)));
        invalid= (new_char == U'_');
        if(! (invalid && was_invalid)) {
          ng(new_codepoint);
          ++counts[ng];
          ++n;
        }
        if(invalid && !was_invalid)
          { ng= ngram_type(U'_');}
      }
    }
    template<typename Out> Out ranks(Out o ) const {
      std::vector<counts_type::value_type>
      std::copy(counts.begin(), counts.end(),
    }
  };
  template<typename Os> Os& operator<<(Os& os, fingerprint const& fp){
    return os;
  }
}
#endif
