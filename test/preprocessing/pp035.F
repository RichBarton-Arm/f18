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

* #if 2 .LT. 3 works
      integer, parameter :: KWM = 666
#if 2 .LT. 3
#define KWM 777
#else
#define KWM 667
#endif
      if (KWM .eq. 777) then
        print *, 'pp035.F pass'
      else
        print *, 'pp035.F FAIL: ', KWM
      end if
      end
