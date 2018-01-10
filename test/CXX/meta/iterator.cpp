#include <cppx/meta>
// #include <cppx/compiler>

// #include <iostream>
// #include <typeinfo>

using namespace cppx;

namespace N {
  void f1();
  void f2();
  void f3();
  void f4();
}

constexpr int count_members(meta::object x) {
  meta::iterator first = begin(x);
  meta::iterator last = end(x);
  int n = 0;
  while (first != last) { 
    ++first;
    ++n;
  }
  return n;
}

int main(int argc, char* argv[]) {
  static_assert(count_members(reflexpr(N)) == 4);
}
