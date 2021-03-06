; RUN: llvm-as < %s | llc -march=arm -mattr=+neon > %t
; RUN: grep {vsli\\.8} %t | count 2
; RUN: grep {vsli\\.16} %t | count 2
; RUN: grep {vsli\\.32} %t | count 2
; RUN: grep {vsli\\.64} %t | count 2
; RUN: grep {vsri\\.8} %t | count 2
; RUN: grep {vsri\\.16} %t | count 2
; RUN: grep {vsri\\.32} %t | count 2
; RUN: grep {vsri\\.64} %t | count 2

define <8 x i8> @vsli8(<8 x i8>* %A, <8 x i8>* %B) nounwind {
	%tmp1 = load <8 x i8>* %A
	%tmp2 = load <8 x i8>* %B
	%tmp3 = call <8 x i8> @llvm.arm.neon.vshiftins.v8i8(<8 x i8> %tmp1, <8 x i8> %tmp2, <8 x i8> < i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7 >)
	ret <8 x i8> %tmp3
}

define <4 x i16> @vsli16(<4 x i16>* %A, <4 x i16>* %B) nounwind {
	%tmp1 = load <4 x i16>* %A
	%tmp2 = load <4 x i16>* %B
	%tmp3 = call <4 x i16> @llvm.arm.neon.vshiftins.v4i16(<4 x i16> %tmp1, <4 x i16> %tmp2, <4 x i16> < i16 15, i16 15, i16 15, i16 15 >)
	ret <4 x i16> %tmp3
}

define <2 x i32> @vsli32(<2 x i32>* %A, <2 x i32>* %B) nounwind {
	%tmp1 = load <2 x i32>* %A
	%tmp2 = load <2 x i32>* %B
	%tmp3 = call <2 x i32> @llvm.arm.neon.vshiftins.v2i32(<2 x i32> %tmp1, <2 x i32> %tmp2, <2 x i32> < i32 31, i32 31 >)
	ret <2 x i32> %tmp3
}

define <1 x i64> @vsli64(<1 x i64>* %A, <1 x i64>* %B) nounwind {
	%tmp1 = load <1 x i64>* %A
	%tmp2 = load <1 x i64>* %B
	%tmp3 = call <1 x i64> @llvm.arm.neon.vshiftins.v1i64(<1 x i64> %tmp1, <1 x i64> %tmp2, <1 x i64> < i64 63 >)
	ret <1 x i64> %tmp3
}

define <16 x i8> @vsliQ8(<16 x i8>* %A, <16 x i8>* %B) nounwind {
	%tmp1 = load <16 x i8>* %A
	%tmp2 = load <16 x i8>* %B
	%tmp3 = call <16 x i8> @llvm.arm.neon.vshiftins.v16i8(<16 x i8> %tmp1, <16 x i8> %tmp2, <16 x i8> < i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7 >)
	ret <16 x i8> %tmp3
}

define <8 x i16> @vsliQ16(<8 x i16>* %A, <8 x i16>* %B) nounwind {
	%tmp1 = load <8 x i16>* %A
	%tmp2 = load <8 x i16>* %B
	%tmp3 = call <8 x i16> @llvm.arm.neon.vshiftins.v8i16(<8 x i16> %tmp1, <8 x i16> %tmp2, <8 x i16> < i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15 >)
	ret <8 x i16> %tmp3
}

define <4 x i32> @vsliQ32(<4 x i32>* %A, <4 x i32>* %B) nounwind {
	%tmp1 = load <4 x i32>* %A
	%tmp2 = load <4 x i32>* %B
	%tmp3 = call <4 x i32> @llvm.arm.neon.vshiftins.v4i32(<4 x i32> %tmp1, <4 x i32> %tmp2, <4 x i32> < i32 31, i32 31, i32 31, i32 31 >)
	ret <4 x i32> %tmp3
}

define <2 x i64> @vsliQ64(<2 x i64>* %A, <2 x i64>* %B) nounwind {
	%tmp1 = load <2 x i64>* %A
	%tmp2 = load <2 x i64>* %B
	%tmp3 = call <2 x i64> @llvm.arm.neon.vshiftins.v2i64(<2 x i64> %tmp1, <2 x i64> %tmp2, <2 x i64> < i64 63, i64 63 >)
	ret <2 x i64> %tmp3
}

define <8 x i8> @vsri8(<8 x i8>* %A, <8 x i8>* %B) nounwind {
	%tmp1 = load <8 x i8>* %A
	%tmp2 = load <8 x i8>* %B
	%tmp3 = call <8 x i8> @llvm.arm.neon.vshiftins.v8i8(<8 x i8> %tmp1, <8 x i8> %tmp2, <8 x i8> < i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8 >)
	ret <8 x i8> %tmp3
}

define <4 x i16> @vsri16(<4 x i16>* %A, <4 x i16>* %B) nounwind {
	%tmp1 = load <4 x i16>* %A
	%tmp2 = load <4 x i16>* %B
	%tmp3 = call <4 x i16> @llvm.arm.neon.vshiftins.v4i16(<4 x i16> %tmp1, <4 x i16> %tmp2, <4 x i16> < i16 -16, i16 -16, i16 -16, i16 -16 >)
	ret <4 x i16> %tmp3
}

define <2 x i32> @vsri32(<2 x i32>* %A, <2 x i32>* %B) nounwind {
	%tmp1 = load <2 x i32>* %A
	%tmp2 = load <2 x i32>* %B
	%tmp3 = call <2 x i32> @llvm.arm.neon.vshiftins.v2i32(<2 x i32> %tmp1, <2 x i32> %tmp2, <2 x i32> < i32 -32, i32 -32 >)
	ret <2 x i32> %tmp3
}

define <1 x i64> @vsri64(<1 x i64>* %A, <1 x i64>* %B) nounwind {
	%tmp1 = load <1 x i64>* %A
	%tmp2 = load <1 x i64>* %B
	%tmp3 = call <1 x i64> @llvm.arm.neon.vshiftins.v1i64(<1 x i64> %tmp1, <1 x i64> %tmp2, <1 x i64> < i64 -64 >)
	ret <1 x i64> %tmp3
}

define <16 x i8> @vsriQ8(<16 x i8>* %A, <16 x i8>* %B) nounwind {
	%tmp1 = load <16 x i8>* %A
	%tmp2 = load <16 x i8>* %B
	%tmp3 = call <16 x i8> @llvm.arm.neon.vshiftins.v16i8(<16 x i8> %tmp1, <16 x i8> %tmp2, <16 x i8> < i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8, i8 -8 >)
	ret <16 x i8> %tmp3
}

define <8 x i16> @vsriQ16(<8 x i16>* %A, <8 x i16>* %B) nounwind {
	%tmp1 = load <8 x i16>* %A
	%tmp2 = load <8 x i16>* %B
	%tmp3 = call <8 x i16> @llvm.arm.neon.vshiftins.v8i16(<8 x i16> %tmp1, <8 x i16> %tmp2, <8 x i16> < i16 -16, i16 -16, i16 -16, i16 -16, i16 -16, i16 -16, i16 -16, i16 -16 >)
	ret <8 x i16> %tmp3
}

define <4 x i32> @vsriQ32(<4 x i32>* %A, <4 x i32>* %B) nounwind {
	%tmp1 = load <4 x i32>* %A
	%tmp2 = load <4 x i32>* %B
	%tmp3 = call <4 x i32> @llvm.arm.neon.vshiftins.v4i32(<4 x i32> %tmp1, <4 x i32> %tmp2, <4 x i32> < i32 -32, i32 -32, i32 -32, i32 -32 >)
	ret <4 x i32> %tmp3
}

define <2 x i64> @vsriQ64(<2 x i64>* %A, <2 x i64>* %B) nounwind {
	%tmp1 = load <2 x i64>* %A
	%tmp2 = load <2 x i64>* %B
	%tmp3 = call <2 x i64> @llvm.arm.neon.vshiftins.v2i64(<2 x i64> %tmp1, <2 x i64> %tmp2, <2 x i64> < i64 -64, i64 -64 >)
	ret <2 x i64> %tmp3
}

declare <8 x i8>  @llvm.arm.neon.vshiftins.v8i8(<8 x i8>, <8 x i8>, <8 x i8>) nounwind readnone
declare <4 x i16> @llvm.arm.neon.vshiftins.v4i16(<4 x i16>, <4 x i16>, <4 x i16>) nounwind readnone
declare <2 x i32> @llvm.arm.neon.vshiftins.v2i32(<2 x i32>, <2 x i32>, <2 x i32>) nounwind readnone
declare <1 x i64> @llvm.arm.neon.vshiftins.v1i64(<1 x i64>, <1 x i64>, <1 x i64>) nounwind readnone

declare <16 x i8> @llvm.arm.neon.vshiftins.v16i8(<16 x i8>, <16 x i8>, <16 x i8>) nounwind readnone
declare <8 x i16> @llvm.arm.neon.vshiftins.v8i16(<8 x i16>, <8 x i16>, <8 x i16>) nounwind readnone
declare <4 x i32> @llvm.arm.neon.vshiftins.v4i32(<4 x i32>, <4 x i32>, <4 x i32>) nounwind readnone
declare <2 x i64> @llvm.arm.neon.vshiftins.v2i64(<2 x i64>, <2 x i64>, <2 x i64>) nounwind readnone
