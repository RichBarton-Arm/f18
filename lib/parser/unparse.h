// Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef FORTRAN_PARSER_UNPARSE_H_
#define FORTRAN_PARSER_UNPARSE_H_

#include "char-block.h"
#include "characters.h"
#include <functional>
#include <iosfwd>

namespace Fortran::evaluate {
struct GenericExprWrapper;
struct GenericAssignmentWrapper;
class ProcedureRef;
}

namespace Fortran::parser {

struct Program;

// A function called before each Statement is unparsed.
using preStatementType =
    std::function<void(const CharBlock &, std::ostream &, int)>;

// Functions to handle unparsing of analyzed expressions and related
// objects rather than their original parse trees.
struct AnalyzedObjectsAsFortran {
  std::function<void(std::ostream &, const evaluate::GenericExprWrapper &)>
      expr;
  std::function<void(
      std::ostream &, const evaluate::GenericAssignmentWrapper &)>
      assignment;
  std::function<void(std::ostream &, const evaluate::ProcedureRef &)> call;
};

// Converts parsed program to out as Fortran.
void Unparse(std::ostream &out, const Program &program,
    Encoding encoding = Encoding::UTF_8, bool capitalizeKeywords = true,
    bool backslashEscapes = true, preStatementType *preStatement = nullptr,
    AnalyzedObjectsAsFortran * = nullptr);
}

#endif
