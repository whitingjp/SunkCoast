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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "astar_heap.h"


///////////////////////////////////////////////////////////////////////////////
//
// DEBUGGING
//
///////////////////////////////////////////////////////////////////////////////

#if defined(ASTAR_DEBUG) || defined(HEAP_DEBUG)
FILE * __heap_debugfp;
#define __debug(format, ...) fprintf(__heap_debugfp, format, ##__VA_ARGS__)
#else
#define __debug(...)
#endif // ASTAR_DEBUG || HEAP_DEBUG


///////////////////////////////////////////////////////////////////////////////
//
// LOCATIONS OF HEAP NODES WITH RESPECT TO OTHER NODES
//
///////////////////////////////////////////////////////////////////////////////

#define PARENT_OF(x) (((x)-1)>>1)
#define CHILD1(x)    (((x)<<1)+1)
#define CHILD2(x)    (((x)<<1)+2)


#define check_null(p,err) if ((p) == NULL) { perror (err); exit (EXIT_FAILURE); }


asheap_t *
astar_heap_new (uint32_t initial_length, uint32_t delta)
{
	asheap_t * heap = (asheap_t *) malloc (sizeof (asheap_t));
	check_null (heap, "heap_new(), allocating memory");

	// Set initial values.
	heap->length = 0;
	heap->delta = delta;
	heap->alloc = initial_length;
	heap->data = (uint32_t *) malloc (sizeof (uint32_t) * heap->alloc);
	check_null (heap->data, "heap_new(), allocating data block");
	heap->squares = (square_t **) malloc (sizeof (square_t *) * heap->alloc);
	check_null (heap->squares, "heap_new(), allocating payload block");

	return heap;
}


void
astar_heap_destroy (asheap_t * heap)
{
	free (heap->data);
	free (heap->squares);
	free (heap);
}


inline void
astar_heap_clear (asheap_t * heap)
{
	assert (heap != NULL);
	heap->length = 0;
}


uint32_t
astar_heap_sizeof (asheap_t * heap)
{
	return sizeof (asheap_t) + sizeof (uint32_t) * 2 * (heap)->alloc;
}


#ifdef HEAP_DEBUG

void
astar_heap_highlight (asheap_t * heap, uint32_t highlight)
{
	uint32_t i;
	for (i=0; i<heap->length; i++) {
		uint32_t val = heap->data[i];
		int p = i > 0? PARENT_OF(i): 0;
		if (val == highlight) printf ("%2d -> \033[0;7m%3d\033[0m", i, val, p);
		else printf ("%2d -> %3d", i, val, p);
		if (i > 0) printf(" <- %3d", p);
#ifdef SQUARE_HAS_OFS
		printf(" ofs=%u \n", heap->squares[i]->ofs);
#endif
	}
	printf("\n");
}

#else

// Remove these calls from non-debugging code.

#define astar_heap_highlight(heap,highlight)

#endif // ASTAR_DEBUG || HEAP_DEBUG || TEST_HEAP


void
astar_heap_add (asheap_t * heap, uint32_t val, square_t * square)
{
	assert (heap != NULL);

	// Is is full?
	if (heap->length == heap->alloc) {
		heap->alloc += heap->delta;
		heap->data = (uint32_t *) realloc (heap->data, sizeof (uint32_t) * heap->alloc);
		check_null (heap->data, "heap_add(), growing data block");
		heap->squares = (square_t **) realloc (heap->squares,
						       sizeof (square_t *) * heap->alloc);
		check_null (heap->squares, "heap_add(), growing payload block");
	}

	// Is it empty? Trivial case.
	if (heap->length == 0) {
		heap->data[0] = val;
		heap->squares[0] = square;
		heap->length = 1;
		return;
	}

	// Stick the new value at the end.
	heap->data [heap->length] = val;
	heap->squares [heap->length] = square;

	// Bubble up.
	register uint32_t parent_val, parent_ofs;
	register uint32_t i = heap->length;
	void * parent_payload;
	while (i > 0) {

		// Get parent value.
		parent_ofs = PARENT_OF(i);
		parent_val = heap->data [parent_ofs];
		parent_payload = heap->squares [parent_ofs];
		
		// If the parent is greater then val, swap them.
		if (parent_val > val) {
			heap->data [i] = parent_val;
			heap->data [parent_ofs] = val;

			heap->squares [i] = parent_payload;
			heap->squares [parent_ofs] = square;

			// Loop again.
			i = parent_ofs;
		} else {
			// Not greater, end bubbling up.
			break;
		}
	}

	// Increase the number of elements.
	heap->length++;
}


int
astar_heap_is_empty (asheap_t * heap)
{
	assert (heap != NULL);
	return heap->length == 0;
}


uint32_t
astar_heap_pop (asheap_t * heap, square_t ** square)
{
	assert (heap != NULL);
	assert (heap->length > 0);

	// Trivial case; singleton element.
	if (heap->length == 1) {
		heap->length = 0;
		if (square != NULL) *square = heap->squares [0];
		return heap->data [0];
	}

	// Get the root value and payload.
	uint32_t retval = heap->data [0];
	if (square != NULL) *square = heap->squares [0];

	// Move the last value to the root.
	uint32_t val = heap->data [heap->length - 1];
	heap->data[0] = val;
	heap->squares[0] = heap->squares [heap->length - 1];

	// Adjust the length.
	heap->length--;
	
	// Bubble down.
	uint32_t child_val1, child_val2, child_ofs1, child_ofs2;
	uint32_t i = 0;

	for (;;) {

#ifdef HEAP_DEBUG
		astar_heap_highlight (heap, val);
#endif

		// Get the two child nodes.
		child_ofs1 = CHILD1 (i);
		child_ofs2 = CHILD2 (i);

		// Get the values of the child nodes. If a value doesn't exist,
		// we use MAXINT, which results in no swaps.
		// NB: For max-heaps, 0 must be used instead.
		child_val1 = child_ofs1 < heap->length? heap->data[child_ofs1]: 0xffffffff;
		child_val2 = child_ofs2 < heap->length? heap->data[child_ofs2]: 0xffffffff;
		
#ifdef HEAP_DEBUG
		__debug("\t\tComparing %d against %d and %d\n", val, child_val1, child_val2);
#endif

		// This node is less than or equal to both of its children (if they exist).
		if ((val <= child_val1) && (val <= child_val2)) break;

		// Locate the minimal child. Use child_ofs1 and child_val1.
		if (child_val2 < child_val1) {
			child_val1 = child_val2;
			child_ofs1 = child_ofs2;
		}
		
#ifdef HEAP_DEBUG
		__debug("\t\tSwap data[%d]=%d with data[%d]=%d\n", i, val, child_ofs1, child_val1);
#endif

		// Swap using data[i]=val and data[child_ofs1]=child_val1.
		heap->data [i] = child_val1;
		heap->data [child_ofs1] = val;

		void * root_payload = heap->squares [i];
		heap->squares [i] = heap->squares [child_ofs1];
		heap->squares [child_ofs1] = root_payload;

		// Loop again, bubbling down.
		i = child_ofs1;
		continue;
	}

	// Return the value that used to be the root.
	return retval;
}


static inline int32_t
astar_heap_getofs (asheap_t * heap, square_t * payload)
{
	uint32_t i;
	for (i = 0; i <= heap->length; i++) {
		if (heap->squares[i] == payload) return i;
	}
	return -1;
}


inline static uint32_t
astar_heap_bubble_up (asheap_t * heap, int32_t heapofs)
{
	// Bubble up. This keeps the heap consistent (all children
	// have greater values).

	register uint32_t parent_val, parent_ofs;
	register uint32_t val;
	register int32_t i = heapofs;

	// Let's do this thang.
	while (i > 0) {

#ifdef HEAP_DEBUG
		__debug("*** BUBBLING UP, i=%d\n", i);
		astar_heap_fprint (heap, __heap_debugfp);
#endif // HEAP_DEBUG

		// Get the current value of f.
		val = heap->data[i];

		// Get parent value.
		parent_ofs = PARENT_OF(i);
		parent_val = heap->data [parent_ofs];
		__debug ("*** THIS:   i=%d val=%d, ofs=%u\n", i, val, heap->squares[i]->ofs);
		__debug ("*** PARENT: i=%d val=%d, ofs=%u\n", parent_ofs, parent_val, heap->squares[parent_ofs]->ofs);
		
		// If the parent is greater then val, swap them.
		if (parent_val > val) {

			// Swap the values.
			heap->data [i] = parent_val;
			heap->data [parent_ofs] = val;

			// Swap the payload (grid offset).
			square_t * tmp = heap->squares [i];
			heap->squares [i] = heap->squares [parent_ofs];
			heap->squares [parent_ofs] = tmp;

			__debug ("*** SWAPPED\n\n");
			__debug ("*** THIS:   i=%d val=%d, ofs=%u\n", i, val, heap->squares[i]->ofs);
			__debug ("*** PARENT: i=%d val=%d, ofs=%u\n", parent_ofs, parent_val, heap->squares[parent_ofs]->ofs);

			// Loop again.
			i = parent_ofs;

		} else {
			// Not greater, stop here.
			break;
		}
	}

	// Return the final offset of the value in the heap.
	return i;
}


uint32_t
astar_heap_update (asheap_t * heap, square_t * payload)
{
	assert (heap != NULL);
	assert (heap->length > 0);

	// First, we need to find which element on the heap has the specified
	// payload (square). This is an expensive O(n) operation, but it's one
	// we only do rarely -- I hope.
	int32_t ofs = astar_heap_getofs (heap, payload);
	assert (ofs >= 0);

	// Sanity check -- we can only lower a value as we only bubble up.
	assert (heap->data[ofs] >= payload->f);

	// Update the value.
	__debug("Set heap->data[%d (ofs)] = %d (new_val)\n", ofs, payload->f);
	heap->data [ofs] = payload->f;

	// Bubble up to keep the heap consistent.
	uint32_t new_ofs = astar_heap_bubble_up (heap, ofs);

	return new_ofs;
}


void
astar_heap_fprint (asheap_t * heap, FILE * fp)
{
	uint32_t i;

	for (i = 0; i < heap->length; i++) {
#ifdef SQUARE_HAS_OFS
		fprintf (fp, "%d -> %d ofs=%u (ptr=%p) square->f = %u\n",
			 i, heap->data[i], heap->squares[i]->ofs, heap->squares[i],
			 heap->squares[i]->f);
		assert (heap->data[i] == heap->squares[i]->f);
#else
		fprintf (fp, "%d -> %d ptr=%p\n", (int)i, (int)heap->data[i], heap->squares[i]);
#endif // SQUARE_HAS_OFS
	}
}


void
astar_heap_print (asheap_t * heap)
{
	astar_heap_fprint (heap, stdout);
}


///////////////////////////////////////////////////////////////////////////////
//
// TEST/DEBUGGING CODE
//
///////////////////////////////////////////////////////////////////////////////


#ifdef TEST_HEAP

#ifndef NUM_INS
#define NUM_INS 10000
#endif // NUM_INS

int
main (int argc, char ** argv)
{
#ifdef HEAP_DEBUG
	__heap_debugfp = stderr;
#endif // HEAP_DEBUG

	asheap_t * h = astar_heap_new (10, 100);
	uint32_t i;
	srand(0);

	printf ("Size of void * on this system: %d bytes\n", sizeof (void *));

	for (i = 0; i < NUM_INS; i++) {
		uint32_t x = rand() % 1000;

		// We use 32-bit integers as payload, rather than
		// pointers. On 64-bit boxes, pointers are 64 bits wide. Fix
		// this.

#if __WORDSIZE == 64
		uint64_t xx = x;
		printf ("Adding #%d (%d) -> %p (%llu)...\n", i, x, (void*)xx, (void*)xx);
#else
		uint32_t xx = x;
		printf ("Adding #%d (%d) -> %p (%u)...\n", i, x, (void*)xx, (void*)xx);
#endif // __WORDSIZE == 64

		astar_heap_add (h, x, (void*)xx);
	}
	assert (h->length == NUM_INS);

	uint32_t prev = 0;
	while (!astar_heap_is_empty (h)) {
		square_t * payload;
		uint32_t next = astar_heap_pop (h, &payload);
		prev = next;
		assert (next >= prev);
#if __WORDSIZE == 64
		assert ((uint64_t) payload == (uint64_t) next);
#else
		assert ((uint32_t) payload == (uint32_t) next);
#endif // __WORDSIZE
	}

	printf ("Popping has been verified to be monotonic.\n");
	printf ("Key to payload mapping has been verified to be consistent.\n");
	astar_heap_destroy (h);
}

#endif // TEST_HEAP


// End of file.
