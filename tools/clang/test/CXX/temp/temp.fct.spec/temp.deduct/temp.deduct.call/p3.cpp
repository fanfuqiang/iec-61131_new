// RUN: clang-cc -fsyntax-only -verify %s

template<typename T> struct A { };

// Top-level cv-qualifiers of P's type are ignored for type deduction.
template<typename T> A<T> f0(const T);

void test_f0(int i, const int ci) {
  A<int> a0 = f0(i);
  A<int> a1 = f0(ci);
}

// If P is a reference type, the type referred to by P is used for type 
// deduction.
template<typename T> A<T> f1(T&);

void test_f1(int i, const int ci, volatile int vi) {
  A<int> a0 = f1(i);
  A<const int> a1 = f1(ci);
  A<volatile int> a2 = f1(vi);
}

template<typename T, unsigned N> struct B { };
template<typename T, unsigned N> B<T, N> g0(T (&array)[N]);

void test_g0() {
  int array0[5];
  B<int, 5> b0 = g0(array0);
  const int array1[] = { 1, 2, 3};
  B<const int, 3> b1 = g0(array1);
}

template<typename T> B<T, 0> g1(const A<T>&);

void test_g1(A<float> af) {
  B<float, 0> b0 = g1(af);
  B<int, 0> b1 = g1(A<int>());
}

//   - If the original P is a reference type, the deduced A (i.e., the type
//     referred to by the reference) can be more cv-qualified than the 
//     transformed A.
template<typename T> A<T> f2(const T&);

void test_f2(int i, const int ci, volatile int vi) {
  A<int> a0 = f2(i);
  A<int> a1 = f2(ci);
  A<volatile int> a2 = f2(vi);
}

//   - The transformed A can be another pointer or pointer to member type that 
//     can be converted to the deduced A via a qualification conversion (4.4).
template<typename T> A<T> f3(T * * const * const);

void test_f3(int ***ip, volatile int ***vip) {
  A<int> a0 = f3(ip);
  A<volatile int> a1 = f3(vip);
}
                             
//   - If P is a class, and P has the form template-id, then A can be a 
//     derived class of the deduced A. Likewise, if P is a pointer to a class
//     of the form template-id, A can be a pointer to a derived class pointed 
//     to by the deduced A.
template<typename T, int I> struct C { };

struct D : public C<int, 1> { };
struct E : public D { };
struct F : A<float> { };

template<typename T, int I>
  C<T, I> *f4a(const C<T, I>&);
template<typename T, int I>
  C<T, I> *f4b(C<T, I>);
template<typename T, int I>
  C<T, I> *f4c(C<T, I>*);
int *f4c(...);

void test_f4(D d, E e, F f) {
  C<int, 1> *ci1a = f4a(d);
  C<int, 1> *ci2a = f4a(e);
  C<int, 1> *ci1b = f4b(d);
  C<int, 1> *ci2b = f4b(e);
  C<int, 1> *ci1c = f4c(&d);
  C<int, 1> *ci2c = f4c(&e);
  int       *ip1 = f4c(&f);
}
