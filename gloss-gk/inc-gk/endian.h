/* Copyright (C) 1992, 1996, 1997, 2000, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef	_ENDIAN_H
#define	_ENDIAN_H	1

#include <features.h>
#include <stdint.h>

/* Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

#ifdef   _LITTLE_ENDIAN
#define  __LITTLE_ENDIAN   _LITTLE_ENDIAN
#else
#define	__LITTLE_ENDIAN	1234
#endif

#ifdef   _BIG_ENDIAN
#define  __BIG_ENDIAN      _BIG_ENDIAN
#else
#define	__BIG_ENDIAN	4321
#endif

#define	__PDP_ENDIAN	3412

/* This file defines `__BYTE_ORDER' for the particular machine.  */
#include <bits/endian.h>

/* Some machines may need to use a different endianness for floating point
   values.  */
#ifndef __FLOAT_WORD_ORDER
# define __FLOAT_WORD_ORDER __BYTE_ORDER
#endif

#ifndef LITTLE_ENDIAN
# define LITTLE_ENDIAN	__LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
# define BIG_ENDIAN	__BIG_ENDIAN
#endif
# define PDP_ENDIAN	__PDP_ENDIAN
# define BYTE_ORDER	__BYTE_ORDER

#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define __LONG_LONG_PAIR(HI, LO) LO, HI
#elif __BYTE_ORDER == __BIG_ENDIAN
# define __LONG_LONG_PAIR(HI, LO) HI, LO
#endif

static uint16_t htobe16(uint16_t x) { return __builtin_bswap16(x); }
static uint16_t htole16(uint16_t x) { return x; }
static uint16_t be16toh(uint16_t x) { return __builtin_bswap16(x); }
static uint16_t le16toh(uint16_t x) { return x; }

static uint32_t htobe32(uint32_t x) { return __builtin_bswap32(x); }
static uint32_t htole32(uint32_t x) { return x; }
static uint32_t be32toh(uint32_t x) { return __builtin_bswap32(x); }
static uint32_t le32toh(uint32_t x) { return x; }

static uint64_t htobe64(uint64_t x) { return __builtin_bswap64(x); }
static uint64_t htole64(uint64_t x) { return x; }
static uint64_t be64toh(uint64_t x) { return __builtin_bswap64(x); }
static uint64_t le64toh(uint64_t x) { return x; }



#endif	/* endian.h */
