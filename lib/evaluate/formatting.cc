// Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "formatting.h"
#include "call.h"
#include "constant.h"
#include "expression.h"
#include "fold.h"
#include "tools.h"
#include "../parser/characters.h"
#include "../semantics/symbol.h"

namespace Fortran::evaluate {

bool formatForPGF90{false};

static void ShapeAsFortran(std::ostream &o, const ConstantSubscripts &shape) {
  if (GetRank(shape) > 1) {
    o << ",shape=";
    char ch{'['};
    for (auto dim : shape) {
      o << ch << dim;
      ch = ',';
    }
    o << "])";
  }
}

template<typename RESULT, typename VALUE>
std::ostream &ConstantBase<RESULT, VALUE>::AsFortran(std::ostream &o) const {
  if (Rank() > 1) {
    o << "reshape(";
  }
  if (Rank() > 0) {
    o << '[' << GetType().AsFortran() << "::";
  }
  bool first{true};
  for (const auto &value : values_) {
    if (first) {
      first = false;
    } else {
      o << ',';
    }
    if constexpr (Result::category == TypeCategory::Integer) {
      o << value.SignedDecimal() << '_' << Result::kind;
    } else if constexpr (Result::category == TypeCategory::Real ||
        Result::category == TypeCategory::Complex) {
      value.AsFortran(o, Result::kind);
    } else if constexpr (Result::category == TypeCategory::Character) {
      o << Result::kind << '_' << parser::QuoteCharacterLiteral(value, true);
    } else if constexpr (Result::category == TypeCategory::Logical) {
      if (value.IsTrue()) {
        o << ".true.";
      } else {
        o << ".false.";
      }
      o << '_' << Result::kind;
    } else {
      StructureConstructor{result_.derivedTypeSpec(), value}.AsFortran(o);
    }
  }
  if (Rank() > 0) {
    o << ']';
  }
  ShapeAsFortran(o, shape());
  return o;
}

template<int KIND>
std::ostream &Constant<Type<TypeCategory::Character, KIND>>::AsFortran(
    std::ostream &o) const {
  if (Rank() > 1) {
    o << "reshape(";
  }
  if (Rank() > 0) {
    o << '[' << GetType().AsFortran(std::to_string(length_)) << "::";
  }
  auto total{static_cast<ConstantSubscript>(size())};
  for (ConstantSubscript j{0}; j < total; ++j) {
    Scalar<Result> value{values_.substr(j * length_, length_)};
    if (j > 0) {
      o << ',';
    }
    if (Result::kind != 1 || !formatForPGF90) {
      o << Result::kind << '_';
    }
    o << parser::QuoteCharacterLiteral(value);
  }
  if (Rank() > 0) {
    o << ']';
  }
  ShapeAsFortran(o, shape());
  return o;
}

std::ostream &ActualArgument::AssumedType::AsFortran(std::ostream &o) const {
  return o << symbol_->name().ToString();
}

std::ostream &ActualArgument::AsFortran(std::ostream &o) const {
  if (keyword_) {
    o << keyword_->ToString() << '=';
  }
  if (isAlternateReturn_) {
    o << '*';
  }
  if (const auto *expr{UnwrapExpr()}) {
    return expr->AsFortran(o);
  } else {
    return std::get<AssumedType>(u_).AsFortran(o);
  }
}

std::ostream &SpecificIntrinsic::AsFortran(std::ostream &o) const {
  return o << name;
}

std::ostream &ProcedureRef::AsFortran(std::ostream &o) const {
  for (const auto &arg : arguments_) {
    if (arg && arg->isPassedObject()) {
      arg->AsFortran(o) << '%';
      break;
    }
  }
  proc_.AsFortran(o);
  char separator{'('};
  for (const auto &arg : arguments_) {
    if (arg && !arg->isPassedObject()) {
      arg->AsFortran(o << separator);
      separator = ',';
    }
  }
  if (separator == '(') {
    o << '(';
  }
  return o << ')';
}

// Operator precedence formatting; insert parentheses around operands
// only when necessary.

enum class Precedence {  // in increasing order for sane comparisons
  DefinedBinary,
  Or,
  And,
  Equivalence,  // .EQV., .NEQV.
  Not,  // which binds *less* tightly in Fortran than relations
  Relational,
  Additive,  // +, -, and (arbitrarily) //
  Negate,  // which binds *less* tightly than *, /, **
  Multiplicative,  // *, /
  Power,  // **, which is right-associative unlike the other dyadic operators
  DefinedUnary,
  Parenthesize,  // (x), (real, imaginary)
  Literal,
  Top,
};

template<typename A> constexpr Precedence ToPrecedence(const A &) {
  return Precedence::Top;
}
template<int KIND>
static Precedence ToPrecedence(const LogicalOperation<KIND> &x) {
  switch (x.logicalOperator) {
    SWITCH_COVERS_ALL_CASES
  case LogicalOperator::And: return Precedence::And;
  case LogicalOperator::Or: return Precedence::Or;
  case LogicalOperator::Not: return Precedence::Not;
  case LogicalOperator::Eqv:
  case LogicalOperator::Neqv: return Precedence::Equivalence;
  }
}
template<int KIND> constexpr Precedence ToPrecedence(const Not<KIND> &) {
  return Precedence::Not;
}
template<typename T> constexpr Precedence ToPrecedence(const Relational<T> &) {
  return Precedence::Relational;
}
template<typename T> constexpr Precedence ToPrecedence(const Add<T> &) {
  return Precedence::Additive;
}
template<typename T> constexpr Precedence ToPrecedence(const Subtract<T> &) {
  return Precedence::Additive;
}
template<int KIND> constexpr Precedence ToPrecedence(const Concat<KIND> &) {
  return Precedence::Additive;
}
template<typename T> constexpr Precedence ToPrecedence(const Negate<T> &) {
  return Precedence::Negate;
}
template<typename T> constexpr Precedence ToPrecedence(const Multiply<T> &) {
  return Precedence::Multiplicative;
}
template<typename T> constexpr Precedence ToPrecedence(const Divide<T> &) {
  return Precedence::Multiplicative;
}
template<typename T> constexpr Precedence ToPrecedence(const Power<T> &) {
  return Precedence::Power;
}
template<typename T>
constexpr Precedence ToPrecedence(const RealToIntPower<T> &) {
  return Precedence::Power;
}
template<typename T> static Precedence ToPrecedence(const Constant<T> &x) {
  static constexpr TypeCategory cat{T::category};
  if constexpr (cat == TypeCategory::Integer || cat == TypeCategory::Real) {
    if (auto n{GetScalarConstantValue<T>(x)}) {
      if (n->IsNegative()) {
        return Precedence::Negate;
      }
    }
  }
  return Precedence::Literal;
}
template<typename T> constexpr Precedence ToPrecedence(const Parentheses<T> &) {
  return Precedence::Parenthesize;
}

template<typename T> static Precedence ToPrecedence(const Expr<T> &expr) {
  return std::visit([](const auto &x) { return ToPrecedence(x); }, expr.u);
}

template<typename T> static bool IsNegatedScalarConstant(const Expr<T> &expr) {
  static constexpr TypeCategory cat{T::category};
  if constexpr (cat == TypeCategory::Integer || cat == TypeCategory::Real) {
    if (auto n{GetScalarConstantValue<T>(expr)}) {
      return n->IsNegative();
    }
  }
  return false;
}

template<TypeCategory CAT>
static bool IsNegatedScalarConstant(const Expr<SomeKind<CAT>> &expr) {
  return std::visit(
      [](const auto &x) { return IsNegatedScalarConstant(x); }, expr.u);
}

struct OperatorSpelling {
  const char *prefix{""}, *infix{","}, *suffix{""};
};

template<typename A> constexpr OperatorSpelling SpellOperator(const A &) {
  return OperatorSpelling{};
}
template<typename A>
constexpr OperatorSpelling SpellOperator(const Negate<A> &) {
  return OperatorSpelling{"-", "", ""};
}
template<int KIND>
static OperatorSpelling SpellOperator(const ComplexComponent<KIND> &x) {
  return OperatorSpelling{x.isImaginaryPart ? "AIMAG(" : "REAL(", "", ")"};
}
template<int KIND> constexpr OperatorSpelling SpellOperator(const Not<KIND> &) {
  return OperatorSpelling{".NOT.", "", ""};
}
template<int KIND>
constexpr OperatorSpelling SpellOperator(const SetLength<KIND> &) {
  return OperatorSpelling{"%SET_LENGTH(", ",", ")"};
}
template<int KIND>
constexpr OperatorSpelling SpellOperator(const ComplexConstructor<KIND> &) {
  return OperatorSpelling{"(", ",", ")"};
}
template<typename A> constexpr OperatorSpelling SpellOperator(const Add<A> &) {
  return OperatorSpelling{"", "+", ""};
}
template<typename A>
constexpr OperatorSpelling SpellOperator(const Subtract<A> &) {
  return OperatorSpelling{"", "-", ""};
}
template<typename A>
constexpr OperatorSpelling SpellOperator(const Multiply<A> &) {
  return OperatorSpelling{"", "*", ""};
}
template<typename A>
constexpr OperatorSpelling SpellOperator(const Divide<A> &) {
  return OperatorSpelling{"", "/", ""};
}
template<typename A>
constexpr OperatorSpelling SpellOperator(const Power<A> &) {
  return OperatorSpelling{"", "**", ""};
}
template<typename A>
constexpr OperatorSpelling SpellOperator(const RealToIntPower<A> &) {
  return OperatorSpelling{"", "**", ""};
}
template<typename A>
static OperatorSpelling SpellOperator(const Extremum<A> &x) {
  return OperatorSpelling{
      x.ordering == Ordering::Less ? "MIN(" : "MAX(", ",", ")"};
}
template<int KIND>
constexpr OperatorSpelling SpellOperator(const Concat<KIND> &) {
  return OperatorSpelling{"", "//", ""};
}
template<int KIND>
static OperatorSpelling SpellOperator(const LogicalOperation<KIND> &x) {
  return OperatorSpelling{"", AsFortran(x.logicalOperator), ""};
}
template<typename T>
static OperatorSpelling SpellOperator(const Relational<T> &x) {
  return OperatorSpelling{"", AsFortran(x.opr), ""};
}

template<typename D, typename R, typename... O>
std::ostream &Operation<D, R, O...>::AsFortran(std::ostream &o) const {
  Precedence lhsPrec{ToPrecedence(left())};
  OperatorSpelling spelling{SpellOperator(derived())};
  o << spelling.prefix;
  Precedence thisPrec{ToPrecedence(derived())};
  if constexpr (operands == 1) {
    if (lhsPrec <= thisPrec) {
      o << '(' << left() << ')';
    } else {
      o << left();
    }
  } else {
    if (lhsPrec == Precedence::Parenthesize) {
      o << left();
    } else if (lhsPrec < thisPrec ||
        (lhsPrec == Precedence::Power && thisPrec == Precedence::Power)) {
      o << '(' << left() << ')';
    } else {
      o << left();
    }
    o << spelling.infix;
    Precedence rhsPrec{ToPrecedence(right())};
    if (rhsPrec == Precedence::Parenthesize) {
      o << right();
    } else if (rhsPrec < thisPrec) {
      o << '(' << right() << ')';
    } else {
      o << right();
    }
  }
  return o << spelling.suffix;
}

template<typename TO, TypeCategory FROMCAT>
std::ostream &Convert<TO, FROMCAT>::AsFortran(std::ostream &o) const {
  static_assert(TO::category == TypeCategory::Integer ||
          TO::category == TypeCategory::Real ||
          TO::category == TypeCategory::Character ||
          TO::category == TypeCategory::Logical,
      "Convert<> to bad category!");
  if constexpr (TO::category == TypeCategory::Character) {
    this->left().AsFortran(o << "achar(iachar(") << ')';
  } else if constexpr (TO::category == TypeCategory::Integer) {
    this->left().AsFortran(o << "int(");
  } else if constexpr (TO::category == TypeCategory::Real) {
    this->left().AsFortran(o << "real(");
  } else {
    this->left().AsFortran(o << "logical(");
  }
  return o << ",kind=" << TO::kind << ')';
}

std::ostream &Relational<SomeType>::AsFortran(std::ostream &o) const {
  std::visit([&](const auto &rel) { rel.AsFortran(o); }, u);
  return o;
}

template<typename T>
std::ostream &EmitArray(std::ostream &o, const Expr<T> &expr) {
  return expr.AsFortran(o);
}

template<typename T>
std::ostream &EmitArray(std::ostream &, const ArrayConstructorValues<T> &);

template<typename T>
std::ostream &EmitArray(std::ostream &o, const ImpliedDo<T> &implDo) {
  o << '(';
  EmitArray(o, implDo.values());
  o << ',' << ImpliedDoIndex::Result::AsFortran()
    << "::" << implDo.name().ToString() << '=';
  implDo.lower().AsFortran(o) << ',';
  implDo.upper().AsFortran(o) << ',';
  implDo.stride().AsFortran(o) << ')';
  return o;
}

template<typename T>
std::ostream &EmitArray(
    std::ostream &o, const ArrayConstructorValues<T> &values) {
  const char *sep{""};
  for (const auto &value : values) {
    o << sep;
    std::visit([&](const auto &x) { EmitArray(o, x); }, value.u);
    sep = ",";
  }
  return o;
}

template<typename T>
std::ostream &ArrayConstructor<T>::AsFortran(std::ostream &o) const {
  o << '[' << GetType().AsFortran() << "::";
  EmitArray(o, *this);
  return o << ']';
}

template<int KIND>
std::ostream &ArrayConstructor<Type<TypeCategory::Character, KIND>>::AsFortran(
    std::ostream &o) const {
  std::stringstream len;
  LEN().AsFortran(len);
  o << '[' << GetType().AsFortran(len.str()) << "::";
  EmitArray(o, *this);
  return o << ']';
}

std::ostream &ArrayConstructor<SomeDerived>::AsFortran(std::ostream &o) const {
  o << '[' << GetType().AsFortran() << "::";
  EmitArray(o, *this);
  return o << ']';
}

template<typename RESULT>
std::ostream &ExpressionBase<RESULT>::AsFortran(std::ostream &o) const {
  std::visit(
      common::visitors{
          [&](const BOZLiteralConstant &x) {
            o << "z'" << x.Hexadecimal() << "'";
          },
          [&](const NullPointer &) { o << "NULL()"; },
          [&](const common::CopyableIndirection<Substring> &s) {
            s.value().AsFortran(o);
          },
          [&](const ImpliedDoIndex &i) { o << i.name.ToString(); },
          [&](const auto &x) { x.AsFortran(o); },
      },
      derived().u);
  return o;
}

std::ostream &StructureConstructor::AsFortran(std::ostream &o) const {
  o << DerivedTypeSpecAsFortran(result_.derivedTypeSpec());
  if (values_.empty()) {
    o << '(';
  } else {
    char ch{'('};
    for (const auto &[symbol, value] : values_) {
      value.value().AsFortran(o << ch << symbol->name().ToString() << '=');
      ch = ',';
    }
  }
  return o << ')';
}

std::string DynamicType::AsFortran() const {
  if (derived_) {
    CHECK(category_ == TypeCategory::Derived);
    return DerivedTypeSpecAsFortran(*derived_);
  } else if (charLength_) {
    std::string result{"CHARACTER(KIND="s + std::to_string(kind_) + ",LEN="};
    if (charLength_->isAssumed()) {
      result += '*';
    } else if (charLength_->isDeferred()) {
      result += ':';
    } else if (const auto &length{charLength_->GetExplicit()}) {
      std::stringstream ss;
      length->AsFortran(ss);
      result += ss.str();
    }
    return result + ')';
  } else if (IsUnlimitedPolymorphic()) {
    return "CLASS(*)";
  } else if (IsAssumedType()) {
    return "TYPE(*)";
  } else if (IsTypelessIntrinsicArgument()) {
    return "(typeless intrinsic function argument)";
  } else {
    return EnumToString(category_) + '(' + std::to_string(kind_) + ')';
  }
}

std::string DynamicType::AsFortran(std::string &&charLenExpr) const {
  if (!charLenExpr.empty() && category_ == TypeCategory::Character) {
    return "CHARACTER(KIND=" + std::to_string(kind_) +
        ",LEN=" + std::move(charLenExpr) + ')';
  } else {
    return AsFortran();
  }
}

std::string SomeDerived::AsFortran() const {
  if (IsUnlimitedPolymorphic()) {
    return "CLASS(*)";
  } else {
    return "TYPE("s + DerivedTypeSpecAsFortran(derivedTypeSpec()) + ')';
  }
}

std::string DerivedTypeSpecAsFortran(const semantics::DerivedTypeSpec &spec) {
  std::stringstream ss;
  ss << spec.name().ToString();
  char ch{'('};
  for (const auto &[name, value] : spec.parameters()) {
    ss << ch << name.ToString() << '=';
    ch = ',';
    if (value.isAssumed()) {
      ss << '*';
    } else if (value.isDeferred()) {
      ss << ':';
    } else {
      value.GetExplicit()->AsFortran(ss);
    }
  }
  if (ch != '(') {
    ss << ')';
  }
  return ss.str();
}

std::ostream &EmitVar(std::ostream &o, const Symbol &symbol) {
  return o << symbol.name().ToString();
}

std::ostream &EmitVar(std::ostream &o, const std::string &lit) {
  return o << parser::QuoteCharacterLiteral(lit);
}

std::ostream &EmitVar(std::ostream &o, const std::u16string &lit) {
  return o << parser::QuoteCharacterLiteral(lit);
}

std::ostream &EmitVar(std::ostream &o, const std::u32string &lit) {
  return o << parser::QuoteCharacterLiteral(lit);
}

template<typename A> std::ostream &EmitVar(std::ostream &o, const A &x) {
  return x.AsFortran(o);
}

template<typename A>
std::ostream &EmitVar(std::ostream &o, common::Reference<A> x) {
  return EmitVar(o, *x);
}

template<typename A>
std::ostream &EmitVar(std::ostream &o, const A *p, const char *kw = nullptr) {
  if (p) {
    if (kw) {
      o << kw;
    }
    EmitVar(o, *p);
  }
  return o;
}

template<typename A>
std::ostream &EmitVar(
    std::ostream &o, const std::optional<A> &x, const char *kw = nullptr) {
  if (x) {
    if (kw) {
      o << kw;
    }
    EmitVar(o, *x);
  }
  return o;
}

template<typename A, bool COPY>
std::ostream &EmitVar(std::ostream &o, const common::Indirection<A, COPY> &p,
    const char *kw = nullptr) {
  if (kw) {
    o << kw;
  }
  EmitVar(o, p.value());
  return o;
}

template<typename A>
std::ostream &EmitVar(std::ostream &o, const std::shared_ptr<A> &p) {
  CHECK(p);
  return EmitVar(o, *p);
}

template<typename... A>
std::ostream &EmitVar(std::ostream &o, const std::variant<A...> &u) {
  std::visit([&](const auto &x) { EmitVar(o, x); }, u);
  return o;
}

std::ostream &BaseObject::AsFortran(std::ostream &o) const {
  return EmitVar(o, u);
}

template<int KIND>
std::ostream &TypeParamInquiry<KIND>::AsFortran(std::ostream &o) const {
  if (base_) {
    return base_->AsFortran(o) << '%';
  }
  return EmitVar(o, parameter_);
}

std::ostream &Component::AsFortran(std::ostream &o) const {
  base_.value().AsFortran(o);
  return EmitVar(o << '%', symbol_);
}

std::ostream &NamedEntity::AsFortran(std::ostream &o) const {
  std::visit(
      common::visitors{
          [&](SymbolRef s) { EmitVar(o, s); },
          [&](const Component &c) { c.AsFortran(o); },
      },
      u_);
  return o;
}

std::ostream &Triplet::AsFortran(std::ostream &o) const {
  EmitVar(o, lower_) << ':';
  EmitVar(o, upper_);
  EmitVar(o << ':', stride_.value());
  return o;
}

std::ostream &Subscript::AsFortran(std::ostream &o) const {
  return EmitVar(o, u);
}

std::ostream &ArrayRef::AsFortran(std::ostream &o) const {
  base_.AsFortran(o);
  char separator{'('};
  for (const Subscript &ss : subscript_) {
    ss.AsFortran(o << separator);
    separator = ',';
  }
  return o << ')';
}

std::ostream &CoarrayRef::AsFortran(std::ostream &o) const {
  bool first{true};
  for (const Symbol &part : base_) {
    if (first) {
      first = false;
    } else {
      o << '%';
    }
    EmitVar(o, part);
  }
  char separator{'('};
  for (const auto &sscript : subscript_) {
    EmitVar(o << separator, sscript);
    separator = ',';
  }
  if (separator == ',') {
    o << ')';
  }
  separator = '[';
  for (const auto &css : cosubscript_) {
    EmitVar(o << separator, css);
    separator = ',';
  }
  if (stat_) {
    EmitVar(o << separator, stat_, "STAT=");
    separator = ',';
  }
  if (team_) {
    EmitVar(
        o << separator, team_, teamIsTeamNumber_ ? "TEAM_NUMBER=" : "TEAM=");
  }
  return o << ']';
}

std::ostream &DataRef::AsFortran(std::ostream &o) const {
  return EmitVar(o, u);
}

std::ostream &Substring::AsFortran(std::ostream &o) const {
  EmitVar(o, parent_) << '(';
  EmitVar(o, lower_) << ':';
  return EmitVar(o, upper_) << ')';
}

std::ostream &ComplexPart::AsFortran(std::ostream &o) const {
  return complex_.AsFortran(o) << '%' << EnumToString(part_);
}

std::ostream &ProcedureDesignator::AsFortran(std::ostream &o) const {
  return EmitVar(o, u);
}

template<typename T>
std::ostream &Designator<T>::AsFortran(std::ostream &o) const {
  std::visit(
      common::visitors{
          [&](SymbolRef symbol) { EmitVar(o, symbol); },
          [&](const auto &x) { x.AsFortran(o); },
      },
      u);
  return o;
}

std::ostream &DescriptorInquiry::AsFortran(std::ostream &o) const {
  switch (field_) {
  case Field::LowerBound: o << "lbound("; break;
  case Field::Extent: o << "size("; break;
  case Field::Stride: o << "%STRIDE("; break;
  case Field::Rank: o << "rank("; break;
  case Field::Len: break;
  }
  base_.AsFortran(o);
  if (field_ == Field::Len) {
    return o << "%len";
  } else {
    if (dimension_ >= 0) {
      o << ",dim=" << (dimension_ + 1);
    }
    return o << ')';
  }
}

INSTANTIATE_CONSTANT_TEMPLATES
INSTANTIATE_EXPRESSION_TEMPLATES
INSTANTIATE_VARIABLE_TEMPLATES
}
