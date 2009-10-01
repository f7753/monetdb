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
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2009 MonetDB B.V.
 * All Rights Reserved.
 */

/**
 * properties
 * Fabian Groffen
 * Simple functions that deal with the property file
 */

#include "sql_config.h"
#include "properties.h"
#include "utils.h"
#include <stdio.h> /* fprintf, fgets */
#include <string.h> /* memcpy */
#include <gdk.h> /* GDKmalloc */

#define MEROPROPFILEHEADER \
	"# DO NOT EDIT THIS FILE - use monetdb(1) to set properties\n" \
	"# This file is used by merovingian and monetdb\n"

static confkeyval _internal_prop_keys[] = {
	{"forward",  NULL, OTHER},
	{"shared",   NULL, STR},
	{"nthreads", NULL, INT},
	{"master",   NULL, BOOL},
	{"slave",    NULL, MURI},
	{ NULL,      NULL, INVALID}
};

/**
 * Returns the currently supported list of properties.  This list can be
 * used to read all values, modify some and write the file back again.
 * The returned list is GDKmalloced, the keys are a pointer to a static
 * copy and hence need not to be freed, e.g. GDKfree after freeConfFile
 * is enough.
 */
confkeyval *
getDefaultProps(void)
{
	confkeyval *ret = GDKmalloc(sizeof(_internal_prop_keys));
	memcpy(ret, _internal_prop_keys, sizeof(_internal_prop_keys));
	return(ret);
}

/**
 * Writes the given key-value list to MEROPROPFILE in the given path.
 * FIXME: report back errors (check for them first)
 */
inline void
writeProps(confkeyval *ckv, char *path)
{
	char file[1024];
	FILE *cnf;

	snprintf(file, 1024, "%s/" MEROPROPFILE, path);
	cnf = fopen(file, "w");

	fprintf(cnf, "%s", MEROPROPFILEHEADER);
	while (ckv->key != NULL) {
		if (ckv->val != NULL)
			fprintf(cnf, "%s=%s\n", ckv->key, ckv->val);
		ckv++;
	}

	fflush(cnf);
	fclose(cnf);
}

/**
 * Writes the given key-value list to a buffer and sets its pointer to
 * buf.  This function deals with the allocation of the buffer, hence
 * the caller should free it.
 */
inline void
writePropsBuf(confkeyval *ckv, char **buf)
{
	confkeyval *w;
	size_t len = sizeof(MEROPROPFILEHEADER);
	char *p;

	w = ckv;
	while (w->key != NULL) {
		if (w->val != NULL)
			len += strlen(w->key) + 1 + strlen(w->val) + 1;
		w++;
	}

	p = *buf = malloc(sizeof(char) * len + 1);
	memcpy(p, MEROPROPFILEHEADER, sizeof(MEROPROPFILEHEADER));
	p += sizeof(MEROPROPFILEHEADER) - 1;
	w = ckv;
	while (w->key != NULL) {
		if (w->val != NULL)
			p += sprintf(p, "%s=%s\n", w->key, w->val);
		w++;
	}
}

/**
 * Read a property file, filling in the requested key-values.
 */
inline void
readProps(confkeyval *ckv, char *path)
{
	char file[1024];
	FILE *cnf;

	snprintf(file, 1024, "%s/" MEROPROPFILE, path);
	if ((cnf = fopen(file, "r")) != NULL) {
		readConfFile(ckv, cnf);
		fclose(cnf);
	}
}

/**
 * Read properties from buf, filling in the requested key-values.
 */
inline void
readPropsBuf(confkeyval *ckv, char *buf)
{
	confkeyval *t;
	char *p;
	char *err;
	char *lasts = NULL;
	size_t len;

	while((p = strtok_r(buf, "\n", &lasts)) != NULL) {
		buf = NULL; /* strtok */
		for (t = ckv; t->key != NULL; t++) {
			len = strlen(t->key);
			if (strncmp(p, t->key, len) == 0 && p[len] == '=') {
				if ((err = setConfVal(t, p + len + 1)) != NULL)
					GDKfree(err); /* ignore, just fall back to default */
			}
		}
	}
}

/**
 * Sets a single property, performing all necessary checks to validate
 * the key and associated value.
 */
char *
setProp(char *path, char *key, char *val)
{
	char *err;
	char buf[8096];
	confkeyval *props = getDefaultProps();
	confkeyval *kv;

	readProps(props, path);
	kv = findConfKey(props, key);
	if (kv == NULL) {
		snprintf(buf, sizeof(buf), "no such property: %s", key);
		return(strdup(buf));
	}

	/* first just attempt to set the value (type-check) in memory */
	if ((err = setConfVal(kv, val)) != NULL)
		return(err);

	if (val != NULL) {
		/* handle the semantially enriched types */
		if (strcmp(key, "forward") == 0) {
			if (strcmp(val, "proxy") != 0 && strcmp(val, "redirect") != 0) {
				snprintf(buf, sizeof(buf), "expected 'proxy' or 'redirect' "
						"for property 'forward', got: %s", val);
				return(strdup(buf));
			}
		} else if (strcmp(key, "shared") == 0) {
			char *value = val;
			/* check if tag matches [A-Za-z0-9./]+ */
			if (*value == '\0')
				return(strdup("tag to share cannot be empty"));
			while (*value != '\0') {
				if (!(
							(*value >= 'A' && *value <= 'Z') ||
							(*value >= 'a' && *value <= 'z') ||
							(*value >= '0' && *value <= '9') ||
							(*value == '.' || *value == '/')
					 ))
				{
					snprintf(buf, sizeof(buf),
							"invalid character '%c' at %d "
							"in tag name '%s'\n",
							*value, (int)(value - val), val);
					return(strdup(buf));
				}
				value++;
			}
		}
	}

	/* ok, if we've reached this point we can write this stuff out! */
	writeProps(props, path);
	freeConfFile(props);

	return(NULL);
}

/* vim:set ts=4 sw=4 noexpandtab: */
