! Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
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

! See Fortran 2018, clause 16.10.2
! TODO: These are placeholder values so that some tests can be run.

include '../runtime/magic-numbers.h' ! for IOSTAT= error/end code values

module iso_fortran_env

  implicit none

  integer, parameter :: atomic_int_kind = selected_int_kind(18)
  integer, parameter :: atomic_logical_kind = atomic_int_kind

  ! TODO: Use PACK([x],test) in place of the array constructor idiom
  ! [(x, integer::j=1,COUNT([test]))] below once PACK() can be folded.

  integer, parameter, private :: &
    selectedASCII = selected_char_kind('ASCII'), &
    selectedUCS_2 = selected_char_kind('UCS-2'), &
    selectedUnicode = selected_char_kind('ISO_10646')
  integer, parameter :: character_kinds(*) = [ &
    [(selectedASCII, integer :: j=1, count([selectedASCII >= 0]))], &
    [(selectedUCS_2, integer :: j=1, count([selectedUCS_2 >= 0]))], &
    [(selectedUnicode, integer :: j=1, count([selectedUnicode >= 0]))]]

  integer, parameter, private :: &
    selectedInt8 = selected_int_kind(2), &
    selectedInt16 = selected_int_kind(4), &
    selectedInt32 = selected_int_kind(9), &
    selectedInt64 = selected_int_kind(18),&
    selectedInt128 = selected_int_kind(38), &
    safeInt8 = merge(selectedInt8, selected_int_kind(0), &
                     selectedInt8 >= 0), &
    safeInt16 = merge(selectedInt16, selected_int_kind(0), &
                      selectedInt16 >= 0), &
    safeInt32 = merge(selectedInt32, selected_int_kind(0), &
                      selectedInt32 >= 0), &
    safeInt64 = merge(selectedInt64, selected_int_kind(0), &
                      selectedInt64 >= 0), &
    safeInt128 = merge(selectedInt128, selected_int_kind(0), &
                       selectedInt128 >= 0)
  integer, parameter :: &
    int8 = merge(selectedInt8, merge(-2, -1, selectedInt8 >= 0), &
                 digits(int(0,kind=safeInt8)) == 7), &
    int16 = merge(selectedInt16, merge(-2, -1, selectedInt16 >= 0), &
                  digits(int(0,kind=safeInt16)) == 15), &
    int32 = merge(selectedInt32, merge(-2, -1, selectedInt32 >= 0), &
                  digits(int(0,kind=safeInt32)) == 31), &
    int64 = merge(selectedInt64, merge(-2, -1, selectedInt64 >= 0), &
                  digits(int(0,kind=safeInt64)) == 63), &
    int128 = merge(selectedInt128, merge(-2, -1, selectedInt128 >= 0), &
                   digits(int(0,kind=safeInt128)) == 127)

  integer, parameter :: integer_kinds(*) = [ &
    selected_int_kind(0), &
    ((selected_int_kind(k), &
      integer :: j=1, count([selected_int_kind(k) >= 0 .and. &
                             selected_int_kind(k) /= &
                               selected_int_kind(k-1)])), &
     integer :: k=1, 39)]

  integer, parameter :: &
    logical8 = int8, logical16 = int16, logical32 = int32, logical64 = int64
  integer, parameter :: logical_kinds(*) = [ &
    [(logical8, integer :: j=1, count([logical8 >= 0]))], &
    [(logical16, integer :: j=1, count([logical16 >= 0]))], &
    [(logical32, integer :: j=1, count([logical32 >= 0]))], &
    [(logical64, integer :: j=1, count([logical64 >= 0]))]]

  integer, parameter, private :: &
    selectedReal16 = selected_real_kind(3, 4), &      ! IEEE half
    selectedBfloat16 = selected_real_kind(2, 37), &   ! truncated IEEE single
    selectedReal32 = selected_real_kind(6, 37), &     ! IEEE single
    selectedReal64 = selected_real_kind(15, 307), &   ! IEEE double
    selectedReal80 = selected_real_kind(18, 4931), &  ! 80x87 extended
    selectedReal64x2 = selected_real_kind(31, 307), & ! "double-double"
    selectedReal128 = selected_real_kind(33, 9863), & ! IEEE quad
    safeReal16 = merge(selectedReal16, selected_real_kind(0,0), &
                       selectedReal16 >= 0), &
    safeBfloat16 = merge(selectedBfloat16, selected_real_kind(0,0), &
                         selectedBfloat16 >= 0), &
    safeReal32 = merge(selectedReal32, selected_real_kind(0,0), &
                       selectedReal32 >= 0), &
    safeReal64 = merge(selectedReal64, selected_real_kind(0,0), &
                       selectedReal64 >= 0), &
    safeReal80 = merge(selectedReal80, selected_real_kind(0,0), &
                       selectedReal80 >= 0), &
    safeReal64x2 = merge(selectedReal64x2, selected_real_kind(0,0), &
                         selectedReal64x2 >= 0), &
    safeReal128 = merge(selectedReal128, selected_real_kind(0,0), &
                        selectedReal128 >= 0)
  integer, parameter :: &
    real16 = merge(selectedReal16, merge(-2, -1, selectedReal16 >= 0), &
                   digits(real(0,kind=safeReal16)) == 11), &
    bfloat16 = merge(selectedBfloat16, merge(-2, -1, selectedBfloat16 >= 0), &
                     digits(real(0,kind=safeBfloat16)) == 8), &
    real32 = merge(selectedReal32, merge(-2, -1, selectedReal32 >= 0), &
                   digits(real(0,kind=safeReal32)) == 24), &
    real64 = merge(selectedReal64, merge(-2, -1, selectedReal64 >= 0), &
                   digits(real(0,kind=safeReal64)) == 53), &
    real80 = merge(selectedReal80, merge(-2, -1, selectedReal80 >= 0), &
                   digits(real(0,kind=safeReal80)) == 64), &
    real64x2 = merge(selectedReal64x2, merge(-2, -1, selectedReal64x2 >= 0), &
                     digits(real(0,kind=safeReal64x2)) == 106), &
    real128 = merge(selectedReal128, merge(-2, -1, selectedReal128 >= 0), &
                    digits(real(0,kind=safeReal128)) == 112)

  integer, parameter :: real_kinds(*) = [ &
    [(real16, integer :: j=1, count([real16 >= 0]))], &
    [(bfloat16, integer :: j=1, count([bfloat16 >= 0]))], &
    [(real32, integer :: j=1, count([real32 >= 0]))], &
    [(real64, integer :: j=1, count([real64 >= 0]))], &
    [(real80, integer :: j=1, count([real80 >= 0]))], &
    [(real64x2, integer :: j=1, count([real64x2 >= 0]))], &
    [(real128, integer :: j=1, count([real128 >= 0]))]]

  integer, parameter :: current_team = -1, initial_team = -2, parent_team = -3

  integer, parameter :: input_unit = 5, output_unit = 6, error_unit = 0
  integer, parameter :: iostat_end = -1, iostat_eor = -2
  integer, parameter :: iostat_inquire_internal_unit = -1

  integer, parameter :: character_storage_size = 8
  integer, parameter :: file_storage_size = 8
  integer, parameter :: numeric_storage_size = 32

  integer, parameter :: stat_failed_image = FORTRAN_RUNTIME_STAT_FAILED_IMAGE
  integer, parameter :: stat_locked = FORTRAN_RUNTIME_STAT_LOCKED
  integer, parameter :: stat_locked_other_image = FORTRAN_RUNTIME_STAT_LOCKED_OTHER_IMAGE
  integer, parameter :: stat_stopped_image = FORTRAN_RUNTIME_STAT_STOPPED_IMAGE
  integer, parameter :: stat_unlocked = FORTRAN_RUNTIME_STAT_UNLOCKED
  integer, parameter :: stat_unlocked_failed_image = FORTRAN_RUNTIME_STAT_UNLOCKED_FAILED_IMAGE

  type :: event_type
    private
    integer(kind=atomic_int_kind) :: count = 0
  end type event_type

  type :: lock_type
    private
    integer(kind=atomic_int_kind) :: count = 0
  end type lock_type

  type :: team_type
    private
    integer(kind=int64) :: id = 0
  end type team_type

 contains

  character(len=80) function compiler_options()
    compiler_options = 'COMPILER_OPTIONS() not yet implemented'
  end function compiler_options

  character(len=80) function compiler_version()
    compiler_version = 'f18 in development'
  end function compiler_version
end module iso_fortran_env
