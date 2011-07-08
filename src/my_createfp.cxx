#include <iterator>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

#include "textcat++.hxx"

int main(int argc, char* argv[]){
  typedef std::tuple<std::string, std::string> lang_info_type;
  std::vector<lang_info_type> languages;
  languages.push_back(lang_info_type(argv[1], argv[2]));
  textcat::classifier<5> cf(languages.begin(), languages.end());
  std::vector<std::string> results;
  std::istream_iterator<char> b(std::cin), e;
  cf(b, e, std::back_inserter(results));
  std::copy(results.begin(), results.end(), std::ostream_iterator<std::string>(std::cout, "\t"));
  std::cout << std::endl;
 return 0;
}
