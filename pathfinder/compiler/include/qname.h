/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 *
 * Declarations for QName handling functions; functions are
 * implemented in pathfinder/qname.c.
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

#ifndef QNAME_H
#define QNAME_H

/* PFns_t */
#include "ns.h"

/** XML namespace qualified name */
typedef struct PFqname_t PFqname_t;

/** This represents a QName `ns:loc'
 *  (a QName `loc' with no namespace has ns.ns = 0 and ns.uri = 0,
 *   see semantics/ns.h).
 */
struct PFqname_t {
  PFns_t ns;       /**< namespace part */
  char   *loc;     /**< local part */
};

/* is QName a wildcard (ns:*)? */
#define PFQNAME_WILDCARD(qn) ((qn).loc == 0)

/* Compare two QNames */
int 
PFqname_eq (PFqname_t, PFqname_t);

/* Create string representation of a QName */
char *
PFqname_str (PFqname_t);

/* Create string representation of a QName (use URI instead of NS prefix) */
char *
PFqname_uri_str (PFqname_t);

/** Return the URI part of a QName */
char *
PFqname_uri (PFqname_t qn);

/** Return the local name of a QName */
char *
PFqname_loc (PFqname_t qn);

/* Extract QName from a string */
PFqname_t PFstr_qname (char *);

PFqname_t PFqname (PFns_t, char *);

#endif   /* QNAME_H */

/* vim:set shiftwidth=4 expandtab: */
