/*
 * Copyright (C) 2004-2007  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 1997-2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: //depot/sw/releases/Aquila_9.2.0_U10.5/apps/dhcp/dhcp-4.1.0a2/common/heap.c#1 $ */

/*
 * This heap implementation is taken from the BIND 9 code. It has been
 * modified to use the DHCP memory management rather than BIND memory
 * contexts.
 */

/*! \file
 * Heap implementation of priority queues adapted from the following:
 *
 *	\li "Introduction to Algorithms," Cormen, Leiserson, and Rivest,
 *	MIT Press / McGraw Hill, 1990, ISBN 0-262-03141-8, chapter 7.
 *
 *	\li "Algorithms," Second Edition, Sedgewick, Addison-Wesley, 1988,
 *	ISBN 0-201-06673-4, chapter 11.
 */

#include "dhcpd.h"
#include "omapip/omapip.h"
#include "heap.h"

#include <assert.h>
#define REQUIRE assert
#define INSIST assert

/*@{*/
/*%
 * Note: to make heap_parent and heap_left easy to compute, the first
 * element of the heap array is not used; i.e. heap subscripts are 1-based,
 * not 0-based.  The parent is index/2, and the left-child is index*2.
 * The right child is index*2+1.
 */
#define heap_parent(i)			((i) >> 1)
#define heap_left(i)			((i) << 1)
/*@}*/

#define SIZE_INCREMENT			1024

/*%
 * When the heap is in a consistent state, the following invariant
 * holds true: for every element i > 1, heap_parent(i) has a priority
 * higher than or equal to that of i.
 */
#define HEAPCONDITION(i) ((i) == 1 || \
			  ! heap->compare(heap->array[(i)], \
					  heap->array[heap_parent(i)]))

/*% ISC heap structure. */
struct isc_heap {
	unsigned int			size;
	unsigned int			size_increment;
	unsigned int			last;
	void				**array;
	isc_heapcompare_t		compare;
	isc_heapindex_t			index;
};

isc_result_t
isc_heap_create(isc_heapcompare_t compare,
		isc_heapindex_t index, unsigned int size_increment,
		isc_heap_t **heapp)
{
	isc_heap_t *heap;

	REQUIRE(heapp != NULL && *heapp == NULL);
	REQUIRE(compare != NULL);

	heap = dmalloc(sizeof(*heap), MDL);
	if (heap == NULL)
		return (ISC_R_NOMEMORY);
	heap->size = 0;
	if (size_increment == 0)
		heap->size_increment = SIZE_INCREMENT;
	else
		heap->size_increment = size_increment;
	heap->last = 0;
	heap->array = NULL;
	heap->compare = compare;
	heap->index = index;

	*heapp = heap;

	return (ISC_R_SUCCESS);
}

void
isc_heap_destroy(isc_heap_t **heapp) {
	isc_heap_t *heap;

	REQUIRE(heapp != NULL);
	heap = *heapp;

	if (heap->array != NULL)
		dfree(heap->array, MDL);
	dfree(heap, MDL);

	*heapp = NULL;
}

static isc_boolean_t
resize(isc_heap_t *heap) {
	void **new_array;
	size_t new_size;

	new_size = heap->size + heap->size_increment;
	new_array = dmalloc(new_size * sizeof(void *), MDL);
	if (new_array == NULL)
		return (ISC_FALSE);
	if (heap->array != NULL) {
		memcpy(new_array, heap->array, heap->size * sizeof(void *));
		dfree(heap->array, MDL);
	}
	heap->size = new_size;
	heap->array = new_array;

	return (ISC_TRUE);
}

static void
float_up(isc_heap_t *heap, unsigned int i, void *elt) {
	unsigned int p;

	for (p = heap_parent(i) ;
	     i > 1 && heap->compare(elt, heap->array[p]) ;
	     i = p, p = heap_parent(i)) {
		heap->array[i] = heap->array[p];
		if (heap->index != NULL)
			(heap->index)(heap->array[i], i);
	}
	heap->array[i] = elt;
	if (heap->index != NULL)
		(heap->index)(heap->array[i], i);

	INSIST(HEAPCONDITION(i));
}

static void
sink_down(isc_heap_t *heap, unsigned int i, void *elt) {
	unsigned int j, size, half_size;
	size = heap->last;
	half_size = size / 2;
	while (i <= half_size) {
		/* Find the smallest of the (at most) two children. */
		j = heap_left(i);
		if (j < size && heap->compare(heap->array[j+1],
					      heap->array[j]))
			j++;
		if (heap->compare(elt, heap->array[j]))
			break;
		heap->array[i] = heap->array[j];
		if (heap->index != NULL)
			(heap->index)(heap->array[i], i);
		i = j;
	}
	heap->array[i] = elt;
	if (heap->index != NULL)
		(heap->index)(heap->array[i], i);

	INSIST(HEAPCONDITION(i));
}

isc_result_t
isc_heap_insert(isc_heap_t *heap, void *elt) {
	unsigned int i;

	i = ++heap->last;
	if (heap->last >= heap->size && !resize(heap))
		return (ISC_R_NOMEMORY);

	float_up(heap, i, elt);

	return (ISC_R_SUCCESS);
}

void
isc_heap_delete(isc_heap_t *heap, unsigned int index) {
	void *elt;
	isc_boolean_t less;

	REQUIRE(index >= 1 && index <= heap->last);

	if (index == heap->last) {
		heap->last--;
	} else {
		elt = heap->array[heap->last--];
		less = heap->compare(elt, heap->array[index]);
		heap->array[index] = elt;
		if (less)
			float_up(heap, index, heap->array[index]);
		else
			sink_down(heap, index, heap->array[index]);
	}
}

void
isc_heap_increased(isc_heap_t *heap, unsigned int index) {
	REQUIRE(index >= 1 && index <= heap->last);

	float_up(heap, index, heap->array[index]);
}

void
isc_heap_decreased(isc_heap_t *heap, unsigned int index) {
	REQUIRE(index >= 1 && index <= heap->last);

	sink_down(heap, index, heap->array[index]);
}

void *
isc_heap_element(isc_heap_t *heap, unsigned int index) {
	REQUIRE(index >= 1 && index <= heap->last);

	return (heap->array[index]);
}

void
isc_heap_foreach(isc_heap_t *heap, isc_heapaction_t action, void *uap) {
	unsigned int i;

	REQUIRE(action != NULL);

	for (i = 1 ; i <= heap->last ; i++)
		(action)(heap->array[i], uap);
}
