/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2008 CWI.
 * All Rights Reserved.
 */

/* getopt_long and getopt_long_only entry points for GNU getopt.
   Copyright (C) 1987,88,89,90,91,92,93,94,96,97,98
     Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <monetdb_config.h>

#include "monet_getopt.h"

#if 0				/* already taken care of in config.h */
#if !defined __STDC__ || !__STDC__
/* This is a separate conditional since some stdc systems
   reject `defined (const)'.  */
#ifndef const
#define const
#endif
#endif
#endif

#include <stdio.h>

/* Comment out all this code if we are using the GNU C Library, and are not
   actually compiling the library itself.  This code is part of the GNU C
   Library, but also included in many other GNU distributions.  Compiling
   and linking in this code is a waste when using the GNU C library
   (especially if it is a shared library).  Rather than having every GNU
   program understand `configure --with-gnu-libc' and omit the object files,
   it is simpler to just do this in the source for each such file.  */

#define GETOPT_INTERFACE_VERSION 2
#if !defined _LIBC && defined __GLIBC__ && __GLIBC__ >= 2
#include <gnu-versions.h>
#if _GNU_GETOPT_INTERFACE_VERSION == GETOPT_INTERFACE_VERSION
#define ELIDE_CODE
#endif
#endif

#ifndef ELIDE_CODE


/* This needs to come after some library #include
   to get __GNU_LIBRARY__ defined.  */
#ifdef __GNU_LIBRARY__
#include <stdlib.h>
#endif

#ifndef	NULL
#define NULL 0
#endif

int
getopt_long(int argc, char *const *argv, const char *options, const struct option *long_options, int *opt_index)
{
	return _getopt_internal(argc, argv, options, long_options, opt_index, 0);
}

/* Like getopt_long, but '-' as well as '--' can indicate a long option.
   If an option that starts with '-' (not '--') doesn't match a long option,
   but does match a short option, it is parsed as a short option
   instead.  */

int
getopt_long_only(int argc, char *const *argv, const char *options, const struct option *long_options, int *opt_index)
{
	return _getopt_internal(argc, argv, options, long_options, opt_index, 1);
}


#endif /* Not ELIDE_CODE.  */

#ifdef TEST

#include <stdio.h>

int
main(argc, argv)
int argc;
char **argv;
{
	int c;
	int digit_optind = 0;

	for (;;) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"add", 1, 0, 0},
			{"append", 0, 0, 0},
			{"delete", 1, 0, 0},
			{"verbose", 0, 0, 0},
			{"create", 0, 0, 0},
			{"file", 1, 0, 0},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "abc:d:0123456789", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf(" with arg %s", optarg);
			printf("\n");
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (digit_optind != 0 && digit_optind != this_option_optind)
				printf("digits occur in two different argv-elements.\n");
			digit_optind = this_option_optind;
			printf("option %c\n", c);
			break;

		case 'a':
			printf("option a\n");
			break;

		case 'b':
			printf("option b\n");
			break;

		case 'c':
			printf("option c with value `%s'\n", optarg);
			break;

		case 'd':
			printf("option d with value `%s'\n", optarg);
			break;

		case '?':
			break;

		default:
			printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	exit(0);
}

#endif /* TEST */
