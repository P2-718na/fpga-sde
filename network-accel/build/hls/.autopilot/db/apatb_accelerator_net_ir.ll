; ModuleID = 'C:/Users/Matteo/Desktop/repo/master-thesis/fpga-sde/network-accel/build/hls/.autopilot/db/a.g.ld.5.gdce.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-i64:64-i128:128-i256:256-i512:512-i1024:1024-i2048:2048-i4096:4096-n8:16:32:64-S128-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "fpga64-xilinx-none"

%"struct.ap_fixed<32, 8>" = type { %"struct.ap_fixed_base<32, 8>" }
%"struct.ap_fixed_base<32, 8>" = type { %"struct.ssdm_int<32, true>" }
%"struct.ssdm_int<32, true>" = type { i32 }

; Function Attrs: noinline willreturn
define void @apatb_accelerator_net_ir(i32* noalias nocapture nonnull readonly "maxi" %row_ptr_ddr, i16* noalias nocapture nonnull readonly "maxi" %col_idx_ddr, %"struct.ap_fixed<32, 8>"* noalias nocapture nonnull "maxi" %u_ddr, %"struct.ap_fixed<32, 8>"* noalias nocapture nonnull "maxi" %v_ddr, i32 %neuron_count, i32 %edge_count, i32 %timesteps, i32 %command, %"struct.ap_fixed<32, 8>"* nocapture readonly %dt, %"struct.ap_fixed<32, 8>"* nocapture readonly %a, %"struct.ap_fixed<32, 8>"* nocapture readonly %J, %"struct.ap_fixed<32, 8>"* nocapture readonly %inv_e, %"struct.ap_fixed<32, 8>"* nocapture readonly %sigma_sqrt_dt, %"struct.ap_fixed<32, 8>"* nocapture readonly %seed, i1 zeroext %reseed) local_unnamed_addr #0 {
entry:
  %row_ptr_ddr_copy = alloca i32, align 512
  %col_idx_ddr_copy = alloca i16, align 512
  %u_ddr_copy = alloca i32, align 512
  %v_ddr_copy = alloca i32, align 512
  call fastcc void @copy_in(i32* nonnull %row_ptr_ddr, i32* nonnull align 512 %row_ptr_ddr_copy, i16* nonnull %col_idx_ddr, i16* nonnull align 512 %col_idx_ddr_copy, %"struct.ap_fixed<32, 8>"* nonnull %u_ddr, i32* nonnull align 512 %u_ddr_copy, %"struct.ap_fixed<32, 8>"* nonnull %v_ddr, i32* nonnull align 512 %v_ddr_copy)
  call void @apatb_accelerator_net_hw(i32* %row_ptr_ddr_copy, i16* %col_idx_ddr_copy, i32* %u_ddr_copy, i32* %v_ddr_copy, i32 %neuron_count, i32 %edge_count, i32 %timesteps, i32 %command, %"struct.ap_fixed<32, 8>"* %dt, %"struct.ap_fixed<32, 8>"* %a, %"struct.ap_fixed<32, 8>"* %J, %"struct.ap_fixed<32, 8>"* %inv_e, %"struct.ap_fixed<32, 8>"* %sigma_sqrt_dt, %"struct.ap_fixed<32, 8>"* %seed, i1 %reseed)
  call void @copy_back(i32* %row_ptr_ddr, i32* %row_ptr_ddr_copy, i16* %col_idx_ddr, i16* %col_idx_ddr_copy, %"struct.ap_fixed<32, 8>"* %u_ddr, i32* %u_ddr_copy, %"struct.ap_fixed<32, 8>"* %v_ddr, i32* %v_ddr_copy)
  ret void
}

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @copy_in(i32* noalias readonly "unpacked"="0", i32* noalias align 512 "unpacked"="1", i16* noalias readonly "unpacked"="2", i16* noalias align 512 "unpacked"="3", %"struct.ap_fixed<32, 8>"* noalias readonly "unpacked"="4", i32* noalias nocapture align 512 "unpacked"="5.0", %"struct.ap_fixed<32, 8>"* noalias readonly "unpacked"="6", i32* noalias nocapture align 512 "unpacked"="7.0") unnamed_addr #1 {
entry:
  call fastcc void @onebyonecpy_hls.p0i32(i32* align 512 %1, i32* %0)
  call fastcc void @onebyonecpy_hls.p0i16(i16* align 512 %3, i16* %2)
  call fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>.14"(i32* align 512 %5, %"struct.ap_fixed<32, 8>"* %4)
  call fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>.14"(i32* align 512 %7, %"struct.ap_fixed<32, 8>"* %6)
  ret void
}

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @onebyonecpy_hls.p0i32(i32* noalias align 512 %dst, i32* noalias readonly %src) unnamed_addr #2 {
entry:
  %0 = icmp eq i32* %dst, null
  %1 = icmp eq i32* %src, null
  %2 = or i1 %0, %1
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  %3 = load i32, i32* %src, align 4
  store i32 %3, i32* %dst, align 512
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @onebyonecpy_hls.p0i16(i16* noalias align 512 %dst, i16* noalias readonly %src) unnamed_addr #2 {
entry:
  %0 = icmp eq i16* %dst, null
  %1 = icmp eq i16* %src, null
  %2 = or i1 %0, %1
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  %3 = load i16, i16* %src, align 2
  store i16 %3, i16* %dst, align 512
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @copy_out(i32* noalias "unpacked"="0", i32* noalias readonly align 512 "unpacked"="1", i16* noalias "unpacked"="2", i16* noalias readonly align 512 "unpacked"="3", %"struct.ap_fixed<32, 8>"* noalias "unpacked"="4", i32* noalias nocapture readonly align 512 "unpacked"="5.0", %"struct.ap_fixed<32, 8>"* noalias "unpacked"="6", i32* noalias nocapture readonly align 512 "unpacked"="7.0") unnamed_addr #3 {
entry:
  call fastcc void @onebyonecpy_hls.p0i32(i32* %0, i32* align 512 %1)
  call fastcc void @onebyonecpy_hls.p0i16(i16* %2, i16* align 512 %3)
  call fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>"(%"struct.ap_fixed<32, 8>"* %4, i32* align 512 %5)
  call fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>"(%"struct.ap_fixed<32, 8>"* %6, i32* align 512 %7)
  ret void
}

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>"(%"struct.ap_fixed<32, 8>"* noalias "unpacked"="0" %dst, i32* noalias nocapture readonly align 512 "unpacked"="1.0" %src) unnamed_addr #2 {
entry:
  %0 = icmp eq %"struct.ap_fixed<32, 8>"* %dst, null
  br i1 %0, label %ret, label %copy

copy:                                             ; preds = %entry
  %dst.0.0.04 = getelementptr %"struct.ap_fixed<32, 8>", %"struct.ap_fixed<32, 8>"* %dst, i64 0, i32 0, i32 0, i32 0
  %1 = load i32, i32* %src, align 512
  store i32 %1, i32* %dst.0.0.04, align 4
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>.14"(i32* noalias nocapture align 512 "unpacked"="0.0" %dst, %"struct.ap_fixed<32, 8>"* noalias readonly "unpacked"="1" %src) unnamed_addr #2 {
entry:
  %0 = icmp eq %"struct.ap_fixed<32, 8>"* %src, null
  br i1 %0, label %ret, label %copy

copy:                                             ; preds = %entry
  %src.0.0.03 = getelementptr %"struct.ap_fixed<32, 8>", %"struct.ap_fixed<32, 8>"* %src, i64 0, i32 0, i32 0, i32 0
  %1 = load i32, i32* %src.0.0.03, align 4
  store i32 %1, i32* %dst, align 512
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

declare i8* @malloc(i64)

declare void @free(i8*)

declare void @apatb_accelerator_net_hw(i32*, i16*, i32*, i32*, i32, i32, i32, i32, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, i1)

; Function Attrs: argmemonly noinline norecurse willreturn
define internal fastcc void @copy_back(i32* noalias "unpacked"="0", i32* noalias readonly align 512 "unpacked"="1", i16* noalias "unpacked"="2", i16* noalias readonly align 512 "unpacked"="3", %"struct.ap_fixed<32, 8>"* noalias "unpacked"="4", i32* noalias nocapture readonly align 512 "unpacked"="5.0", %"struct.ap_fixed<32, 8>"* noalias "unpacked"="6", i32* noalias nocapture readonly align 512 "unpacked"="7.0") unnamed_addr #3 {
entry:
  call fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>"(%"struct.ap_fixed<32, 8>"* %4, i32* align 512 %5)
  call fastcc void @"onebyonecpy_hls.p0struct.ap_fixed<32, 8>"(%"struct.ap_fixed<32, 8>"* %6, i32* align 512 %7)
  ret void
}

declare void @accelerator_net_hw_stub(i32* noalias nocapture nonnull readonly, i16* noalias nocapture nonnull readonly, %"struct.ap_fixed<32, 8>"* noalias nocapture nonnull, %"struct.ap_fixed<32, 8>"* noalias nocapture nonnull, i32, i32, i32, i32, %"struct.ap_fixed<32, 8>"* nocapture readonly, %"struct.ap_fixed<32, 8>"* nocapture readonly, %"struct.ap_fixed<32, 8>"* nocapture readonly, %"struct.ap_fixed<32, 8>"* nocapture readonly, %"struct.ap_fixed<32, 8>"* nocapture readonly, %"struct.ap_fixed<32, 8>"* nocapture readonly, i1 zeroext)

define void @accelerator_net_hw_stub_wrapper(i32*, i16*, i32*, i32*, i32, i32, i32, i32, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, %"struct.ap_fixed<32, 8>"*, i1) #4 {
entry:
  %15 = call i8* @malloc(i64 4)
  %16 = bitcast i8* %15 to %"struct.ap_fixed<32, 8>"*
  %17 = call i8* @malloc(i64 4)
  %18 = bitcast i8* %17 to %"struct.ap_fixed<32, 8>"*
  call void @copy_out(i32* null, i32* %0, i16* null, i16* %1, %"struct.ap_fixed<32, 8>"* %16, i32* %2, %"struct.ap_fixed<32, 8>"* %18, i32* %3)
  call void @accelerator_net_hw_stub(i32* %0, i16* %1, %"struct.ap_fixed<32, 8>"* %16, %"struct.ap_fixed<32, 8>"* %18, i32 %4, i32 %5, i32 %6, i32 %7, %"struct.ap_fixed<32, 8>"* %8, %"struct.ap_fixed<32, 8>"* %9, %"struct.ap_fixed<32, 8>"* %10, %"struct.ap_fixed<32, 8>"* %11, %"struct.ap_fixed<32, 8>"* %12, %"struct.ap_fixed<32, 8>"* %13, i1 %14)
  call void @copy_in(i32* null, i32* %0, i16* null, i16* %1, %"struct.ap_fixed<32, 8>"* %16, i32* %2, %"struct.ap_fixed<32, 8>"* %18, i32* %3)
  call void @free(i8* %15)
  call void @free(i8* %17)
  ret void
}

attributes #0 = { noinline willreturn "fpga.wrapper.func"="wrapper" }
attributes #1 = { argmemonly noinline norecurse willreturn "fpga.wrapper.func"="copyin" }
attributes #2 = { argmemonly noinline norecurse willreturn "fpga.wrapper.func"="onebyonecpy_hls" }
attributes #3 = { argmemonly noinline norecurse willreturn "fpga.wrapper.func"="copyout" }
attributes #4 = { "fpga.wrapper.func"="stub" }

!llvm.dbg.cu = !{}
!llvm.ident = !{!0, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1}
!llvm.module.flags = !{!2, !3, !4}
!blackbox_cfg = !{!5}

!0 = !{!"AMD/Xilinx clang version 16.0.6"}
!1 = !{!"clang version 7.0.0 "}
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{}
