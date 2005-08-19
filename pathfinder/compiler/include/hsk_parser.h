/**
 * @file
 *
 * Parse output of the Haskell XQuery-to-Algebra Mapper into
 * Pathfinder algebra.
 *
 *
 * Copyright Notice:
 * -----------------
 *
 * The contents of this file are subject to the Pathfinder Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/PathfinderLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Pathfinder system.
 *
 * The Initial Developer of the Original Code is the Database &
 * Information Systems Group at the University of Konstanz, Germany.
 * Portions created by the University of Konstanz are Copyright (C)
 * 2000-2005 University of Konstanz.  All Rights Reserved.
 *
 * $Id$
 */

#ifndef HSK_PARSER_H
#define HSK_PARSER_H

#include "algebra.h"

/* function prototype from parse.c */
PFla_op_t *PFhsk_parse (void);

#endif  /* HSK_PARSER_H */

/* vim:set shiftwidth=4 expandtab: */
