// RUN: clang-cc -fsyntax-only -verify %s

struct S; // expected-note{{forward declaration of 'struct S'}}
typedef int FOO();

@interface INTF
{
	struct F {} JJ;
	int arr[];  // expected-error {{field has incomplete type}}
	struct S IC;  // expected-error {{field has incomplete type}}
	struct T { // expected-note {{previous definition is here}}
	  struct T {} X;  // expected-error {{nested redefinition of 'T'}}
	}YYY; 
	FOO    BADFUNC;  // expected-error {{field 'BADFUNC' declared as a function}}
	int kaka;	// expected-note {{previous declaration is here}}
	int kaka;	// expected-error {{duplicate member 'kaka'}}
	char ch[];	// expected-error {{field has incomplete type}}
}
@end
