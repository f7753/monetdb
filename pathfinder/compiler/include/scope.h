/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 *
 * Functions and data structures to maintain nested scopes (e.g., to
 * implement variable visibility).
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
 *  created by U Konstanz are Copyright (C) 2000-2003 University
 *  of Konstanz. All Rights Reserved.
 *
 *  Contributors:
 *          Torsten Grust <torsten.grust@uni-konstanz.de>
 *          Maurice van Keulen <M.van.Keulen@bigfoot.com>
 *          Jens Teubner <jens.teubner@uni-konstanz.de>
 *
 * $Id$
 */

#ifndef SCOPE_H
#define SCOPE_H

/* PFarray_t */
#include "array.h"

/* PFqname_t */
#include "qname.h"

/* # of buckets in entry hash table */
#define SCOPE_HASH_SZ 31

typedef struct PFscope_t PFscope_t;

struct PFscope_t {
    PFarray_t *hash[SCOPE_HASH_SZ];      /**< entry hash table */
    PFarray_t *undo;                     /**< undo stack */
};

/** create a scope data structure */
PFscope_t *PFscope ();

/** bring new entry key |--> value into scope */
void PFscope_into (PFscope_t *, PFqname_t, void *);

/** overall number of entries in scope data structure */ 
unsigned int PFscope_count (PFscope_t *);

/** open a new scope */
void PFscope_open (PFscope_t *);

/** close current scope */
void PFscope_close (PFscope_t *);

/** is key currently in scope? (returns 0 if key is not in scope) */
void *PFscope_lookup (PFscope_t *, PFqname_t);

#endif /* SCOPE_H */

/* vim:set shiftwidth=4 expandtab: */
