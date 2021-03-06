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

// Constraint checking for procedure references

#ifndef FORTRAN_SEMANTICS_CHECK_CALL_H_
#define FORTRAN_SEMANTICS_CHECK_CALL_H_

#include "../evaluate/call.h"

namespace Fortran::parser {
class Messages;
class ContextualMessages;
}
namespace Fortran::evaluate::characteristics {
struct Procedure;
}
namespace Fortran::evaluate {
class FoldingContext;
}

namespace Fortran::semantics {
class Scope;

// The Boolean flag argument should be true when the called procedure
// does not actually have an explicit interface at the call site, but
// its characteristics are known because it is a subroutine or function
// defined at the top level in the same source file.
void CheckArguments(const evaluate::characteristics::Procedure &,
    evaluate::ActualArguments &, evaluate::FoldingContext &, const Scope &,
    bool treatingExternalAsImplicit = false);

// Checks actual arguments against a procedure with an explicit interface.
// Reports a buffer of errors when not compatible.
parser::Messages CheckExplicitInterface(
    const evaluate::characteristics::Procedure &, evaluate::ActualArguments &,
    const evaluate::FoldingContext &, const Scope &);

// Checks actual arguments for the purpose of resolving a generic interface.
bool CheckInterfaceForGeneric(const evaluate::characteristics::Procedure &,
    evaluate::ActualArguments &, const evaluate::FoldingContext &);
}
#endif
