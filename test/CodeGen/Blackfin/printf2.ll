; RUN: llvm-as < %s | llc -march=bfin

declare i32 @printf(i8*, ...)

define i32 @main() {
	%1 = call i32 (i8*, ...)* @printf(i8* undef, i1 undef)
	ret i32 0
}
