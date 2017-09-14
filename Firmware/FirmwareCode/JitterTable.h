/*
   Copyright (c) 2015, Caleb Kemere
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   --/COPYRIGHT--

 *******************************************************************************

                         Rodent Stimulation Module (RSM) Firmware

  This file contains a table of random numbers from 0-3999. These are used as
  random jitter added to the stimulation period. We also will divide these
  values by 2 or 4 to generate random numbers from 0-1999 and 0-999.

*/

#include "SwitchMatrix.h"

unsigned int jitterTableCounter = 0;
int i;
unsigned int jitterValueTable[100] = {3178, 803, 2778, 2062, 2171, 2872, 1535, 247, 3853, 3139, 1086, 1825, 3780, 3801, 2690, 3384, 2586, 2653, 1398, 884, 604, 2943, 979, 647, 352, 3589, 108, 2915, 3784, 182, 3051, 901, 2536, 373, 1727, 1317, 3889, 1604, 2175, 3474, 2530, 1691, 3537, 1014, 186, 2448, 2107, 1433, 3722, 3827, 3484, 2489, 1265, 1918, 1903, 546, 3571, 3889, 3951, 193, 1570, 863, 3070, 1819, 1982, 1375, 2860, 2471, 2799, 3330, 672, 435, 2942, 3920, 360, 2452, 1768, 3771, 2984, 1894, 314, 55, 450, 1874, 3603, 3056, 2952, 3427, 52, 3932, 2183, 1240, 2429, 921, 322, 3105, 745, 623, 1762, 2146};;
const unsigned int jitterTableLength = sizeof(jitterValueTable) / sizeof(jitterValueTable[0]);
