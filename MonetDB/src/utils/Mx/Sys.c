/*
 * The contents of this file are subject to the MonetDB Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the Monet Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2005 CWI.
 * All Rights Reserved.
 */

#include	<monetdb_config.h>
#include	<stdio.h>
#include	<ctype.h>
#include 	<stdlib.h>
#include 	<string.h>

#include	"Mx.h"
#include	"MxFcnDef.h"
#include 	<stdarg.h>	/* va_alist.. */

int mx_out = 1;
extern int somethingPrinted;	/* used for preventing to empty display lines */

/* VARARGS */
void
ofile_printf(char *format, ...)
{
	va_list ap;

	va_start(ap, format);

/*	format = va_arg(ap, char*);*/
	if (mx_out & 1)
		vfprintf(ofile, format, ap);
	va_start(ap, format);
	if (ofile_index && (mx_out & 2))
		vfprintf(ofile_index, format, ap);
	va_start(ap, format);
	if (ofile_body && (mx_out & 4))
		vfprintf(ofile_body, format, ap);
	va_end(ap);
	somethingPrinted++;
}

void
ofile_puts(char *s)
{
	if (mx_out & 1)
		fputs(s, ofile);
	if (ofile_index && (mx_out & 2))
		fputs(s, ofile_index);
	if (ofile_body && (mx_out & 4))
		fputs(s, ofile_body);
	somethingPrinted++;
}

void
ofile_putc(char c)
{
	if (mx_out & 1)
		fputc(c, ofile);
	if (ofile_index && (mx_out & 2))
		fputc(c, ofile_index);
	if (ofile_body && (mx_out & 4))
		fputc(c, ofile_body);
	somethingPrinted++;
}


void
Fatal(char *fcn, char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	fprintf(stderr, "Mx(%s):", fcn);
	vfprintf(stderr, format, ap);
	if (mx_file)
		fprintf(stderr, "[%s:%d]", mx_file, mx_line);
	fprintf(stderr, ".\n");
	va_end(ap);

	exit(1);
}


char *
Malloc(size_t size)
{
	char *buf;

	if ((buf = malloc((unsigned) size)) == 0)
		Fatal("Malloc", "Not enough memory");
	return buf;
}

void
Free(char *ptr)
{
	(void) ptr;

/*
	free(ptr);
 */
}

char *
StrDup(const char *str)
{
	return strcpy(Malloc(strlen(str) +1), str);
}

 /*VARGARGS*/ void
Error(char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	fprintf(stderr, "Mx:");
	vfprintf(stderr, format, ap);
	if (mx_file)
		fprintf(stderr, "[%s:%d]", mx_file, mx_line);
	fprintf(stderr, ".\n");
	mx_err++;
	va_end(ap);
}

/*VARGARGS1*/
void
Message(char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	vfprintf(stderr, format, ap);
	if (mx_file)
		fprintf(stderr, "[%s:%d]", mx_file, mx_line);
	fprintf(stderr, ".\n");
	va_end(ap);
}
