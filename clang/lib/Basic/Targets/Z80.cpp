//===--- Z80.cpp - Implement Z80 target feature support -----------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Z80 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Z80.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"

using namespace clang;
using namespace clang::targets;

static constexpr int NumBuiltins =
    clang::Z80::LastTSBuiltin - Builtin::FirstTSBuiltin;

#define GET_BUILTIN_STR_TABLE
#include "clang/Basic/BuiltinsZ80.inc"
#undef GET_BUILTIN_STR_TABLE

static constexpr Builtin::Info BuiltinInfos[] = {
#define GET_BUILTIN_INFOS
#include "clang/Basic/BuiltinsZ80.inc"
#undef GET_BUILTIN_INFOS
};
static_assert(std::size(BuiltinInfos) == NumBuiltins);

SmallVector<Builtin::InfosShard> Z80TargetInfo::getTargetBuiltins() const {
  return {{&BuiltinStrings, BuiltinInfos}};
}

Z80TargetInfo::Z80TargetInfo(const llvm::Triple &Triple, const TargetOptions &)
    : TargetInfo(Triple) {
  // Must match Z80TargetMachine data layout
  resetDataLayout("e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16");

  PointerWidth = 16;
  PointerAlign = 8;
  ShortAlign = 8;
  IntWidth = 16;
  IntAlign = 8;
  LongWidth = 32;
  LongAlign = 8;
  LongLongWidth = 64;
  LongLongAlign = 8;
  Int128Align = 8;
  FloatAlign = 8;
  DoubleAlign = 8;
  LongDoubleAlign = 8;
  SuitableAlign = 8;
  DefaultAlignForAttributeAligned = 8;
  SizeType = UnsignedInt;
  PtrDiffType = SignedInt;
  IntPtrType = SignedInt;
  WCharType = UnsignedInt;
  WIntType = SignedInt;
  Char16Type = UnsignedInt;
  Char32Type = UnsignedLong;
  Int16Type = SignedInt;
  SigAtomicType = UnsignedChar;
}

bool Z80TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  // Braced register constraints: {bc}, {de}, {hl}, {af}, {ix}, {iy}, {sp}.
  // The caller iterates Name character by character; we must consume the
  // entire {regname} sequence including the closing brace.
  if (*Name == '{') {
    const char *End = strchr(Name, '}');
    if (End) {
      StringRef RegName(Name + 1, End - Name - 1);
      if (RegName == "bc" || RegName == "de" || RegName == "hl" ||
          RegName == "af" || RegName == "ix" || RegName == "iy" ||
          RegName == "sp" ||
          RegName == "a" || RegName == "b" || RegName == "c" ||
          RegName == "d" || RegName == "e" || RegName == "h" ||
          RegName == "l") {
        Name = End; // advance past closing brace (caller advances past first)
        Info.setAllowsRegister();
        return true;
      }
    }
  }
  // Multi-character register pair constraints (bc, de, hl, af, ix, iy, sp).
  // Name points to the first char; advance past the second on success.
  if (Name[0] && Name[1]) {
    StringRef R(Name, 2);
    if (R == "bc" || R == "de" || R == "hl" || R == "af" ||
        R == "ix" || R == "iy" || R == "sp") {
      ++Name; // advance past second char (caller advances past first)
      Info.setAllowsRegister();
      return true;
    }
  }
  switch (*Name) {
  default:
    return false;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'h':
  case 'l': // Individual 8-bit registers
  case 'r': // Any register (8/16-bit)
  case 'R': // 8-bit register class
    Info.setAllowsRegister();
    return true;
  }
}

// Rewrite a bare two-letter register-pair name (hl, bc, de, af, ix, iy, sp)
// into the braced specific-register form ({hl}, ...) for the emitted IR.
// validateAsmConstraint accepts bare "hl", but LLVM's IR-level InlineAsm
// parser splits a multi-letter constraint into single-register *alternatives*
// ("hl" -> h|l), which then can't hold a 16-bit operand and fatally aborts
// IRTranslator.  Emitting the braced form keeps it as one specific-register
// token (the path that already works).  Braced constraints are passed through
// verbatim so their inner letters are not re-interpreted as pair names.
std::string Z80TargetInfo::convertConstraint(const char *&Constraint) const {
  if (*Constraint == '{') {
    std::string Result = "{";
    while (*Constraint != '}' && Constraint[1]) {
      ++Constraint;
      Result += *Constraint;
    }
    // Constraint now points at '}'; the caller's loop advances past it.
    return Result;
  }
  if (Constraint[0] && Constraint[1]) {
    StringRef R(Constraint, 2);
    if (R == "bc" || R == "de" || R == "hl" || R == "af" || R == "ix" ||
        R == "iy" || R == "sp") {
      ++Constraint; // consume the second char (caller advances past the first)
      return std::string("{") + R.str() + "}";
    }
  }
  return std::string(1, *Constraint);
}

static const char *const Z80GCCRegNames[] = {
    "a",  "b",  "c",  "d",  "e",  "h",  "l",  "f",
    "bc", "de", "hl", "af", "ix", "iy", "sp",
};

static const char *const SM83GCCRegNames[] = {
    "a", "b", "c", "d", "e", "h", "l", "f", "bc", "de", "hl", "af", "sp",
};

llvm::ArrayRef<const char *> Z80TargetInfo::getGCCRegNames() const {
  if (getTriple().isSM83())
    return SM83GCCRegNames;
  return Z80GCCRegNames;
}

Z80TargetInfo::CallingConvCheckResult
Z80TargetInfo::checkCallingConvention(CallingConv CC) const {
  switch (CC) {
  case CC_C:
  case CC_Z80SDCCCall0:
  case CC_Z80AllReg:
  case CC_Z80FastCall:
  case CC_Z80Callee:
    return CCCR_OK;
  default:
    return CCCR_Warning;
  }
}

void Z80TargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  if (getTriple().isSM83()) {
    Builder.defineMacro("__sm83__");
    Builder.defineMacro("__SM83__");
    Builder.defineMacro("__GAMEBOY__");
  } else {
    Builder.defineMacro("__z80__");
    Builder.defineMacro("__Z80__");
  }
  // Z80/SM83 uses sdasz80 .rel object format, not ELF.
  // Do not define __ELF__.
}

bool Z80TargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  bool Ok = TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
  // The base Z80 ISA is implied for the z80 triple but not for sm83 (Game Boy),
  // which has no interrupt modes and no I register.  This gates the z80-only
  // builtins __builtin_z80_im2 / __builtin_z80_set_i (Features = "z80" in
  // BuiltinsZ80.td) so Sema rejects them on sm83 with a clean diagnostic
  // instead of a backend cannot-select.  ravn/llvm-z80#208.
  if (!getTriple().isSM83())
    Features["z80"] = true;
  return Ok;
}

bool Z80TargetInfo::hasFeature(StringRef Feature) const {
  return Feature == "z80" ? !getTriple().isSM83() : false;
}
