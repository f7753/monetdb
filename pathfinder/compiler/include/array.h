/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 *
 * Typed, dynamic arrays: accessing an element at an out-of-bounds index
 * does not yield an error but lets the array grow as needed.
 *
 * Basic interface:
 *  - PFarray (s)       create new array with elements of s bytes length each,
 *                      set array ``append index'' to 0
 *  - PFarray_at (a, i) address of ith element in array a (a grows as needed)
 *  - PFarray_last (a)  ``append index'': where to append to this array (r/w)
 *
 * More complex operations may be constructed using 
 * the basic interface, e.g.,
 *  - append one element e to array a (available as #PFarray_add):
 *    *(PFarray_at (a, PFarray_last (a)++) = e
 *    .
 *  - append n elements e1, ..., en, to array a (available as #PFarray_nadd):
 *    i = PFarray_last (a);
 *    (void) PFarray_at (a, i);
 *    (void) PFarray_at (a, i + (n - 1));
 *    PFarray_last(a) += n;
 *    *(PFarray_at (a, i)) = e1; ...; *(PFarray_at (a, i + (n - 1))) = en;
 *
 *
 * Copyright Notice:
 * -----------------
 *
 *  The contents of this file are subject to the MonetDB Public
 *  License Version 1.0 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is the ``Pathfinder'' system. The Initial
 *  Developer of the Original Code is the Database & Information
 *  Systems Group at the University of Konstanz, Germany. Portions
 *  created by U Konstanz are Copyright (C) 2000-2004 University
 *  of Konstanz. All Rights Reserved.
 *
 *  Contributors:
 *          Torsten Grust <torsten.grust@uni-konstanz.de>
 *          Maurice van Keulen <M.van.Keulen@bigfoot.com>
 *          Jens Teubner <jens.teubner@uni-konstanz.de>
 *
 * $Id$
 */

#ifndef ARRAY_H
#define ARRAY_H

/* size_t */
#include <stddef.h>

/* va_list */
#include <stdarg.h>

typedef struct PFarray_t PFarray_t;

struct PFarray_t {
  void        *base;   /**< array base address */
  unsigned int bound;  /**< current index bound (index 0...bound - 1 OK) */
  unsigned int appi;   /**< current array append index */
  size_t       esize;  /**< array element size */
};

PFarray_t *PFarray (size_t);

void *PFarray_at (PFarray_t *, unsigned int);

/**
 * Append index of array @c a (may be written to).
 *
 * @param a array
 * @return  append index accessed of array @c a
 */
#define PFarray_last(a) ((a)->appi)

/** 
 * Test: is array @c a empty?
 *
 * @param a array
 * @retur n true, if no elements inserted into array, false otherwise
 */
#define PFarray_empty(a) ((a)->appi == 0)

/**
 * ``Append address'' for array @c a (address of element position just
 * after element already written).  Repeated application of this macro
 * advances the append index.
 *
 * @param a  array
 * @return   array element address (or 0, in case of errors)
 */
#define PFarray_add(a) (PFarray_at ((a), PFarray_last (a)++))

/**
 * Address of front element for array @c a
 */
#define PFarray_top(a) (PFarray_at ((a), PFarray_last (a) - 1))

void *PFarray_nadd (PFarray_t *, unsigned int);

int PFarray_printf (PFarray_t *, const char *, ...)
    __attribute__ ((format (printf, 2, 3)));

int PFarray_vprintf (PFarray_t *, const char *, va_list);

/**
 * Array concatenation.  The entries of a2 are inserted 
 * into a1 (at end).  
 *
 * @attention NB. this makes sense only if both arrays have entries of
 *   the same size.
 */
PFarray_t *PFarray_concat (PFarray_t *, PFarray_t *);

/**
 * Array deletion.  Delete the array's front element.
 */
void PFarray_del (PFarray_t *);

#endif

/* vim:set shiftwidth=4 expandtab: */
