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
 * is now maintained by the Database Systems Group at the Technische
 * Universitaet Muenchen, Germany.  Portions created by the University of
 * Konstanz and the Technische Universitaet Muenchen are Copyright (C)
 * 2000-2005 University of Konstanz and (C) 2005-2007 Technische
 * Universitaet Muenchen, respectively.  All Rights Reserved.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "shred_helper.h"
#include "oops.h"

/**
 * Alternative definition of strdup. It just duplicates a string
 * given by @a s.
 *
 * @param s  String to duplicate.
 */
char *
strdup (const char * s)
{
    assert (s);
    size_t len = strlen (s);
	char * new = malloc ((len+1) * sizeof(char));
	if (new == NULL)
	    return NULL;
    new[len+1] = '\0';
    return (char *)memcpy (new, s, len);
}

/**
 * Alternative definition of strdup. It just duplicates a string
 * given by @a s.
 * If the length exceeds n duplicate only the first @a n characters.
 *
 * @param s  String to duplicate.
 * @param n  Copy only the first n characters.
 */
char *
strndup (const char * s, size_t n)
{
    assert (s);
    size_t len = strlen (s);
	len = (len > n)?n:len;
	char * new = malloc ((len+1) * sizeof(char));
	if (new == NULL)
	    return NULL;
    new[len+1] = '\0';
    return (char *)memcpy (new, s, len);
}

/**
 * Test if we have the right to read the given path.
 */
bool 
SHreadable (const char *path)
{
    assert (path);
    if (access (path, F_OK) < 0)
	    return false;
    if (access (path, R_OK) < 0)
	    return false;
	return true;
}

/**
 * Test if the given @a path exists. 
 */
bool
SHexists (const char *path)
{
    assert (path);
    if (access (path, F_OK) < 0)
	    return false;
	return true;
}

/**
 * Open file @a path for writing.
 */
FILE *
SHopen_write (const char *path)
{
    assert (path);
    FILE * ret;	

    /* open file */
    ret = fopen (path, "w");

    if (!ret)
        SHoops (SH_FATAL, "Could not open file");

    return ret;
}
