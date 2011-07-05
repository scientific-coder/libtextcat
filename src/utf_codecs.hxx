#ifndef UTF_CODECS_HXX
#define UTF_CODECS_HXX
// good code  (C) Mathias Gaunard to appear in Boost::Unicode
// bugs (c) Bernard Hugueney, GPL v3+
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <boost/type_traits/make_unsigned.hpp> 
#include <boost/throw_exception.hpp>

#ifndef BOOST_NO_STD_LOCALE
#include <sstream>
#include <ios>
#endif

char32_t constexpr invalid_codepoint= (0x10FFFFu +1);
namespace {
  // (C) Mathias Gaunard in Boost::Unicode
  template<int UnitSize, typename Iterator>
  inline void invalid_utf_sequence(Iterator begin, Iterator end) {
    typedef typename std::iterator_traits<Iterator>::value_type ValueType;
    typedef typename boost::make_unsigned<ValueType>::type UnsignedType;
    
#ifndef BOOST_NO_STD_LOCALE
	std::stringstream ss;
	ss << "Invalid UTF-" << UnitSize << " sequence " << std::showbase << std::hex;
	for(Iterator it = begin; it != end; ++it) {
          //          std::size_t const tmp(static_cast<std::size_t>(static_cast<UnsignedType>(*it)));
          //          ss << tmp << " "; // bug in snapshot libstdc++ ???
        }
	ss << "encountered while trying to convert to UTF-32";
	std::out_of_range e(ss.str());
#else
	std::out_of_range e("Invalid UTF sequence encountered while trying to convert to UTF-32");
#endif
	boost::throw_exception(e);
  }
  template<typename In>
  void check(bool test, In begin, In end) {
    if(!test) { invalid_utf_sequence<8>(begin, end); }
  }
}
template<typename In> char32_t from_utf8(In& b, In e) {
  char32_t res(invalid_codepoint);
  In const backup(b);
  try {// silent exception : unchanged b signals an invalid utf-8 sequence
    if (b != e) {
      unsigned char const b0(*b++);
      //    std::cerr<<std::hex<<"reading utf8 :"<< b0 <<std::endl;
      if (!(b0 & 0x80)) { res= b0; }
      else {
        check(b!=e, b, e);
        unsigned char const b1(*b++);
        if( (b0 & 0xE0) == 0xC0) {
          res= ((b0 & 0x1F)<<6) | (b1 & 0x3F);
        }
        else {
          check(b!=e, b, e);
          unsigned char const b2(*b++);
          if ( (b0 & 0xF0u) == 0xE0u) {
            res= ((b0 & 0x0Fu) << 12) | ( (b1 & 0x3Fu) << 6) | (b2 & 0x3Fu) ;
          } else {
            check(b!=e, b, e);
            unsigned char const b3(*b++);
            if( (b0 & 0xF8) == 0xF0) {
              res= ((b0 & 0x07u) << 18) | ((b1 & 0x3Fu) << 12) | ((b2 & 0x3Fu) << 6) | (b3 & 0x3Fu);
            }
          }
        }
      }
    }
  } catch (std::out_of_range const&e) { b= backup; throw; }
  return res;
}

template<typename Out> Out to_utf8(char32_t c, Out out) {
  //  std::cerr<<std::hex<<"writing char32 :"<< c <<std::endl;
  // should we throw detail::invalid_code_point(c) otherwise ???
  if (c <= 0x10FFFu) {
    if (c < 0x80u) {
      //  std::cerr<<"writing "<<static_cast< char>(c)<<std::endl;
      *out++= static_cast<unsigned char>(c);
    } else if (c < 0x800u) {
      *out++= static_cast<unsigned char>(0xC0u + (c >> 6));
      *out++= static_cast<unsigned char>(0x80u + (c & 0x3Fu));
    } else if (c < 0x10000u) {
      *out++ = static_cast<unsigned char>(0xE0u + (c >> 12));
      *out++ = static_cast<unsigned char>(0x80u + ((c >> 6) & 0x3Fu));
      *out++ = static_cast<unsigned char>(0x80u + (c & 0x3Fu));
    } else {
      *out++ = static_cast<unsigned char>(0xF0u + (c >> 18));
      *out++ = static_cast<unsigned char>(0x80u + ((c >> 12) & 0x3Fu));
      *out++ = static_cast<unsigned char>(0x80u + ((c >> 6) & 0x3Fu));
      *out++ = static_cast<unsigned char>(0x80u + (c & 0x3Fu));
    }
  }
  return out;
}

template<typename Out> Out& operator<<(Out& o, char32_t c){
  to_utf8(c, std::ostream_iterator<char>(o, ""));
  return o;
}

template<typename Out, std::size_t N> Out& operator<<(Out& o, std::array<char32_t, N> const& a){
  o<<"[";
  std::for_each(a.begin(), a.end(), [&o](char32_t c){ to_utf8(c, std::ostream_iterator<char>(o, "")); o<<",";});
  o<<"]";
  //  std::copy(a.begin(), a.end(), std::ostream_iterator<char32_t>(o, ","));
  return o;
}

/*
 */
template<std::size_t K, std::size_t N, typename T, typename Op> struct array_lexicographical_helper : std::binary_function<std::array<T, N>, std::array<T, N>, bool> {
  array_lexicographical_helper(Op o=Op()) : op(o) {}
  bool operator()(std::array<T, N> const& a1, std::array<T, N> const& a2) const 
  { return op(a1[K], a2[K]) ? true : array_lexicographical_helper<K+1, N, T, Op>(op)(a1, a2) ; }
  Op op;
};
template<std::size_t N, typename T, typename Op> struct array_lexicographical_helper<N, N, T, Op> : std::binary_function<std::array<T, N>, std::array<T, N>, bool> {
  array_lexicographical_helper(Op o=Op()) : op(o) {}
  bool operator()(std::array<T, N> const& a1, std::array<T, N> const& a2) const { return false;}
  Op op;
};
// providing an operator[] to ngram would allow direct use on ngrams but maybe acces would be too slow anyway
template<typename T, std::size_t N,  typename Op= std::less<T> > // bool LittleEndian= true ? or "included" in char32_t
struct array_less : std::binary_function<std::array<T, N>, std::array<T, N>, bool> {
  array_less(Op o=Op()): op(o) {}
  bool operator()(std::array<char32_t, N> const& a1, std::array<char32_t, N> const& a2) const 
#ifdef TEMPLATED_UNROLL
  { return array_lexicographical_helper<0, N, char32_t, Op>(op)(a1, a2); }
#else
  { return std::lexicographical_compare(a1.begin(), a1.end(), a2.begin(), a2.end(), op); }
#endif
  Op op;
};

#endif
