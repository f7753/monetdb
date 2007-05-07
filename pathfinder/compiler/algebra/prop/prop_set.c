/**
 * @file
 *
 * Inference of set property of logical algebra expressions. This property
 * marks all operators that may return their result without duplicates.
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

#include "properties.h"

#include "alg_dag.h"
#include "oops.h"
#include "mem.h"
#include <stdio.h>

/*
 * Easily access subtree-parts.
 */
/** starting from p, make a step left */
#define L(p) ((p)->child[0])
/** starting from p, make a step right */
#define R(p) ((p)->child[1])
/** starting from p, make two steps left */
#define RL(p) L(R(p))
/** starting from p, make two steps right */
#define RR(p) R(R(p))

/**
 * Test if an operator referenced via its container @a prop
 * may result a set of tuples instead of a bag of tuples.
 */
bool
PFprop_set (const PFprop_t *prop)
{
    return prop->set;
}

/**
 * worker for PFprop_infer_set
 * infers the set property during the second run
 * (uses edge counter stored in n->state_label from the first run)
 */
static void
prop_infer_set (PFla_op_t *n, bool set)
{
    bool l_set = false, 
         r_set = false;

    assert (n);

    /* collect the set properties of all parents */
    n->prop->set = n->prop->set && set;

    /* nothing to do if we haven't collected
       all incoming set properties of the parents */
    if (n->state_label > 1) {
        n->state_label--;
        return;
    }

    /* infer set property for children */
    switch (n->kind) {
        case la_serialize:
        case la_trace:
        case la_trace_msg:
        case la_trace_map:
            l_set = false;
            r_set = false;
            break;

        case la_lit_tbl:
        case la_empty_tbl:
            break;

        case la_attach:
        case la_project:
        case la_select:
        case la_fun_1to1:
        case la_num_eq:
        case la_num_gt:
        case la_bool_and:
        case la_bool_or:
        case la_bool_not:
        case la_type:
        case la_cast:
        case la_type_assert:
        case la_roots:
        case la_doc_tbl:
            l_set = n->prop->set;
            break;

        case la_cross:
        case la_eqjoin:
        case la_thetajoin:
        case la_cross_mvd:
        case la_eqjoin_unq:
        case la_disjunion:
            l_set = n->prop->set;
            r_set = n->prop->set;
            break;

        case la_intersect:
            l_set = true;
            r_set = true;
            break;

        case la_semijoin:
        case la_difference:
            l_set = n->prop->set;
            r_set = true;
            break;

        case la_distinct:
	case la_max:
	case la_min:
        case la_seqty1:
        case la_all:
            l_set = true;
            break;

        case la_avg:
        case la_sum:
        case la_count:
        case la_rownum:
        case la_number:
        case la_attribute:
        case la_textnode:
            l_set = false;
            break;

        case la_scjoin:
        case la_dup_scjoin:
            l_set = false;
            r_set = true;
            break;

        case la_doc_access:
            l_set = false;
            r_set = n->prop->set;
            break;

        case la_element:
        case la_element_tag:
        case la_merge_adjacent:
        case la_string_join:
            l_set = false;
            r_set = false;
            break;

        case la_docnode:
        case la_comment:
        case la_processi:
            assert (!"not implemented yet?");
            break;

        case la_frag_union:
            r_set = n->prop->set;
        case la_fragment:
        case la_empty_frag:
            l_set = n->prop->set;
            break;

        case la_cond_err:
            l_set = n->prop->set;
            r_set = false;
            break;

        /* For proxies we overwrite inferred property at its
           base by the input property of the proxy. */
        case la_proxy:
            prop_infer_set (L(n), n->prop->set);
            if (n->sem.proxy.kind == 1)
                n->sem.proxy.base1->prop->set = n->prop->set;

            /* start the property inference for the base operator */
            prop_infer_set (L(n->sem.proxy.base1),
                            n->sem.proxy.base1->prop->set);
            return;
            
        case la_proxy_base:
            /* avoid the inference as the proxy node
               may overwrite the property after traversal */
            return;

        case la_nil:
            break;

        /* ignore this property for the recursion */
        case la_rec_fix:
        case la_rec_param:
        case la_rec_arg:
            l_set = false;
            r_set = false;
        case la_rec_base:
            break;
            
        case la_dummy:
            l_set = n->prop->set;
            break;
    }

    /* infer properties for children */
    for (unsigned int i = 0; i < PFLA_OP_MAXCHILD && n->child[i]; i++)
        prop_infer_set (n->child[i], i==0?l_set:r_set);
}

/* worker for PFprop_infer_set */
static void
prop_infer (PFla_op_t *n)
{
    assert (n);

    /* count number of incoming edges
       (during first run) */
    n->state_label++;

    /* nothing to do if we already visited that node */
    if (n->bit_dag)
        return;
    /* otherwise initialize edge counter (first occurrence) */
    else
        n->state_label = 1;

    /* infer properties for children */
    for (unsigned int i = 0; i < PFLA_OP_MAXCHILD && n->child[i]; i++)
        prop_infer (n->child[i]);

    n->bit_dag = true;

    /* reset set property */
    n->prop->set = true;
}

/**
 * Infer set property (duplicates can be thrown away) 
 * for a DAG rooted in @a root
 */
void
PFprop_infer_set (PFla_op_t *root) {
    /* collect number of incoming edges (parents) */
    prop_infer (root);
    PFla_dag_reset (root);

    /* second run infers set property */
    prop_infer_set (root, false);
}

/* vim:set shiftwidth=4 expandtab filetype=c: */
