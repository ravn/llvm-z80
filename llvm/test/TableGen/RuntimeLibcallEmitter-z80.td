// RUN: llvm-tblgen -gen-runtime-libcalls -I %p/../../include %s | FileCheck %s

include "llvm/IR/RuntimeLibcalls.td"

// The profiles are disjoint; adding z88dk must not change bare Z80/SM83.
// CHECK-LABEL: if ((TT.getArch() == Triple::z80 || TT.getArch() == Triple::sm83) && !(TT.getEnvironment() == Triple::Z88DK)) {
// CHECK: RTLIB::impl___addsf3,
// CHECK: RTLIB::impl___addsf3_fast,
// CHECK-NOT: CallingConv::Z80_SDCCCall0
// CHECK-LABEL: if ((TT.getArch() == Triple::z80 || TT.getArch() == Triple::sm83) && TT.getEnvironment() == Triple::Z88DK) {
// CHECK-NOT: RTLIB::impl___addsf3,
// CHECK: static const RTLIB::LibcallImpl LibraryCalls_AlwaysAvailable_Z80_SDCCCall0[] = {
// CHECK-NEXT: RTLIB::impl_cm32_sdcc_fsadd,
// CHECK: RTLIB::impl_cm32_sdcc_fsadd_fast,
// CHECK: RTLIB::impl_cm32_sdcc___fs2sint,
// CHECK: RTLIB::impl___divmodsi4,
// CHECK: RTLIB::impl___divsi3,
// CHECK: RTLIB::impl_cm32_sdcc___slong2fs,
// CHECK: setLibcallImplCallingConv(Impl, CallingConv::Z80_SDCCCall0);
