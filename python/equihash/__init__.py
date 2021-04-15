"""Wrapper for libequihash library

   Copyright (c) 2021, Marsiske Stefan.
   All rights reserved.

   This file is part of libequihash

   libequihash is free software: you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   libequihash is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libequihash If not, see <http://www.gnu.org/licenses/>.
"""

import ctypes
import ctypes.util
from ctypes import sizeof, c_uint16, c_uint8, c_size_t, c_uint32

equihashlib = ctypes.cdll.LoadLibrary(ctypes.util.find_library('equihash') or ctypes.util.find_library('libequihash'))

if not equihashlib._name:
    raise ValueError('Unable to find libequihash')

def __check(code):
    if code == 0:
        raise ValueError

def solsize(n,k):
    return equihashlib.solsize(n,k)

def solve(n, k, seed):
    sol_len = equihashlib.solsize(n,k)
    sol = ctypes.create_string_buffer(sol_len)
    __check(equihashlib.solve(n,k,seed,len(seed),sol,sol_len))
    return sol.raw

def verify(n,k,seed,sol):
    return equihashlib.verify(n,k,seed,len(seed),sol,len(sol)) == 1
