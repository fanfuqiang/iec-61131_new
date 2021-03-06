; RUN: llvm-as < %s | llc -march=arm -mattr=+neon > %t
; RUN: grep {vmov\\.s8} %t | count 2
; RUN: grep {vmov\\.s16} %t | count 2
; RUN: grep {vmov\\.u8} %t | count 2
; RUN: grep {vmov\\.u16} %t | count 2
; RUN: grep {vmov\\.32} %t | count 2

define i32 @vget_lanes8(<8 x i8>* %A) nounwind {
	%tmp1 = load <8 x i8>* %A
	%tmp2 = extractelement <8 x i8> %tmp1, i32 1
	%tmp3 = sext i8 %tmp2 to i32
	ret i32 %tmp3
}

define i32 @vget_lanes16(<4 x i16>* %A) nounwind {
	%tmp1 = load <4 x i16>* %A
	%tmp2 = extractelement <4 x i16> %tmp1, i32 1
	%tmp3 = sext i16 %tmp2 to i32
	ret i32 %tmp3
}

define i32 @vget_laneu8(<8 x i8>* %A) nounwind {
	%tmp1 = load <8 x i8>* %A
	%tmp2 = extractelement <8 x i8> %tmp1, i32 1
	%tmp3 = zext i8 %tmp2 to i32
	ret i32 %tmp3
}

define i32 @vget_laneu16(<4 x i16>* %A) nounwind {
	%tmp1 = load <4 x i16>* %A
	%tmp2 = extractelement <4 x i16> %tmp1, i32 1
	%tmp3 = zext i16 %tmp2 to i32
	ret i32 %tmp3
}

; Do a vector add to keep the extraction from being done directly from memory.
define i32 @vget_lanei32(<2 x i32>* %A) nounwind {
	%tmp1 = load <2 x i32>* %A
	%tmp2 = add <2 x i32> %tmp1, %tmp1
	%tmp3 = extractelement <2 x i32> %tmp2, i32 1
	ret i32 %tmp3
}

define i32 @vgetQ_lanes8(<16 x i8>* %A) nounwind {
	%tmp1 = load <16 x i8>* %A
	%tmp2 = extractelement <16 x i8> %tmp1, i32 1
	%tmp3 = sext i8 %tmp2 to i32
	ret i32 %tmp3
}

define i32 @vgetQ_lanes16(<8 x i16>* %A) nounwind {
	%tmp1 = load <8 x i16>* %A
	%tmp2 = extractelement <8 x i16> %tmp1, i32 1
	%tmp3 = sext i16 %tmp2 to i32
	ret i32 %tmp3
}

define i32 @vgetQ_laneu8(<16 x i8>* %A) nounwind {
	%tmp1 = load <16 x i8>* %A
	%tmp2 = extractelement <16 x i8> %tmp1, i32 1
	%tmp3 = zext i8 %tmp2 to i32
	ret i32 %tmp3
}

define i32 @vgetQ_laneu16(<8 x i16>* %A) nounwind {
	%tmp1 = load <8 x i16>* %A
	%tmp2 = extractelement <8 x i16> %tmp1, i32 1
	%tmp3 = zext i16 %tmp2 to i32
	ret i32 %tmp3
}

; Do a vector add to keep the extraction from being done directly from memory.
define i32 @vgetQ_lanei32(<4 x i32>* %A) nounwind {
	%tmp1 = load <4 x i32>* %A
	%tmp2 = add <4 x i32> %tmp1, %tmp1
	%tmp3 = extractelement <4 x i32> %tmp2, i32 1
	ret i32 %tmp3
}
