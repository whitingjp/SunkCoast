/* 

$Id

Copyright (C) 2009 Alexios Chouchoulas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/


#ifndef __ASTAR_HEAP_H
#define __ASTAR_HEAP_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <sys/time.h>
#include "astar_config.h"


#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#  ifndef uint32_t
#    error "stdint.h is not available, and the various intXX_t and uintXX_t types are undefined."
#  endif // uint32_t
#endif // HAVE_STDINT_H


/*
 * This defines one square in the grid, and is the binary heap's payload. It
 * maintains f, g, and h values for a grid square.
 *
 * We also maintain a bitfield that stores compactly the necessary flags to
 * represent this square.
 *
 * The base cost to move onto this square stored as a 0-255 value. A value of
 * 255 means the square is completely inaccessible and will never be
 * considered. Values below that are added to the cost of moving onto the
 * square so that more 'expensive' moves can be avoided.
 *
 * The direction stored is towards the square's parent in the solution --
 * i.e. from the END of the path towards the BEGINNING. Must be reversed when a
 * route is found. The path proper is obtained at the end of the processing and
 * is stored piecemeal in square_t.rdir.
 *
 */

typedef struct {
	uint32_t    f;
	uint32_t    g;
	uint32_t    h;

#if defined(HEAP_DEBUG) || defined(ASTAR_DEBUG)
#define SQUARE_HAS_OFS
	// We use this for debugging.
	uint32_t    ofs;
#endif

	// This bitfield uses 18 of 32 bits.

	uint32_t    cost:8;     // We assign a base cost 0-255. 255=impassable.
	uint32_t    open:1;	// Is this in the open set?
	uint32_t    closed:1;	// Is this in the closed list?
	uint32_t    dir:3;      // Direction to this square's parent.
	uint32_t    rdir:3;     // Source->Destination direction.
	uint32_t    route:1;    // This is part of the final route.
	uint32_t    init:1;     // This square has been initialised.

} square_t;


typedef struct {
	uint32_t  *  data;	// Data.
	square_t  ** squares;   // Payload (array of square_t pointers)
	uint32_t     length;	// Entries in use.
	uint32_t     alloc;	// Entries allocated.
	uint32_t     delta;     // Size increase.
} asheap_t;


asheap_t * astar_heap_new (uint32_t initial_length, uint32_t delta);


void astar_heap_destroy (asheap_t * heap);


inline void astar_heap_clear (asheap_t * heap);


uint32_t astar_heap_sizeof (asheap_t * heap);


void astar_heap_add (asheap_t * heap, uint32_t val, square_t * payload);


uint32_t astar_heap_pop (asheap_t * heap, square_t ** payload);


uint32_t astar_heap_update (asheap_t * heap, square_t * payload);


int astar_heap_is_empty (asheap_t * heap);


void astar_heap_print (asheap_t * heap);


void astar_heap_fprint (asheap_t * heap, FILE * fp);


#ifdef __cplusplus
};
#endif // __cplusplus


#endif // _ASTAR_HEAP_H

// End of file.
