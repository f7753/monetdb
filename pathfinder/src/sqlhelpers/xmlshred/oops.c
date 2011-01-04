/**
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
 * 2008-2011 Eberhard Karls Universitaet Tuebingen, respectively.  All
 * Rights Reserved.
 *
 * $Id$
 */

#include "pf_config.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "oops.h"
#include <assert.h>

#define PUFFER_SIZE 1024

/* worker for SHoops */
static void
SHoops_ (int err, const char *fmt, va_list az) 
{
    assert (fmt);

    /* error msg from errno */
    int errmsg = errno;
        char *emsg;
    char puffer[PUFFER_SIZE+1];

    vsnprintf (puffer, PUFFER_SIZE, fmt, az);
    puffer[PUFFER_SIZE] = 0;
    if (err) {
        emsg = strerror (errmsg);
        snprintf (puffer+strlen(puffer), 
                  PUFFER_SIZE - strlen (puffer), 
                  ": %s", emsg);
        puffer[PUFFER_SIZE] = 0;
    }
    fflush(stdout); /* if stdout and stderr are equal */
    /* write everything in the puffer to stderr */
    fprintf (stderr, "%s\n", puffer);
    fflush (NULL);
    return;
}

/* Global error-message routine */
void
SHoops (err_t err, const char *fmt, ...)
{
    assert (fmt);

    va_list az;
        
    va_start (az, fmt);
    switch (err) {
        case SH_WARNING:
        case SH_FATAL:
            SHoops_ (0, fmt, az);
            break;
        case SH_DUMP:
            SHoops_ (1, fmt, az);
            break;
        default:
            SHoops_ (1, "This err_msg is unknown", az);
            exit (3);
    }
    va_end (az);

    if (err == SH_WARNING)
        return;
    else if (err == SH_DUMP)
        abort ();
    exit (1);
}
