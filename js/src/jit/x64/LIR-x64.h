/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_x64_LIR_x64_h
#define jit_x64_LIR_x64_h

namespace js {
namespace jit {

// Given an untyped input, guards on whether it's a specific type and returns
// the unboxed payload.
class LUnbox : public LInstructionHelper<1, BOX_PIECES, 0> {
 public:
  LIR_HEADER(Unbox)

  explicit LUnbox(const LAllocation& input) : LInstructionHelper(classOpcode) {
    setOperand(0, input);
  }

  static const size_t Input = 0;

  LBoxAllocation input() const { return getBoxOperand(Input); }

  MUnbox* mir() const { return mir_->toUnbox(); }
  const char* extraName() const { return StringFromMIRType(mir()->type()); }
};

class LDivOrModI64 : public LBinaryMath<1> {
 public:
  LIR_HEADER(DivOrModI64)

  LDivOrModI64(const LAllocation& lhs, const LAllocation& rhs,
               const LDefinition& temp)
      : LBinaryMath(classOpcode) {
    setOperand(0, lhs);
    setOperand(1, rhs);
    setTemp(0, temp);
  }

  const LDefinition* remainder() { return getTemp(0); }

  MBinaryArithInstruction* mir() const {
    MOZ_ASSERT(mir_->isDiv() || mir_->isMod());
    return static_cast<MBinaryArithInstruction*>(mir_);
  }
  bool canBeDivideByZero() const {
    if (mir_->isMod()) {
      return mir_->toMod()->canBeDivideByZero();
    }
    return mir_->toDiv()->canBeDivideByZero();
  }
  bool canBeNegativeOverflow() const {
    if (mir_->isMod()) {
      return mir_->toMod()->canBeNegativeDividend();
    }
    return mir_->toDiv()->canBeNegativeOverflow();
  }
  const wasm::TrapSiteDesc& trapSiteDesc() const {
    MOZ_ASSERT(mir_->isDiv() || mir_->isMod());
    if (mir_->isMod()) {
      return mir_->toMod()->trapSiteDesc();
    }
    return mir_->toDiv()->trapSiteDesc();
  }
};

// This class performs a simple x86 'div', yielding either a quotient or
// remainder depending on whether this instruction is defined to output
// rax (quotient) or rdx (remainder).
class LUDivOrModI64 : public LBinaryMath<1> {
 public:
  LIR_HEADER(UDivOrModI64);

  LUDivOrModI64(const LAllocation& lhs, const LAllocation& rhs,
                const LDefinition& temp)
      : LBinaryMath(classOpcode) {
    setOperand(0, lhs);
    setOperand(1, rhs);
    setTemp(0, temp);
  }

  const LDefinition* remainder() { return getTemp(0); }

  const char* extraName() const {
    return mir()->isTruncated() ? "Truncated" : nullptr;
  }

  MBinaryArithInstruction* mir() const {
    MOZ_ASSERT(mir_->isDiv() || mir_->isMod());
    return static_cast<MBinaryArithInstruction*>(mir_);
  }

  bool canBeDivideByZero() const {
    if (mir_->isMod()) {
      return mir_->toMod()->canBeDivideByZero();
    }
    return mir_->toDiv()->canBeDivideByZero();
  }

  const wasm::TrapSiteDesc& trapSiteDesc() const {
    MOZ_ASSERT(mir_->isDiv() || mir_->isMod());
    if (mir_->isMod()) {
      return mir_->toMod()->trapSiteDesc();
    }
    return mir_->toDiv()->trapSiteDesc();
  }
};

}  // namespace jit
}  // namespace js

#endif /* jit_x64_LIR_x64_h */
