// RUN: %clang_cc1 -triple i386-pc-unknown -fsyntax-only -Wstrict-prototypes -verify %s
// RUN: %clang_cc1 -triple i386-pc-unknown -fsyntax-only -Wstrict-prototypes -fdiagnostics-parseable-fixits %s 2>&1 | FileCheck %s

// function declaration with unspecified params
void foo1(); // expected-warning {{this function declaration is not a prototype}}
             // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:11-[[@LINE-1]]:11}:"void"
// function declaration with 0 params
void foo2(void);

// function definition with 0 params(for both cases),
// valid according to 6.7.5.3/14
void foo1() {}
void foo2(void) {}

// function type typedef unspecified params
typedef void foo3(); // expected-warning {{this function declaration is not a prototype}}
                     // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:19-[[@LINE-1]]:19}:"void"

// global fp unspecified params
void (*foo4)(); // expected-warning {{this function declaration is not a prototype}}
                // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:14-[[@LINE-1]]:14}:"void"

// struct member fp unspecified params
struct { void (*foo5)(); } s; // expected-warning {{this function declaration is not a prototype}}
                              // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:23-[[@LINE-1]]:23}:"void"

// param fp unspecified params
void bar2(void (*foo6)()) { // expected-warning {{this function declaration is not a prototype}}
                            // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:24-[[@LINE-1]]:24}:"void"
  // local fp unspecified params
  void (*foo7)() = 0; // expected-warning {{this function declaration is not a prototype}}
                      // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:16-[[@LINE-1]]:16}:"void"
  // array fp unspecified params
  void (*foo8[2])() = {0}; // expected-warning {{this function declaration is not a prototype}}
                           // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:19-[[@LINE-1]]:19}:"void"
}

// function type cast using using an anonymous function declaration
void bar3(void) {
  // casting function w/out prototype to unspecified params function type
  (void)(void(*)()) foo1; // expected-warning {{this function declaration is not a prototype}}
                          // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:18-[[@LINE-1]]:18}:"void"
  // .. specified params
  (void)(void(*)(void)) foo1;
}

// K&R function definition not preceded by full prototype
int foo9(a, b) // expected-warning {{old-style function definition is not preceded by a prototype}}
  int a, b;
{
  return a + b;
}

// Function declaration with no types
void foo10(); // expected-warning {{this function declaration is not a prototype}}
              // CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:12-[[@LINE-1]]:12}:"void"
// K&R function definition with incomplete param list declared
void foo10(p, p2) void *p; {} // expected-warning {{old-style function definition is not preceded by a prototype}}

// K&R function definition with previous prototype declared is not diagnosed.
void foo11(int p, int p2);
void foo11(p, p2) int p; int p2; {}

// PR31020
void __attribute__((cdecl)) foo12(d) // expected-warning {{this old-style function definition is not preceded by a prototype}}
  short d;
{}
