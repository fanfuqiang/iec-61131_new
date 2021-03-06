// RUN: clang-cc -triple x86_64-apple-darwin9 -fobjc-gc -emit-llvm -o %t %s &&
// RUN: grep -e "objc_assign_weak" %t | grep -e "call" | count 6

__weak id* x;
id* __weak y;
id* __weak* z;

__weak id* a1[20];
id* __weak a2[30];
id** __weak a3[40];

int main()
{
	*x = 0;
	*y = 0;
        **z = 0;

        a1[3] = 0;
        a2[3] = 0;
        a3[3][4] = 0;
}

