/**
 * @file
 *
 * Rewrite/optimize algebra expression tree.
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
 *          Jens Teubner <jens.teubner@uni-konstanz.de>
 *
 * $Id$
 */

#include <assert.h>

#include "pathfinder.h"
#include "algopt.h"

/** twig-generated node type identifiers */
#include "algopt.symbols.h"

/** twig: type of tree node */
#define TWIG_NODE PFalg_op_t

/** twig: max number of children under a parse tree node */
#define TWIG_MAXCHILD PFALG_OP_MAXCHILD

static int TWIG_ID[] = {
      [aop_lit_tbl]   lit_tbl    /**< literal table */
    , [aop_disjunion] disjunion  /**< union two relations with same schema */
    , [aop_cross]     cross      /**< cross product (Cartesian product) */
    , [aop_eqjoin]    eqjoin     /**< equi-join */
    , [aop_project]   project    /**< projection and renaming operator */
    , [aop_rownum]    rownum     /**< consecutive number generation */

    , [aop_serialize] serialize  /**< serialize algebra expression below
                                      (This is mainly used explicitly match
                                      the expression root during the Twig
                                      pass.) */
};

/** twig: setup twig */
#include "twig.h"

/* undefine the twig node ids because we introduce
 * MIL tree constructor functions of the same name below
 */
#undef lit_tbl
#undef disjunion
#undef cross
#undef eqjoin
#undef project
#undef rownum
#undef serialize

/* ----------------------- End of twig setup -------------------- */

#include "algebra_mnemonic.h"

/**
 * Rewrite/optimize algebra expression tree. Returns modified tree.
 */
PFalg_op_t *
PFalgopt (PFalg_op_t *root)
{
    assert (root);

    return rewrite (root, 0);
}
