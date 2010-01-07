/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
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
 * the Database Group at the Technische Universitaet Muenchen, Germany.
 * It is now maintained by the Database Systems Group at the Eberhard
 * Karls Universitaet Tuebingen, Germany.  Portions created by the
 * University of Konstanz, the Technische Universitaet Muenchen, and the
 * Universitaet Tuebingen are Copyright (C) 2000-2005 University of
 * Konstanz, (C) 2005-2008 Technische Universitaet Muenchen, and (C)
 * 2008-2010 Eberhard Karls Universitaet Tuebingen, respectively.  All
 * Rights Reserved.
 *
 * $Id$
 */

#ifndef CORE2ALG_H
#define CORE2ALG_H

#include "compile.h"

#include "array.h"

#include "core.h"
#include "pf_xq.h"

/** Compile XQuery Core into Relational Algebra */
struct PFla_op_t *PFcore2alg (PFcnode_t *, PFquery_t *PFquery,
                              enum PFoutput_format_t output_format);


/* ............. environment entry specification .............. */

/**
 * Each entry in the (variable) environment consists of a reference to
 * the variable (pointer to PFvar_t) and the algebra operator that
 * represents the variable. The environment itself will be represented
 * by an array.
 */

/** environment entry node */
struct PFla_env_t {
    PFvar_t               *var;
    struct PFla_op_t      *rel;
    struct PFla_op_t      *map;
    PFarray_t             *frag;
};
/** environment entry node */
typedef struct PFla_env_t PFla_env_t;


#endif   /* CORE2ALG_H */

/* vim:set shiftwidth=4 expandtab: */
