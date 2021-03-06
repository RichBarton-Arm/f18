! Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
!
! Licensed under the Apache License, Version 2.0 (the "License");
! you may not use this file except in compliance with the License.
! You may obtain a copy of the License at
!
!     http://www.apache.org/licenses/LICENSE-2.0
!
! Unless required by applicable law or agreed to in writing, software
! distributed under the License is distributed on an "AS IS" BASIS,
! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
! See the License for the specific language governing permissions and
! limitations under the License.

! Tests -fget-symbols-sources with COMMON.

program main
  integer :: x
  integer :: y
  block
    integer :: x
    x = y
  end block
  x = y
end program

! RUN: ${F18} -fget-symbols-sources -fparse-only -fdebug-semantics %s 2>&1 | ${FileCheck} %s
! CHECK:x:.*getsymbols05.f90, 18, 14-15
! CHECK:y:.*getsymbols05.f90, 19, 14-15
! CHECK:x:.*getsymbols05.f90, 21, 16-17
