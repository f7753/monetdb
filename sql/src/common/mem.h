#ifndef _MEM_H_ 
#define _MEM_H_ 

#include <sql_config.h>

#include <stdio.h>
#include <assert.h>

#ifdef HAVE_MALLOC_H 
#include <malloc.h>
#endif

extern void* 	GDKmalloc  (size_t size);
extern void* 	GDKrealloc (void* pold, size_t size);
extern void	GDKfree    (void* blk); 
extern char*	GDKstrdup  (char *s);

#define NEW( type ) (type*)GDKmalloc(sizeof(type) )
#define NEW_ARRAY( type, size ) (type*)GDKmalloc((size)*sizeof(type))
#define RENEW_ARRAY( type,ptr,size) (type*)GDKrealloc((void*)ptr,(size)*sizeof(type))
#define NEWADT( size ) (adt*)GDKmalloc(size)
#define _DELETE( ptr )	GDKfree(ptr)
#define _strdup( ptr )	GDKstrdup(ptr)

#endif /*_MEM_H_*/
