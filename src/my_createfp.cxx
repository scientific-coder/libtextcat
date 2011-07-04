#include <iterator>
#include <iostream>

#include "textcat++.hxx"

int main(int argc, char* argv[]){
  textcat::fingerprint fp(std::istream_iterator<char>(std::cin), 400);
  std::cout<< fp << std::endl;
 return 0;
}
