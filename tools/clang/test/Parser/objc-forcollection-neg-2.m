// RUN: clang-cc -fsyntax-only -verify %s

typedef struct objc_class *Class;
typedef struct objc_object {
 Class isa;
} *id;
    
            
@protocol P @end

@interface MyList
@end
    
@implementation MyList
- (unsigned int)countByEnumeratingWithState:  (struct __objcFastEnumerationState *)state objects:  (id *)items count:(unsigned int)stackcount
{
        return 0;
}
@end

@interface MyList (BasicTest)
- (void)compilerTestAgainst;
@end

@implementation MyList (BasicTest)
- (void)compilerTestAgainst {
    static i;// expected-warning {{type specifier missing, defaults to 'int'}}
        for (id el, elem in self)  // expected-error {{only one element declaration is allowed}}
           ++i;
        for (id el in self) 
           ++i;
	MyList<P> ***p;
        for (p in self)  // expected-error {{selector element type 'MyList<P> ***' is not a valid object type}}
           ++i;

}
@end

