#ifndef _ATOM_H_
#define _ATOM_H_

#define atom_prefix ""

#include "catalog.h"

typedef	enum atomtype {
	string_value,
	int_value,
	float_value,
	general_value,
} atomtype;

typedef struct atom {
	type *tpe;
	atomtype type;
	union { 
		int ival;
		char *sval;
		double dval;
	} data;
} atom;


extern atom *atom_int( type *tpe, int val );
extern atom *atom_string( char *val );
extern atom *atom_float( double val );
extern atom *atom_general( type *tpe, char *val );

/* duplicate atom */
extern atom *atom_dup( atom *a );

extern char *atom2string(atom *a);
extern const char *atomtype2string(atom *a);

extern void atom_destroy( atom *a );
#endif /* _ATOM_H_ */
