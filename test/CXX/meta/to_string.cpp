// RUN: %clang -std=c++1z %s

#include <cppx/meta>

using namespace cppx;

extern "C" int printf(const char*, ...);

enum E {
  X, A, B, C
};


template<meta::object X, typename T>
const char* find_enumerator(T val)
{
  if constexpr (!is_null(X)) {
    if (valueof(X) == val)
      return name(X);
    return find_enumerator<next(X)>(val);
  }
  return nullptr;
}

template<typename T>
const char* enum_to_string(T val) {
  return find_enumerator<meta::front(reflexpr(T))>(val);
}

int main() {
  {  
    const char* s = enum_to_string(A);
    assert(s);
    printf("%s\n", s);
  }

  {  
    const char* s = enum_to_string(B);
    assert(s);
    printf("%s\n", s);
  }

  {  
    const char* s = enum_to_string(C);
    assert(s);
    printf("%s\n", s);
  }
}
