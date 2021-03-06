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

// Parser templates and constexpr parsers shared by multiple
// per-type parser implementation source files.

#ifndef FORTRAN_PARSER_MISC_PARSERS_H_
#define FORTRAN_PARSER_MISC_PARSERS_H_

#include "basic-parsers.h"
#include "message.h"
#include "parse-tree.h"
#include "token-parsers.h"
#include "type-parsers.h"

namespace Fortran::parser {

// R401 xzy-list -> xzy [, xzy]...
template<typename PA> inline constexpr auto nonemptyList(const PA &p) {
  return nonemptySeparated(p, ","_tok);  // p-list
}

template<typename PA>
inline constexpr auto nonemptyList(MessageFixedText error, const PA &p) {
  return withMessage(error, nonemptySeparated(p, ","_tok));  // p-list
}

template<typename PA> inline constexpr auto optionalList(const PA &p) {
  return defaulted(nonemptySeparated(p, ","_tok));  // [p-list]
}

// R402 xzy-name -> name

// R516 keyword -> name
constexpr auto keyword{construct<Keyword>(name)};

// R1101 block -> [execution-part-construct]...
constexpr auto block{many(executionPartConstruct)};

constexpr auto listOfNames{nonemptyList("expected names"_err_en_US, name)};

constexpr auto star{construct<Star>("*"_tok)};
constexpr auto allocatable{construct<Allocatable>("ALLOCATABLE"_tok)};
constexpr auto contiguous{construct<Contiguous>("CONTIGUOUS"_tok)};
constexpr auto optional{construct<Optional>("OPTIONAL"_tok)};
constexpr auto pointer{construct<Pointer>("POINTER"_tok)};
constexpr auto protectedAttr{construct<Protected>("PROTECTED"_tok)};
constexpr auto save{construct<Save>("SAVE"_tok)};

}
#endif
