#ifndef TEXTCAT_HXX
#define TEXTCAT_HXX
#include <iterator>
namespace textcat {
  struct fingerprint {
    template<typename In>
    fingerprint(In src, std::size_t max_ngrams){
    }
  };
  template<typename Os> Os& operator<<(Os& os, fingerprint const& fp){
    return os;
  }
}
#endif
