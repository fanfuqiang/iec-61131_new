; RUN: llvm-as < %s | llc -march=arm -mattr=+vfp2 | grep -E {fmscs\\W*s\[0-9\]+,\\W*s\[0-9\]+,\\W*s\[0-9\]+} | count 1
; RUN: llvm-as < %s | llc -march=arm -mattr=+neon,+neonfp | grep -E {fmscs\\W*s\[0-9\]+,\\W*s\[0-9\]+,\\W*s\[0-9\]+} | count 1
; RUN: llvm-as < %s | llc -march=arm -mattr=+neon,-neonfp | grep -E {fmscs\\W*s\[0-9\]+,\\W*s\[0-9\]+,\\W*s\[0-9\]+} | count 1
; RUN: llvm-as < %s | llc -march=arm -mcpu=cortex-a8 | grep -E {fmscs\\W*s\[0-9\]+,\\W*s\[0-9\]+,\\W*s\[0-9\]+} | count 1
; RUN: llvm-as < %s | llc -march=arm -mcpu=cortex-a9 | grep -E {fmscs\\W*s\[0-9\]+,\\W*s\[0-9\]+,\\W*s\[0-9\]+} | count 1

define float @test(float %acc, float %a, float %b) {
entry:
	%0 = fmul float %a, %b
        %1 = fsub float %0, %acc
	ret float %1
}

