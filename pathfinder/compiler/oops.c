/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 * Stack-based error handling.
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

#include "pathfinder.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "oops.h"

#include "mem.h"

/**
 * This specifices a mapping from error codes (OOPS_*, see oops.h)
 * to a message describing the general kind of the error.  Functions
 * detecting a failure are expected to provide a more detailed
 * description when they call PFoops().
 */
static char *oops_msg[] = {
     [OOPS_OK]                  "successful"
    ,[-OOPS_FATAL]              "fatal error"
    ,[-OOPS_NOTICE]             "notice"
    ,[-OOPS_UNKNOWNERROR]       "unknown error"
    ,[-OOPS_CMDLINEARGS]        "unknown or incomplete command line argument"
    ,[-OOPS_PARSE]              "parse error"
    ,[-OOPS_OUTOFMEM]           "insufficient memory"
    ,[-OOPS_BADNS]              "bad usage of XML namespaces"
    ,[-OOPS_UNKNOWNVAR]         "variable(s) out of scope or unknown"
    ,[-OOPS_NESTDEPTH]          "query nested too deeply"
    ,[-OOPS_NOCONTEXT]          "illegal reference to context node"
    ,[-OOPS_NOSERVICE]          "invalid TCP port (privileged?)"
    ,[-OOPS_TAGMISMATCH]        "XML start/end tags do not match"
    ,[-OOPS_NOTPRETTY]          "prettyprinting problem"
    ,[-OOPS_APPLYERROR]         "error in function application"
    ,[-OOPS_FUNCREDEF]          "function redefined"
    ,[-OOPS_DUPLICATE_KEY]      "duplicate key in environment"
    ,[-OOPS_TYPENOTDEF]         "use of undefined type"
    ,[-OOPS_TYPEREDEF]          "duplicate type names in one symbol space"
    ,[-OOPS_TYPECHECK]          "type error"
    ,[-OOPS_SCHEMAIMPORT]       "XML Schema import"
    ,[-OOPS_BURG]               "tree matching"
    ,[-OOPS_WARNING]            "warning" /* only warnings below */
    ,[-OOPS_WARN_NOTSUPPORTED]  "warning: unsupported feature"
    ,[-OOPS_WARN_VARREUSE]      "warning: variable reuse"
};

/** 
 * Maximum length of error message strings (messages will be
 *  truncated if necessary) 
 */
#define OOPS_SIZE   1024

/**
 * Log message to compiler log file. This function actually does the
 * work for PFinfo() and PFlog().
 * @param msg printf style format string
 * @param msgs argument list for printf format string. See also the
 *   va_start manpage.
 */
static void
log_worker (const char *msg, va_list msgs)
{
    time_t now;
    char *anow;
    char buf[OOPS_SIZE];
    int n;

    /* generate time stamp and write to log file */
    (void) time (&now);
    anow = asctime (localtime (&now));
    anow[strlen (anow) - 1] = '\0';

    n = snprintf (buf, OOPS_SIZE, "%s: ", anow);
    write (fileno (stderr), buf, n < OOPS_SIZE ? n : OOPS_SIZE - 1);
    
    n = vsnprintf (buf, OOPS_SIZE, msg, msgs);
    write (fileno (stderr), buf, n < OOPS_SIZE ? n : OOPS_SIZE - 1);

    write (fileno (stderr), "\n", 1);
}

/**
 * Write formatted string with time stamp to compiler log file, even if
 * '-q' command line switch set.
 *
 * @param msg printf style format string for message to log
 */
void
PFlog (const char *msg, ...)
{
    va_list msgs;

    va_start (msgs, msg);
    log_worker (msg, msgs);
    va_end (msgs);
}

/**
 * Does the work for #PFoops and #PFoops_loc.
 */
void
oops (PFrc_t rc, bool halt, 
      const char *file, const char *func, const int line,
      const char *msg, va_list msgs)
{
    char *mbuf;
    size_t nmsg;

    mbuf = PFstrndup (oops_msg[-rc], OOPS_SIZE);

    if (msg) {
	/* generate an error message of the form `<oops_msg>: <msg>' */
	nmsg = strlen (oops_msg[-rc]);
	mbuf[nmsg] = ':';
	mbuf[nmsg + 1] = ' ';

	vsnprintf (mbuf + nmsg + 2, OOPS_SIZE - nmsg - 2 - 1, msg, msgs);

        PFlog ("%s", mbuf);
    }

    /* halt the compiler if requested */
    if (halt) {
#ifndef NDEBUG
        /* if this is a debug version of Pathfinder, log source location */
        PFlog ("halted in %s (%s), line %d", file, func, line);
#else
	/* fool compilers that otherwise complain about unused parameters */
	(void)file;
	(void)func;
	(void)line;
#endif

        exit (-rc);
    }
}


/**
 * Logs message code @a rc and message string @a msg, then halts 
 * the compiler.
 *
 * You are expected to call this function whenever you encounter a
 * failure or error condition you cannot handle locally: @a rc
 * describes the general kind of the error (see oops.h for known
 * error kinds OOPS_*), @a msg is meant to be a detailed description of
 * the failure (you may use `printf'-like formatting).  
 *
 * NB. DOES NOT RETURN.
 *
 * @param rc   error code
 * @param file C file in which the error has occured
 * @param func C function where the error has occured
 * @param line line number the error has occured
 * @param msg  error message string
 * @return Returns rc
 */
void
PFoops_ (PFrc_t rc, 
         const char *file, const char *func, const int line,
         const char *msg, ...)
{
    va_list args;

    va_start (args, msg);

    /* does not return */
    oops (rc, true, file, func, line, msg, args);
    
    exit (EXIT_FAILURE);
}

/**
 * See #PFoops. In addition, this function handles a location passed
 * as @a loc. The resulting string looks like "at (1,1-3,7): ...".
 */
void
PFoops_loc_ (PFrc_t rc, PFloc_t loc, 
             const char *file, const char *func, const int line,
             const char *msg, ...)
{
    va_list args;
    char    buf[OOPS_SIZE];

    snprintf (buf, OOPS_SIZE, "at (%i,%i-%i,%i): %s",
              loc.first_row, loc.first_col,
              loc.last_row, loc.last_col, msg);

    va_start (args, msg);

    /* does not return */
    oops (rc, true, file, func, line, buf, args);

    exit (EXIT_FAILURE);
}

/**
 * Log an informational message, but only if '-q' command line switch
 * not given.  Then return.
 *
 * @param rc  error code
 * @param msg printf style format string for message to log
 */
void
PFinfo (PFrc_t rc, const char *msg, ...)
{
    va_list args;

    /* no logging if we are supposed to be quiet */
    if (PFstate.quiet)
        return;

    va_start (args, msg);
    oops (rc, false, 0, 0, 0, msg, args); 
    va_end (args);
}

/**
 * See #PFinfo. In addition, this function handles a location passed
 * as @a loc. The resulting string looks like "at (1,1-3,7): ...".
 */
void
PFinfo_loc (PFrc_t rc, PFloc_t loc, const char *msg, ...)
{
    va_list args;
    char    buf[OOPS_SIZE];

    /* no logging if we are supposed to be quiet */
    if (PFstate.quiet)
        return;

    snprintf (buf, OOPS_SIZE, "at (%i,%i-%i,%i): %s",
              loc.first_row, loc.first_col,
              loc.last_row, loc.last_col, msg);

    va_start (args, msg);
    oops (rc, false, 0, 0, 0, buf, args);
    va_end (args);
}

/* vim:set shiftwidth=4 expandtab: */
