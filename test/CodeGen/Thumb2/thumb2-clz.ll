; RUN: llvm-as < %s | llc -march=thumb -mattr=+thumb2,+v7a | grep "clz " | count 1

define i32 @f1(i32 %a) {
    %tmp = tail call i32 @llvm.ctlz.i32(i32 %a)
    ret i32 %tmp
}

declare i32 @llvm.ctlz.i32(i32) nounwind readnone
