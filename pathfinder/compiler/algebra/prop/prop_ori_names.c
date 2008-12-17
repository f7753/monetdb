/**
 * @file
 *
 * Inference of original column names in logical algebra expressions.
 *
 * The main function PFprop_infer_ori_names() collects in a first DAG
 * traversal the number of incoming edges for each operator.
 * The second DAG traversal (infer_ori_names()) proceeds Top-Down
 * and first collects all incoming name pairs. The columns not referenced
 * by the parent operators are then initialized and the name pair lists
 * for the children are prepared before the children are processed.
 * In a last step the name pair lists of the children are incorporated
 * in the operators (patch_ori_names()).
 * The name mapping may then trigger projections whenever the names
 * of the child operators conflict.
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
 * 2000-2005 University of Konstanz and (C) 2005-2008 Technische
 * Universitaet Muenchen, respectively.  All Rights Reserved.
 *
 * $Id$
 */

/* always include pathfinder.h first! */
#include "pathfinder.h"
#include <assert.h>
#include <stdio.h>

#include "properties.h"
#include "alg_dag.h"
#include "oops.h"
#include "mem.h"

/* Easily access subtree-parts */
#include "child_mnemonic.h"

#define ARRAY_SIZE(n) ((n)->schema.count > 10 ? (n)->schema.count : 10)

/* reuse the icols field to maintain the bitlist of free variables */
#define FREE(n) ((n)->prop->free_cols)
/* initial value for lists that encode free variables */
#define ALL (~col_NULL)

/* worker for PFprop_ori_name* */
static PFalg_col_t
find_ori_name (PFarray_t *np_list, PFalg_col_t col)
{
    if (!np_list) return 0;

    for (unsigned int i = 0; i < PFarray_last (np_list); i++)
        if (col == ((name_pair_t *) PFarray_at (np_list, i))->unq)
            return ((name_pair_t *) PFarray_at (np_list, i))->ori;

    return 0;
}

/**
 * Return original name of unique column @a col stored
 * in property container @a prop.
 */
PFalg_col_t
PFprop_ori_name (const PFprop_t *prop, PFalg_col_t col)
{
    assert (prop);
    return find_ori_name (prop->name_pairs, col);
}

/**
 * Return original name of column @a col stored
 * in the left name mapping field of property container @a prop.
 */
PFalg_col_t
PFprop_ori_name_left (const PFprop_t *prop, PFalg_col_t col)
{
    assert (prop);
    return find_ori_name (prop->l_name_pairs, col);
}

/**
 * Return original name of column @a col stored
 * in the right name mapping field of property container @a prop.
 */
PFalg_col_t
PFprop_ori_name_right (const PFprop_t *prop, PFalg_col_t col)
{
    assert (prop);
    return find_ori_name (prop->r_name_pairs, col);
}

/**
 * Add a new original name/unique name pair to the list of name pairs @a np
 */
static void
add_name_pair (PFarray_t *np, PFalg_col_t ori, PFalg_col_t unq)
{
    assert (np);

    *(name_pair_t *) PFarray_add (np)
        = (name_pair_t) { .ori = ori, .unq = unq };
}

/**
 * Returns difference of free variable lists
 */
static PFalg_col_t
diff (PFalg_col_t a, PFalg_col_t b)
{
    return a & (~b);
}

/**
 * Builds the difference of the unique column @a unq
 * and the name pair list @a np_list (using its unq field).
 * The difference is returned in a new list.
 */
static void
diff_np (PFarray_t  *ret, PFarray_t *np_list, PFalg_col_t unq)
{
    name_pair_t np;

    for (unsigned int i = 0; i < PFarray_last (np_list); i++) {
        np = *(name_pair_t *) PFarray_at (np_list, i);
        if (np.unq != unq)
            *(name_pair_t *) PFarray_add (ret) = np;
    }
}


/**
 * patch_ori_names replaces all proposed original names in the
 * @a np_list list whose name was not accepted by the child operator.
 * List @a child_np_list contains the replacements: updated original
 * column names.
 */
static void
patch_ori_names (PFarray_t *np_list, PFarray_t *child_np_list)
{
    PFalg_col_t child_unq, child_ori, unq, ori;

    for (unsigned int i = 0; i < PFarray_last (child_np_list); i++) {
        child_unq = ((name_pair_t *) PFarray_at (child_np_list, i))->unq;
        child_ori = ((name_pair_t *) PFarray_at (child_np_list, i))->ori;

        for (unsigned int j = 0; j < PFarray_last (np_list); j++) {
            unq = ((name_pair_t *) PFarray_at (np_list, j))->unq;
            ori = ((name_pair_t *) PFarray_at (np_list, j))->ori;

            if (child_unq == unq && child_ori != ori) {
                /* replace value -- the mapping has to translate this
                   into a renaming projection */
                ((name_pair_t *) PFarray_at (np_list, j))->ori = child_ori;
                break;
            }
        }
    }
}

/**
 * Infer original name properties; worker for prop_infer().
 */
static PFarray_t *
infer_ori_names (PFla_op_t *n, PFarray_t *par_np_list)
{
    PFalg_col_t unq, ori;
    PFalg_col_t par_unq, par_ori;
    PFarray_t *np_list;

    assert (n);

    np_list = n->prop->name_pairs;

    /* include (possibly) new matching columns in the name pairs list */
    for (unsigned int i = 0; i < PFarray_last (par_np_list); i++) {
        par_unq = ((name_pair_t *) PFarray_at (par_np_list, i))->unq;
        par_ori = ((name_pair_t *) PFarray_at (par_np_list, i))->ori;

        /* skip the rest of the loop if column is already present */
        if (find_ori_name (np_list, par_unq)) continue;

        /* name pair is missing - add it to the name pairs list ... */
        for (unsigned int i = 0; i < n->schema.count; i++)
            /* ... if it correspond to a column of the current schema */
            if (n->schema.items[i].name == par_unq) {
                /* The proposed original column name is already
                   in use - thus replace it by a new one. */
                if (!(FREE(n) & par_ori))
                    par_ori = PFcol_ori_name (par_unq, FREE(n));

                FREE(n) = diff (FREE(n), par_ori);
                add_name_pair (np_list, par_ori, par_unq);
                break;
            }
    }

    PFprop_refctr (n) = PFprop_refctr (n) - 1;
    /* nothing to do if we haven't collected
       all incoming name pair lists of that node */
    if (PFprop_refctr (n) > 0) {
        /* return the current mappings to the parent operator */
        return np_list;
    }

    /* create new names for the remaining names that are visible
       in the current operator but not referenced by parent operators */
    for (unsigned int i = 0; i < n->schema.count; i++) {
        unq = n->schema.items[i].name;
        if (!find_ori_name (np_list, unq)) {
            PFalg_col_t ori = PFcol_ori_name (unq, FREE(n));
            add_name_pair (np_list, ori, unq);
            FREE(n) = diff (FREE(n), ori);
        }
    }

    /* create name pair lists for the child operators */
    switch (n->kind) {
        case la_serialize_seq:
        case la_serialize_rel:
            n->prop->r_name_pairs = PFarray_copy (np_list);
            break;

        case la_side_effects:
            /* do not infer name pairs to the children */
            break;

        case la_lit_tbl:
        case la_empty_tbl:
        case la_ref_tbl:
            break;

        case la_attach:
            /* infer all columns except the one introduced by the attach
               operator */
            diff_np (n->prop->l_name_pairs, np_list, n->sem.attach.res);
            break;

        case la_cross:
        case la_thetajoin:
        case la_eqjoin:
            /* only infer matching columns to the children */
            for (unsigned int i = 0; i < L(n)->schema.count; i++) {
                unq = L(n)->schema.items[i].name;
                ori = find_ori_name (np_list, unq);
                add_name_pair (n->prop->l_name_pairs, ori, unq);
            }
            for (unsigned int i = 0; i < R(n)->schema.count; i++) {
                unq = R(n)->schema.items[i].name;
                ori = find_ori_name (np_list, unq);
                add_name_pair (n->prop->r_name_pairs, ori, unq);
            }
            break;

        case la_internal_op:
            /* interpret this operator as internal join */
            if (n->sem.eqjoin_opt.kind == la_eqjoin) {
                /* do the same as for normal joins and
                   correctly update the columns names */
#define proj_at(l,i) (*(PFalg_proj_t *) PFarray_at ((l),(i)))
                PFarray_t   *lproj = n->sem.eqjoin_opt.lproj,
                            *rproj = n->sem.eqjoin_opt.rproj;
                unsigned int i;
                PFalg_col_t  unq_old,
                             unq_new,
                             ori_new,
                             unq_col2 = proj_at (rproj, 0).old,
                             ori_col2 = PFcol_ori_name (unq_col2, FREE(n));
                FREE(n) = diff (FREE(n), ori_col2);

                /* create name pair list for the left operand */
                for (i = 0; i < PFarray_last (lproj); i++) {
                    unq_new = proj_at (lproj, i).new;
                    unq_old = proj_at (lproj, i).old;
                    ori_new = find_ori_name (np_list, unq_new);
                    add_name_pair (n->prop->l_name_pairs, ori_new, unq_old);
                }

                /* create name pair list for the right operand */
                /* add the join column separately ... */
                add_name_pair (n->prop->r_name_pairs, ori_col2, unq_col2);
                /* ... and discard the join column in the iteration */
                for (i = 1; i < PFarray_last (rproj); i++) {
                    unq_new = proj_at (rproj, i).new;
                    unq_old = proj_at (rproj, i).old;
                    ori_new = find_ori_name (np_list, unq_new);
                    add_name_pair (n->prop->r_name_pairs, ori_new, unq_old);
                }
            }
            else
                PFoops (OOPS_FATAL,
                        "internal optimization operator is not allowed here");
            break;

        case la_semijoin:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            /* the left and the right join argument may both
               have the same name as only the left one survives */
            add_name_pair (n->prop->r_name_pairs,
                           find_ori_name (np_list, n->sem.eqjoin.col1),
                           n->sem.eqjoin.col2);
            break;

        case la_project:
            /* Infer only columns that are used in the projection.
               Renamings do not have to be mapped. Conflicting names
               are patched by the function patch_ori_names(). */
            for (unsigned int i = 0; i < n->sem.proj.count; i++) {
                ori = find_ori_name (np_list, n->sem.proj.items[i].new);
                add_name_pair (n->prop->l_name_pairs,
                               ori,
                               n->sem.proj.items[i].old);
            }
            break;

        case la_select:
        case la_pos_select:
        case la_distinct:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            break;

        case la_disjunion:
        case la_intersect:
        case la_difference:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            n->prop->r_name_pairs = PFarray_copy (np_list);
            break;

        case la_fun_1to1:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.fun_1to1.res);
            break;

        case la_num_eq:
        case la_num_gt:
        case la_bool_and:
        case la_bool_or:
        case la_to:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.binary.res);
            break;

        case la_bool_not:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.unary.res);
            break;

        case la_avg:
        case la_max:
        case la_min:
        case la_sum:
        case la_count:
        case la_seqty1:
        case la_all:
            /* operators introducing a completely new set of names
               for its operands (aggregates, steps, doc_tbl, element,
               merge_adjacent, and string_join) store these columns
               both in the name pair list of the child and in the current
               name pair list to support correct renaming if the proposed
               column name was modified. */
            unq = n->sem.aggr.res;
            ori = find_ori_name (np_list, unq);
            /* use the result name also as column name
               of the input value column */
            if (n->kind != la_count) {
                unq = n->sem.aggr.col;
                add_name_pair (np_list, ori, unq);
                add_name_pair (n->prop->l_name_pairs, ori, unq);
            }

            if (n->sem.aggr.part) {
                unq = n->sem.aggr.part;
                ori = find_ori_name (np_list, unq);
                add_name_pair (n->prop->l_name_pairs, ori, unq);
            }
            break;

        case la_rownum:
        case la_rowrank:
        case la_rank:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.sort.res);
            break;

        case la_rowid:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.rowid.res);
            break;

        case la_type:
        case la_cast:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.type.res);
            break;

        case la_type_assert:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            break;

        case la_step:
        case la_guide_step:
            /* input iter column */
            unq = n->sem.step.iter;
            ori = find_ori_name (np_list, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* input item column (use same name as for output) */
            ori = find_ori_name (np_list, n->sem.step.item_res);
            unq = n->sem.step.item;
            if (!find_ori_name (np_list, unq))
                add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);
            break;

        case la_step_join:
        case la_guide_step_join:
            diff_np (n->prop->r_name_pairs, np_list, n->sem.step.item_res);
            break;

        case la_doc_index_join:
            diff_np (n->prop->r_name_pairs, np_list, n->sem.doc_join.item_res);
            break;

        case la_doc_tbl:
            diff_np (n->prop->l_name_pairs, np_list, n->sem.doc_tbl.res);
            break;

        case la_doc_access:
            diff_np (n->prop->r_name_pairs, np_list, n->sem.doc_access.res);
            break;

        case la_twig:
        case la_fcns:
            break;

        case la_docnode:
            /* input iter column */
            ori = col_iter;
            unq = n->sem.docnode.iter;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);
            break;

        case la_element:
        case la_textnode:
        case la_comment:
            /* input iter column */
            ori = col_iter;
            unq = n->sem.iter_item.iter;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);

            /* input item column */
            ori = col_item;
            unq = n->sem.iter_item.item;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);
            break;

        case la_attribute:
        case la_processi:
            /* input iter column */
            ori = col_iter;
            unq = n->sem.iter_item1_item2.iter;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);

            /* input item1 column */
            ori = col_item;
            unq = n->sem.iter_item1_item2.item1;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);

            /* input item2 column */
            ori = col_item1;
            unq = n->sem.iter_item1_item2.item2;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);
            break;

        case la_content:
            /* input iter column */
            ori = col_iter;
            unq = n->sem.iter_pos_item.iter;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* input pos column */
            ori = PFcol_ori_name (n->sem.iter_pos_item.pos, ~col_iter);
            unq = n->sem.iter_pos_item.pos;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* input item column */
            ori = PFcol_ori_name (n->sem.iter_pos_item.item,
                                  ~(col_iter | ori));
            unq = n->sem.iter_pos_item.item;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);
            break;

        case la_merge_adjacent:
            /* input iter column (use same name as for output) */
            ori = find_ori_name (np_list,
                                 n->sem.merge_adjacent.iter_res);
            unq = n->sem.merge_adjacent.iter_in;
            assert (n->sem.merge_adjacent.iter_in ==
                    n->sem.merge_adjacent.iter_res);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* input pos column (use same name as for output) */
            ori = find_ori_name (np_list,
                                 n->sem.merge_adjacent.pos_res);
            unq = n->sem.merge_adjacent.pos_in;
            assert (n->sem.merge_adjacent.pos_in ==
                    n->sem.merge_adjacent.pos_res);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* input item column (use same name as for output) */
            ori = find_ori_name (np_list,
                                 n->sem.merge_adjacent.item_res);
            unq = n->sem.merge_adjacent.item_in;
            if (!find_ori_name (np_list, unq))
                add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);
            break;

        case la_roots:
        {
            PFarray_t *child_np_list;
            PFalg_col_t unq, ori;

            /* child_np_list is expected to be equal to np_list,
               meaning that the constructor in the child node
               accepted all variable names. If they differ the
               constructor is probably called more than once. */
            child_np_list = infer_ori_names (L(n), np_list);

            /* make sure that no projection is required after
               the roots operator */
            for (unsigned int j = 0; j < PFarray_last (np_list); j++) {
                unq = ((name_pair_t *) PFarray_at (np_list, j))->unq;
                ori = ((name_pair_t *) PFarray_at (np_list, j))->ori;

                if (find_ori_name (child_np_list, unq) != ori)
                    PFoops (OOPS_FATAL,
                            "a constructor has to have exactly one "
                            "referencing root node");
            }

            n->prop->l_name_pairs = PFarray_copy (np_list);

            return np_list;
        } break;

        case la_fragment:
        case la_frag_extract:
        case la_frag_union:
        case la_empty_frag:
        case la_fun_frag_param:
            /* do not infer name pairs to the children */
            break;

        case la_error:
            /* do not infer name pairs to the other sie effects */
            n->prop->r_name_pairs = PFarray_copy (np_list);
            break;

        case la_trace:
            /* do not infer name pairs to the children */
            break;

        case la_trace_items:
        case la_trace_msg:
        case la_trace_map:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            break;

        case la_nil:
            /* we do not have properties */
            break;

        case la_rec_fix:
            n->prop->r_name_pairs = PFarray_copy (np_list);
            break;

        case la_rec_param:
            /* do not infer name pairs to the children */
            break;

        case la_rec_arg:
            /* The both inputs (seed and recursion) may not use
               the same column names. Thus we live with inconsistent
               original names and introduce a renaming projection
               (Schema L -> Schema Base and Schema R -> Schema Base)
               during name mapping. */
            n->prop->l_name_pairs = PFarray_copy (np_list);
            n->prop->r_name_pairs = PFarray_copy (np_list);
            break;

        case la_rec_base:
            break;

        case la_fun_call:
            break;

        case la_fun_param:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            break;

        case la_proxy:
        case la_proxy_base:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            break;

        case la_string_join:
            /* input iter column (use same name as for output) */
            ori = find_ori_name (np_list, n->sem.string_join.iter_res);
            /* value iter column */
            unq = n->sem.string_join.iter;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);
            /* separator iter column */
            unq = n->sem.string_join.iter_sep;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* input item column (use same name as for output) */
            ori = find_ori_name (np_list, n->sem.string_join.item_res);
            /* value item column */
            unq = n->sem.string_join.item;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);
            /* separator item column */
            unq = n->sem.string_join.item_sep;
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->r_name_pairs, ori, unq);

            /* introduce new column pos
               (FREE(n) ensures that item and
                res column names are not used) */
            unq = n->sem.string_join.pos;
            ori = PFcol_ori_name (unq, FREE(n));
            add_name_pair (np_list, ori, unq);
            add_name_pair (n->prop->l_name_pairs, ori, unq);
            FREE(n) = diff (FREE(n), ori);
            break;

        case la_dummy:
            n->prop->l_name_pairs = PFarray_copy (np_list);
            break;
    }

    /* infer properties for children and update proposals of original
       column names if they were modified by the child operator */
    if (L(n))
        patch_ori_names (n->prop->l_name_pairs,
                         infer_ori_names (L(n), n->prop->l_name_pairs));

    if (R(n))
        patch_ori_names (n->prop->r_name_pairs,
                         infer_ori_names (R(n), n->prop->r_name_pairs));

    /* return the current mappings to the parent operator */
    return np_list;
}

/* worker for PFprop_infer_ori_names */
static void
reset_fun (PFla_op_t *n)
{
    /* reset the original name information */
    if (n->prop->name_pairs)
        PFarray_last (n->prop->name_pairs) = 0;
    else
        n->prop->name_pairs = PFarray (sizeof (name_pair_t),
                                       ARRAY_SIZE(n));

    if (L(n)) {
        if (n->prop->l_name_pairs)
            PFarray_last (n->prop->l_name_pairs) = 0;
        else
            n->prop->l_name_pairs = PFarray (sizeof (name_pair_t),
                                             ARRAY_SIZE(L(n)));
    }

    if (R(n)) {
        if (n->prop->r_name_pairs)
            PFarray_last (n->prop->r_name_pairs) = 0;
        else
            n->prop->r_name_pairs = PFarray (sizeof (name_pair_t),
                                             ARRAY_SIZE(R(n)));
    }

    /* reset the list of available column names */
    FREE(n) = ALL;
}

/**
 * Infer original names for a DAG rooted in root
 */
void
PFprop_infer_ori_names (PFla_op_t *root)
{
    /* collect number of incoming edges (parents) */
    PFprop_infer_refctr (root);

    /* reset the old property information */
    PFprop_reset (root, reset_fun);

    /* infer new original names property */
    infer_ori_names (root,
                     PFarray (sizeof (name_pair_t), 0));
}

/* vim:set shiftwidth=4 expandtab: */
