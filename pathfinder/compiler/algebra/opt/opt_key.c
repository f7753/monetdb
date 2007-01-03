/**
 * @file
 *
 * Optimize relational algebra expression DAG
 * based on the key property.
 * (This requires no burg pattern matching as we 
 *  apply optimizations in a peep-hole style on 
 *  single nodes only.)
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
 * The Original Code has initially been developed by the Database &
 * Information Systems Group at the University of Konstanz, Germany and
 * is now maintained by the Database Systems Group at the Technische
 * Universitaet Muenchen, Germany.  Portions created by the University of
 * Konstanz and the Technische Universitaet Muenchen are Copyright (C)
 * 2000-2005 University of Konstanz and (C) 2005-2007 Technische
 * Universitaet Muenchen, respectively.  All Rights Reserved.
 *
 * $Id$
 */

/* always include pathfinder.h first! */
#include "pathfinder.h"
#include <assert.h>
#include <stdio.h>

#include "algopt.h"
#include "properties.h"
#include "alg_dag.h"
#include "mem.h"          /* PFmalloc() */

/*
 * Easily access subtree-parts.
 */
/** starting from p, make a step left */
#define L(p) ((p)->child[0])
/** starting from p, make a step right */
#define R(p) ((p)->child[1])

#define SEEN(p) ((p)->bit_dag)

/* worker for PFalgopt_key */
static void
opt_key (PFla_op_t *p)
{
    assert (p);

    /* rewrite each node only once */
    if (SEEN(p))
        return;
    else
        SEEN(p) = true;

    /* apply key-related optimization for children */
    for (unsigned int i = 0; i < PFLA_OP_MAXCHILD && p->child[i]; i++)
        opt_key (p->child[i]);

    /* action code */
    switch (p->kind) {
        case la_distinct:
            for (unsigned int i = 0; i < p->schema.count; i++)
                if (PFprop_key_left (p->prop, p->schema.items[i].name)) {
                    *p = *PFla_dummy (L(p));
                    break;
                }
            break;
            
        case la_avg:
	case la_max:
	case la_min:
        case la_sum:
        case la_seqty1:
        case la_all:
            /* if part is key we already have our aggregate */
            if (p->sem.aggr.part &&
                PFprop_key_left (p->prop, p->sem.aggr.part)) {
                *p = *PFla_project (
                          L(p), 
                          PFalg_proj (p->sem.aggr.res,
                                      p->sem.aggr.att),
                          PFalg_proj (p->sem.aggr.part,
                                      p->sem.aggr.part));
                SEEN(p) = true;                   
            }
            break;

        case la_count:
            /* if part is key we already have our aggregate */
            if (p->sem.aggr.part &&
                PFprop_key_left (p->prop, p->sem.aggr.part)) {
                *p = *PFla_attach (
                          PFla_project (
                              L(p),
                              PFalg_proj (p->sem.aggr.part,
                                          p->sem.aggr.part)),
                          p->sem.aggr.res,
                          PFalg_lit_int (1));
                SEEN(p) = true;                   
            }
            break;

        case la_rownum:
            if (PFprop_key_left (p->prop, p->sem.rownum.part)) {
                /* replace rownum by attach if part is key */
                *p = *PFla_attach (
                          L(p),
                          p->sem.rownum.attname,
                          PFalg_lit_nat (1));
                SEEN(p) = true;                   
            } else {
                /* discard all sort criterions after a key attribute */
                PFord_ordering_t sortby = PFordering ();

                for (unsigned int i = 0;
                     i < PFord_count (p->sem.rownum.sortby);
                     i++) {
                    sortby = PFord_refine (
                                 sortby,
                                 PFord_order_col_at (
                                     p->sem.rownum.sortby,
                                     i),
                                 PFord_order_dir_at (
                                     p->sem.rownum.sortby,
                                     i));
                    if (PFprop_key_left (
                            p->prop,
                            PFord_order_col_at (
                                p->sem.rownum.sortby,
                                i)))
                        break;
                }

                p->sem.rownum.sortby = sortby;
            }
            break;
        
        case la_number:
            if (PFprop_key_left (p->prop, p->sem.number.part)) {
                /* replace number by attach if part is key */
                *p = *PFla_attach (
                          L(p),
                          p->sem.number.attname,
                          PFalg_lit_nat (1));
                SEEN(p) = true;                   
            } else if (p->sem.number.part)
                break;

/* FIXME: We discard the following optimization as it
   replaces number operators by rownum operators. These
   are more difficult to cope with in the proxy generation
   phase. */
#if 0
            /* if there already exists a key column with the
               same type we can replace the number operator
               by a projection. */
            for (unsigned int i = 0; i < p->schema.count; i++)
                if (PFprop_key_left (p->prop, p->schema.items[i].name) &&
                    p->sem.number.attname != p->schema.items[i].name &&
                    p->schema.items[i].type == aat_nat) {
                    PFalg_proj_t *proj = PFmalloc (
                                             p->schema.count
                                             * sizeof (PFalg_proj_t));
                    /* copy property list */
                    for (unsigned int j = 0; j < p->schema.count; j++)
                        if (p->sem.number.attname == p->schema.items[j].name)
                            proj[j] = PFalg_proj (p->sem.number.attname,
                                                  p->schema.items[i].name);
                        else
                            proj[j] = PFalg_proj (p->schema.items[j].name,
                                                  p->schema.items[j].name);
                                                  
                    *p = *PFla_project_ (L(p), p->schema.count, proj);
                    SEEN(p) = true;
                    break;
                }
#endif
            break;

        default:
            break;
    }
}

/**
 * Invoke algebra optimization.
 */
PFla_op_t *
PFalgopt_key (PFla_op_t *root)
{
    /* Infer key properties first */
    PFprop_infer_key (root);

    /* Optimize algebra tree */
    opt_key (root);
    PFla_dag_reset (root);

    return root;
}

/* vim:set shiftwidth=4 expandtab filetype=c: */
