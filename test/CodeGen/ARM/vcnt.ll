; RUN: llvm-as < %s | llc -march=arm -mattr=+neon > %t
; RUN: grep {vcnt\\.8} %t | count 2

define <8 x i8> @vcnt8(<8 x i8>* %A) nounwind {
	%tmp1 = load <8 x i8>* %A
	%tmp2 = call <8 x i8> @llvm.arm.neon.vcnt.v8i8(<8 x i8> %tmp1)
	ret <8 x i8> %tmp2
}

define <16 x i8> @vcntQ8(<16 x i8>* %A) nounwind {
	%tmp1 = load <16 x i8>* %A
	%tmp2 = call <16 x i8> @llvm.arm.neon.vcnt.v16i8(<16 x i8> %tmp1)
	ret <16 x i8> %tmp2
}

declare <8 x i8>  @llvm.arm.neon.vcnt.v8i8(<8 x i8>) nounwind readnone
declare <16 x i8> @llvm.arm.neon.vcnt.v16i8(<16 x i8>) nounwind readnone
