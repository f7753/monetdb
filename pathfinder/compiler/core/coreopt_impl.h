/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 *
 * Implementations for functions required in coreopt.mt
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


#include "pathfinder.h"

#include <limits.h>
#include <assert.h>
#include <string.h>

#include "coreopt.h"

/** twig-generated node identifiers */
#include "coreopt.symbols.h"

/** twig: type of tree node */
#define TWIG_NODE PFcnode_t

/** twig: max number of children under a core tree node */
#define TWIG_MAXCHILD PFCNODE_MAXCHILD

static int TWIG_ID[] = {
      [c_var]          var_       /**< variable */
    , [c_lit_str]      lit_str    /**< string literal */
    , [c_lit_int]      lit_int    /**< integer literal */
    , [c_lit_dec]      lit_dec    /**< decimal literal */
    , [c_lit_dbl]      lit_dbl    /**< double literal */
    , [c_nil]          nil        /**< end-of-sequence marker */

    , [c_seq]          seq        /**< sequence construction */

    , [c_let]          let        /**< let binding */
    , [c_for]          for_       /**< for binding */

    , [c_apply]        apply      /**< function application */
    , [c_arg]          arg        /**< function argument (list) */

    , [c_typesw]       typesw     /**< typeswitch clause */
    , [c_cases]        cases      /**< case concatenation for typeswitch */
    , [c_case]         case_      /**< single case for typeswitch */
    , [c_seqtype]      seqtype    /**< a SequenceType */
    , [c_seqcast]      seqcast    /**< cast along <: */
    , [c_proof]        proof      /**< type checker only: prove <: rel.ship */
    , [c_stattype]     stattype

    , [c_ifthenelse]   ifthenelse /**< if-then-else conditional */

    , [c_locsteps]     locsteps   /**< path of location steps only */

    , [c_ancestor]            ancestor
    , [c_ancestor_or_self]    ancestor_or_self
    , [c_attribute]           attribute
    , [c_child]               child_
    , [c_descendant]          descendant
    , [c_descendant_or_self]  descendant_or_self
    , [c_following]           following
    , [c_following_sibling]   following_sibling
    , [c_parent]              parent_
    , [c_preceding]           preceding
    , [c_preceding_sibling]   preceding_sibling
    , [c_self]                self

    , [c_kind_node]           kind_node
    , [c_kind_comment]        kind_comment
    , [c_kind_text]           kind_text
    , [c_kind_pi]             kind_pi
    , [c_kind_doc]            kind_doc
    , [c_kind_elem]           kind_elem
    , [c_kind_attr]           kind_attr

    , [c_namet]               namet

    /* Constructor Nodes */
    , [c_elem]         elem       /**< the element constructor */
    , [c_attr]         attr       /**< the attribute constructor */
    , [c_text]         text       /**< the text constructor */
    , [c_doc]          doc        /**< the document constructor */
    , [c_comment]      comment    /**< the comment constructor */
    , [c_pi]           pi         /**< the processing-instruction constructor */
    , [c_tag]          tag        /**< the tagname for element and attribute constructors */

    , [c_true]         true_      /**< Built-in function 'true' */
    , [c_false]        false_     /**< Built-in function 'false' */
    , [c_error]        error      /**< Built-in function 'error' */
    , [c_root]         root_      /**< Built-in function 'root' */
    , [c_empty]        empty_     /**< Built-in function 'empty' */
};

/** twig: setup twig */
#include "twig.h"

#undef var_       
#undef lit_str    
#undef lit_int    
#undef lit_dec    
#undef lit_dbl    
#undef nil        
#undef seq        
#undef let        
#undef for_       
#undef apply      
#undef arg        
#undef typesw     
#undef cases      
#undef case_      
#undef seqtype    
#undef seqcast     
#undef proof
#undef ifthenelse 
#undef locsteps   
#undef ancestor
#undef ancestor_or_self
#undef attribute
#undef child_
#undef descendant
#undef descendant_or_self
#undef following
#undef following_sibling
#undef parent_
#undef preceding
#undef preceding_sibling
#undef self
#undef kind_node
#undef kind_comment
#undef kind_text
#undef kind_pi
#undef kind_doc
#undef kind_elem
#undef kind_attr
#undef namet
#undef elem
#undef attr 
#undef text
#undef doc 
#undef comment
#undef pi  
#undef tag
#undef true_  
#undef false_ 
#undef error  
#undef root_  
#undef empty_ 

#include "subtyping.h"

/** 
 * Phases for Core tree optimization
 *
 *  - phase1
 */
enum {
      phase1 = 0x01
};

/** 
 * Core tree optimization (with known type information).
 *
 * @param r root of core tree
 * @return root of optimized core tree
 */
PFcnode_t *
PFcoreopt (PFcnode_t *r)
{
    return rewrite (r, (int []) { phase1, 
                                  -1 
                                });
}

/* vim:set shiftwidth=4 expandtab: */
