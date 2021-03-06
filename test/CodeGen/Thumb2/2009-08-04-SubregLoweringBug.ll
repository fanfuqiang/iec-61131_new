; RUN: llvm-as < %s | llc -mtriple=thumbv7-apple-darwin9 -mattr=+neon,+neonfp 
; RUN: llvm-as < %s | llc -mtriple=thumbv7-apple-darwin9 -mattr=+neon,+neonfp | grep fcpys | count 1
; rdar://7117307

	%struct.Hosp = type { i32, i32, i32, %struct.List, %struct.List, %struct.List, %struct.List }
	%struct.List = type { %struct.List*, %struct.Patient*, %struct.List* }
	%struct.Patient = type { i32, i32, i32, %struct.Village* }
	%struct.Results = type { float, float, float }
	%struct.Village = type { [4 x %struct.Village*], %struct.Village*, %struct.List, %struct.Hosp, i32, i32 }

define arm_apcscc void @get_results(%struct.Results* noalias nocapture sret %agg.result, %struct.Village* %village) nounwind {
entry:
	br i1 undef, label %bb, label %bb6.preheader

bb6.preheader:		; preds = %entry
	call void @llvm.memcpy.i32(i8* undef, i8* undef, i32 12, i32 4)
	br i1 undef, label %bb15, label %bb13

bb:		; preds = %entry
	ret void

bb13:		; preds = %bb13, %bb6.preheader
	%0 = fadd float undef, undef		; <float> [#uses=1]
	%1 = fadd float undef, 1.000000e+00		; <float> [#uses=1]
	br i1 undef, label %bb15, label %bb13

bb15:		; preds = %bb13, %bb6.preheader
	%r1.0.0.lcssa = phi float [ 0.000000e+00, %bb6.preheader ], [ %1, %bb13 ]		; <float> [#uses=1]
	%r1.1.0.lcssa = phi float [ undef, %bb6.preheader ], [ %0, %bb13 ]		; <float> [#uses=0]
	store float %r1.0.0.lcssa, float* undef, align 4
	ret void
}

declare void @llvm.memcpy.i32(i8* nocapture, i8* nocapture, i32, i32) nounwind
