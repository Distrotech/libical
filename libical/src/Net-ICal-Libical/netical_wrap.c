/*
 * FILE : netical_wrap.c
 * 
 * This file was automatically generated by :
 * Simplified Wrapper and Interface Generator (SWIG)
 * Version 1.1 (Patch 5)
 * 
 * Portions Copyright (c) 1995-1998
 * The University of Utah and The Regents of the University of California.
 * Permission is granted to distribute this file in any manner provided
 * this notice remains intact.
 * 
 * Do not make changes to this file--changes will be lost!
 *
 */


#define SWIGCODE
/* Implementation : PERL 5 */

#define SWIGPERL
#define SWIGPERL5
#ifdef __cplusplus
#include <math.h>
#include <stdlib.h>
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#undef free
#undef malloc
#include <string.h>
#ifdef __cplusplus
}
#endif
/* Definitions for compiling Perl extensions on a variety of machines */

#if defined(WIN32) || defined(__WIN32__)
#   if defined(_MSC_VER)
#	define SWIGEXPORT(a,b) __declspec(dllexport) a b
#   else
#	if defined(__BORLANDC__)
#	    define SWIGEXPORT(a,b) a _export b
#	else
#	    define SWIGEXPORT(a,b) a b
#	endif
#   endif
#else
#   define SWIGEXPORT(a,b) a b
#endif

#ifdef PERL_OBJECT
#define MAGIC_PPERL  CPerl *pPerl = (CPerl *) this;
#define MAGIC_CAST   (int (CPerl::*)(SV *, MAGIC *))
#define SWIGCLASS_STATIC 
#else
#define MAGIC_PPERL
#define MAGIC_CAST
#define SWIGCLASS_STATIC static
#endif


/*****************************************************************************
 * $Header: /tmp/freeassociation-cvsbackup/libical/src/Net-ICal-Libical/netical_wrap.c,v 1.1 2001-01-28 16:37:44 ebusboom Exp $
 *
 * perl5ptr.swg
 *
 * This file contains supporting code for the SWIG run-time type checking
 * mechanism.  The following functions are available :
 *
 * SWIG_RegisterMapping(char *origtype, char *newtype, void *(*cast)(void *));
 *
 *      Registers a new type-mapping with the type-checker.  origtype is the
 *      original datatype and newtype is an equivalent type.  cast is optional
 *      pointer to a function to cast pointer values between types (this
 *      is only used to cast pointers from derived classes to base classes in C++)
 *      
 * SWIG_MakePtr(char *buffer, void *ptr, char *typestring);
 *     
 *      Makes a pointer string from a pointer and typestring.  The result is returned
 *      in buffer.
 *
 * char * SWIG_GetPtr(SV *obj, void **ptr, char *type)
 *
 *      Gets a pointer value from a Perl5 scalar value.  If there is a 
 *      type-mismatch, returns a character string to the received type.  
 *      On success, returns NULL.
 *
 *
 * You can remap these functions by making a file called "swigptr.swg" in
 * your the same directory as the interface file you are wrapping.
 *
 * These functions are normally declared static, but this file can be
 * can be used in a multi-module environment by redefining the symbol
 * SWIGSTATIC.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1996/12/26 22:17:29  beazley
 * Initial revision
 *
 *****************************************************************************/

#include <stdlib.h>

#ifdef SWIG_GLOBAL
#ifdef __cplusplus
#define SWIGSTATIC extern "C"
#else
#define SWIGSTATIC
#endif
#endif

#ifndef SWIGSTATIC
#define SWIGSTATIC static
#endif

/* These are internal variables.   Should be static */

typedef struct SwigPtrType {
  char               *name;
  int                 len;
  void               *(*cast)(void *);
  struct SwigPtrType *next;
} SwigPtrType;

/* Pointer cache structure */

typedef struct {
  int                 stat;               /* Status (valid) bit             */
  SwigPtrType        *tp;                 /* Pointer to type structure      */
  char                name[256];          /* Given datatype name            */
  char                mapped[256];        /* Equivalent name                */
} SwigCacheType;

static int SwigPtrMax  = 64;           /* Max entries that can be currently held */
static int SwigPtrN    = 0;            /* Current number of entries              */
static int SwigPtrSort = 0;            /* Status flag indicating sort            */
static SwigPtrType *SwigPtrTable = 0;  /* Table containing pointer equivalences  */
static int SwigStart[256];             /* Table containing starting positions    */

/* Cached values */

#define SWIG_CACHESIZE  8
#define SWIG_CACHEMASK  0x7
static SwigCacheType SwigCache[SWIG_CACHESIZE];  
static int SwigCacheIndex = 0;
static int SwigLastCache = 0;

/* Sort comparison function */
static int swigsort(const void *data1, const void *data2) {
	SwigPtrType *d1 = (SwigPtrType *) data1;
	SwigPtrType *d2 = (SwigPtrType *) data2;
	return strcmp(d1->name,d2->name);
}

/* Binary Search function */
static int swigcmp(const void *key, const void *data) {
  char *k = (char *) key;
  SwigPtrType *d = (SwigPtrType *) data;
  return strncmp(k,d->name,d->len);
}

/* Register a new datatype with the type-checker */

#ifndef PERL_OBJECT
SWIGSTATIC 
void SWIG_RegisterMapping(char *origtype, char *newtype, void *(*cast)(void *)) {
#else
SWIGSTATIC
#define SWIG_RegisterMapping(a,b,c) _SWIG_RegisterMapping(pPerl, a,b,c)
void _SWIG_RegisterMapping(CPerl *pPerl, char *origtype, char *newtype, void *(*cast)(void *)) {
#endif

  int i;
  SwigPtrType *t = 0, *t1;

  if (!SwigPtrTable) {     
    SwigPtrTable = (SwigPtrType *) malloc(SwigPtrMax*sizeof(SwigPtrType));
    SwigPtrN = 0;
  }
  if (SwigPtrN >= SwigPtrMax) {
    SwigPtrMax = 2*SwigPtrMax;
    SwigPtrTable = (SwigPtrType *) realloc(SwigPtrTable,SwigPtrMax*sizeof(SwigPtrType));
  }
  for (i = 0; i < SwigPtrN; i++)
    if (strcmp(SwigPtrTable[i].name,origtype) == 0) {
      t = &SwigPtrTable[i];
      break;
    }
  if (!t) {
    t = &SwigPtrTable[SwigPtrN];
    t->name = origtype;
    t->len = strlen(t->name);
    t->cast = 0;
    t->next = 0;
    SwigPtrN++;
  }
  while (t->next) {
    if (strcmp(t->name,newtype) == 0) {
      if (cast) t->cast = cast;
      return;
    }
    t = t->next;
  }
  t1 = (SwigPtrType *) malloc(sizeof(SwigPtrType));
  t1->name = newtype;
  t1->len = strlen(t1->name);
  t1->cast = cast;
  t1->next = 0;
  t->next = t1;
  SwigPtrSort = 0;
}

/* Make a pointer value string */

SWIGSTATIC 
void SWIG_MakePtr(char *_c, const void *_ptr, char *type) {
  static char _hex[16] =
  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'a', 'b', 'c', 'd', 'e', 'f'};
  unsigned long _p, _s;
  char _result[20], *_r;    /* Note : a 64-bit hex number = 16 digits */
  _r = _result;
  _p = (unsigned long) _ptr;
  if (_p > 0) {
    while (_p > 0) {
      _s = _p & 0xf;
      *(_r++) = _hex[_s];
      _p = _p >> 4;
    }
    *_r = '_';
    while (_r >= _result)
      *(_c++) = *(_r--);
  } else {
    strcpy (_c, "NULL");
  }
  if (_ptr)
    strcpy (_c, type);
}

/* Define for backwards compatibility */

#define _swig_make_hex   SWIG_MakePtr 

/* Function for getting a pointer value */

#ifndef PERL_OBJECT
SWIGSTATIC 
char *SWIG_GetPtr(SV *sv, void **ptr, char *_t)
#else
SWIGSTATIC
#define SWIG_GetPtr(a,b,c) _SWIG_GetPtr(pPerl,a,b,c)
char *_SWIG_GetPtr(CPerl *pPerl, SV *sv, void **ptr, char *_t)
#endif
{
  char temp_type[256];
  char *name,*_c;
  int  len,i,start,end;
  IV   tmp;
  SwigPtrType *sp,*tp;
  SwigCacheType *cache;

  /* If magical, apply more magic */

  if (SvGMAGICAL(sv))
    mg_get(sv);

  /* Check to see if this is an object */
  if (sv_isobject(sv)) {
    SV *tsv = (SV*) SvRV(sv);
    if ((SvTYPE(tsv) == SVt_PVHV)) {
      MAGIC *mg;
      if (SvMAGICAL(tsv)) {
	mg = mg_find(tsv,'P');
	if (mg) {
	  SV *rsv = mg->mg_obj;
	  if (sv_isobject(rsv)) {
	    tmp = SvIV((SV*)SvRV(rsv));
	  }
	}
      } else {
	return "Not a valid pointer value";
      }
    } else {
      tmp = SvIV((SV*)SvRV(sv));
    }
    if (!_t) {
      *(ptr) = (void *) tmp;
      return (char *) 0;
    }
  } else if (sv == &sv_undef) {            /* Check for undef */
    *(ptr) = (void *) 0;
    return (char *) 0;
  } else if (SvTYPE(sv) == SVt_RV) {       /* Check for NULL pointer */
    *(ptr) = (void *) 0;
    if (!SvROK(sv)) 
      return (char *) 0;
    else
      return "Not a valid pointer value";
  } else {                                 /* Don't know what it is */
      *(ptr) = (void *) 0;
      return "Not a valid pointer value";
  }
  if (_t) {
    /* Now see if the types match */      

    if (!sv_isa(sv,_t)) {
      _c = HvNAME(SvSTASH(SvRV(sv)));
      if (!SwigPtrSort) {
	qsort((void *) SwigPtrTable, SwigPtrN, sizeof(SwigPtrType), swigsort);  
	for (i = 0; i < 256; i++) {
	  SwigStart[i] = SwigPtrN;
	}
	for (i = SwigPtrN-1; i >= 0; i--) {
	  SwigStart[SwigPtrTable[i].name[0]] = i;
	}
	for (i = 255; i >= 1; i--) {
	  if (SwigStart[i-1] > SwigStart[i])
	    SwigStart[i-1] = SwigStart[i];
	}
	SwigPtrSort = 1;
	for (i = 0; i < SWIG_CACHESIZE; i++)  
	  SwigCache[i].stat = 0;
      }
      /* First check cache for matches.  Uses last cache value as starting point */
      cache = &SwigCache[SwigLastCache];
      for (i = 0; i < SWIG_CACHESIZE; i++) {
	if (cache->stat) {
	  if (strcmp(_t,cache->name) == 0) {
	    if (strcmp(_c,cache->mapped) == 0) {
	      cache->stat++;
	      *ptr = (void *) tmp;
	      if (cache->tp->cast) *ptr = (*(cache->tp->cast))(*ptr);
	      return (char *) 0;
	    }
	  }
	}
	SwigLastCache = (SwigLastCache+1) & SWIG_CACHEMASK;
	if (!SwigLastCache) cache = SwigCache;
	else cache++;
      }

      start = SwigStart[_t[0]];
      end = SwigStart[_t[0]+1];
      sp = &SwigPtrTable[start];
      while (start < end) {
	if (swigcmp(_t,sp) == 0) break;
	sp++;
	start++;
      }
      if (start >= end) sp = 0;
      if (sp) {
	while (swigcmp(_t,sp) == 0) {
	  name = sp->name;
	  len = sp->len;
	  tp = sp->next;
	  while(tp) {
	    if (tp->len >= 255) {
	      return _c;
	    }
	    strcpy(temp_type,tp->name);
	    strncat(temp_type,_t+len,255-tp->len);
	    if (sv_isa(sv,temp_type)) {
	      /* Get pointer value */
	      *ptr = (void *) tmp;
	      if (tp->cast) *ptr = (*(tp->cast))(*ptr);

	      strcpy(SwigCache[SwigCacheIndex].mapped,_c);
	      strcpy(SwigCache[SwigCacheIndex].name,_t);
	      SwigCache[SwigCacheIndex].stat = 1;
	      SwigCache[SwigCacheIndex].tp = tp;
	      SwigCacheIndex = SwigCacheIndex & SWIG_CACHEMASK;
	      return (char *) 0;
	    }
	    tp = tp->next;
	  } 
	  /* Hmmm. Didn't find it this time */
 	  sp++;
	}
      }
      /* Didn't find any sort of match for this data.  
	 Get the pointer value and return the received type */
      *ptr = (void *) tmp;
      return _c;
    } else {
      /* Found a match on the first try.  Return pointer value */
      *ptr = (void *) tmp;
      return (char *) 0;
    }
  } 
  *ptr = (void *) tmp;
  return (char *) 0;
}

/* Compatibility mode */

#define _swig_get_hex  SWIG_GetPtr
/* Magic variable code */
#ifndef PERL_OBJECT
#define swig_create_magic(s,a,b,c) _swig_create_magic(s,a,b,c)
static void _swig_create_magic(SV *sv, char *name, int (*set)(SV *, MAGIC *), int (*get)(SV *,MAGIC *)) {
#else
#define swig_create_magic(s,a,b,c) _swig_create_magic(pPerl,s,a,b,c)
static void _swig_create_magic(CPerl *pPerl, SV *sv, char *name, int (CPerl::*set)(SV *, MAGIC *), int (CPerl::*get)(SV *, MAGIC *)) {
#endif
  MAGIC *mg;
  sv_magic(sv,sv,'U',name,strlen(name));
  mg = mg_find(sv,'U');
  mg->mg_virtual = (MGVTBL *) malloc(sizeof(MGVTBL));
  mg->mg_virtual->svt_get = get;
  mg->mg_virtual->svt_set = set;
  mg->mg_virtual->svt_len = 0;
  mg->mg_virtual->svt_clear = 0;
  mg->mg_virtual->svt_free = 0;
}

#define SWIG_init    boot_Net__ICal__Libical

#define SWIG_name   "Net::ICal::Libical::boot_Net__ICal__Libical"
#define SWIG_varinit "Net::ICal::Libical::var_Net__ICal__Libical_init();"
#ifdef __cplusplus
extern "C"
#endif
#ifndef PERL_OBJECT
SWIGEXPORT(void,boot_Net__ICal__Libical)(CV* cv);
#else
SWIGEXPORT(void,boot_Net__ICal__Libical)(CPerl *, CV *cv);
#endif

#include "ical.h"

#include <sys/types.h> /* for size_t */
#include <time.h>



    int* new_array(int size){
	int* p = malloc(size*sizeof(int));
	return p; /* Caller handles failures */
    }

    void free_array(int* array){
	free(array);
    }

    int access_array(int* array, int index) {
	return array[index];
    }
#ifdef PERL_OBJECT
#define MAGIC_CLASS _wrap_Net__ICal__Libical_var::
class _wrap_Net__ICal__Libical_var : public CPerl {
public:
#else
#define MAGIC_CLASS
#endif
SWIGCLASS_STATIC int swig_magic_readonly(SV *sv, MAGIC *mg) {
    MAGIC_PPERL
    sv = sv; mg = mg;
    croak("Value is read-only.");
    return 0;
}


#ifdef PERL_OBJECT
};
#endif

XS(_wrap_icalparser_parse_string) {

    icalcomponent * _result;
    char * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalparser_parse_string(str);");
    _arg0 = (char *) SvPV(ST(0),na);
    _result = (icalcomponent *)icalparser_parse_string(_arg0);
    ST(argvi) = sv_newmortal();
    sv_setref_pv(ST(argvi++),"icalcomponentPtr", (void *) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalcomponent_as_ical_string) {

    char * _result;
    icalcomponent * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalcomponent_as_ical_string(component);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalcomponent_as_ical_string. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    _result = (char *)icalcomponent_as_ical_string(_arg0);
    ST(argvi) = sv_newmortal();
    sv_setpv((SV*)ST(argvi++),(char *) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalcomponent_free) {

    icalcomponent * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalcomponent_free(component);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalcomponent_free. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    icalcomponent_free(_arg0);
    XSRETURN(argvi);
}

XS(_wrap_icalcomponent_count_errors) {

    int  _result;
    icalcomponent * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalcomponent_count_errors(component);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalcomponent_count_errors. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    _result = (int )icalcomponent_count_errors(_arg0);
    ST(argvi) = sv_newmortal();
    sv_setiv(ST(argvi++),(IV) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalcomponent_strip_errors) {

    icalcomponent * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalcomponent_strip_errors(component);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalcomponent_strip_errors. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    icalcomponent_strip_errors(_arg0);
    XSRETURN(argvi);
}

XS(_wrap_icalcomponent_convert_errors) {

    icalcomponent * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalcomponent_convert_errors(component);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalcomponent_convert_errors. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    icalcomponent_convert_errors(_arg0);
    XSRETURN(argvi);
}

XS(_wrap_icalrestriction_check) {

    int  _result;
    icalcomponent * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalrestriction_check(comp);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalrestriction_check. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    _result = (int )icalrestriction_check(_arg0);
    ST(argvi) = sv_newmortal();
    sv_setiv(ST(argvi++),(IV) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalrecur_expand_recurrence) {

    int  _result;
    char * _arg0;
    int  _arg1;
    int  _arg2;
    int * _arg3;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 4) || (items > 4)) 
        croak("Usage: icalrecur_expand_recurrence(rule,start,count,array);");
    _arg0 = (char *) SvPV(ST(0),na);
    _arg1 = (int )SvIV(ST(1));
    _arg2 = (int )SvIV(ST(2));
    if (SWIG_GetPtr(ST(3),(void **) &_arg3,"intPtr")) {
        croak("Type error in argument 4 of icalrecur_expand_recurrence. Expected intPtr.");
        XSRETURN(1);
    }
    _result = (int )icalrecur_expand_recurrence(_arg0,_arg1,_arg2,_arg3);
    ST(argvi) = sv_newmortal();
    sv_setiv(ST(argvi++),(IV) _result);
    XSRETURN(argvi);
}

XS(_wrap_new_array) {

    int * _result;
    int  _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: new_array(size);");
    _arg0 = (int )SvIV(ST(0));
    _result = (int *)new_array(_arg0);
    ST(argvi) = sv_newmortal();
    sv_setref_pv(ST(argvi++),"intPtr", (void *) _result);
    XSRETURN(argvi);
}

XS(_wrap_free_array) {

    int * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: free_array(array);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,"intPtr")) {
        croak("Type error in argument 1 of free_array. Expected intPtr.");
        XSRETURN(1);
    }
    free_array(_arg0);
    XSRETURN(argvi);
}

XS(_wrap_access_array) {

    int  _result;
    int * _arg0;
    int  _arg1;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 2) || (items > 2)) 
        croak("Usage: access_array(array,index);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,"intPtr")) {
        croak("Type error in argument 1 of access_array. Expected intPtr.");
        XSRETURN(1);
    }
    _arg1 = (int )SvIV(ST(1));
    _result = (int )access_array(_arg0,_arg1);
    ST(argvi) = sv_newmortal();
    sv_setiv(ST(argvi++),(IV) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalperl_get_property) {

    icalproperty * _result;
    icalcomponent * _arg0;
    int  _arg1;
    char * _arg2;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 3) || (items > 3)) 
        croak("Usage: icalperl_get_property(c,n,prop);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalperl_get_property. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    _arg1 = (int )SvIV(ST(1));
    _arg2 = (char *) SvPV(ST(2),na);
    _result = (icalproperty *)icalperl_get_property(_arg0,_arg1,_arg2);
    ST(argvi) = sv_newmortal();
    sv_setref_pv(ST(argvi++),"icalpropertyPtr", (void *) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalperl_get_property_val) {

    char * _result;
    icalproperty * _arg0;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 1) || (items > 1)) 
        croak("Usage: icalperl_get_property_val(p);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalperl_get_property_val. Expected icalpropertyPtr.");
        XSRETURN(1);
    }
    _result = (char *)icalperl_get_property_val(_arg0);
    ST(argvi) = sv_newmortal();
    sv_setpv((SV*)ST(argvi++),(char *) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalperl_get_parameter) {

    char * _result;
    icalproperty * _arg0;
    char * _arg1;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 2) || (items > 2)) 
        croak("Usage: icalperl_get_parameter(p,parameter);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalperl_get_parameter. Expected icalpropertyPtr.");
        XSRETURN(1);
    }
    _arg1 = (char *) SvPV(ST(1),na);
    _result = (char *)icalperl_get_parameter(_arg0,_arg1);
    ST(argvi) = sv_newmortal();
    sv_setpv((SV*)ST(argvi++),(char *) _result);
    XSRETURN(argvi);
}

XS(_wrap_icalperl_get_component) {

    icalcomponent * _result;
    icalcomponent * _arg0;
    char * _arg1;
    int argvi = 0;
    dXSARGS ;

    cv = cv;
    if ((items < 2) || (items > 2)) 
        croak("Usage: icalperl_get_component(c,comp);");
    if (SWIG_GetPtr(ST(0),(void **) &_arg0,(char *) 0 )) {
        croak("Type error in argument 1 of icalperl_get_component. Expected icalcomponentPtr.");
        XSRETURN(1);
    }
    _arg1 = (char *) SvPV(ST(1),na);
    _result = (icalcomponent *)icalperl_get_component(_arg0,_arg1);
    ST(argvi) = sv_newmortal();
    sv_setref_pv(ST(argvi++),"icalcomponentPtr", (void *) _result);
    XSRETURN(argvi);
}

XS(_wrap_perl5_Net__ICal__Libical_var_init) {
    dXSARGS;
    SV *sv;
    cv = cv; items = items;
    XSRETURN(1);
}
#ifdef __cplusplus
extern "C"
#endif
XS(boot_Net__ICal__Libical) {
	 dXSARGS;
	 char *file = __FILE__;
	 cv = cv; items = items;
	 newXS("Net::ICal::Libical::var_Net__ICal__Libical_init", _wrap_perl5_Net__ICal__Libical_var_init, file);
	 newXS("Net::ICal::Libical::icalparser_parse_string", _wrap_icalparser_parse_string, file);
	 newXS("Net::ICal::Libical::icalcomponent_as_ical_string", _wrap_icalcomponent_as_ical_string, file);
	 newXS("Net::ICal::Libical::icalcomponent_free", _wrap_icalcomponent_free, file);
	 newXS("Net::ICal::Libical::icalcomponent_count_errors", _wrap_icalcomponent_count_errors, file);
	 newXS("Net::ICal::Libical::icalcomponent_strip_errors", _wrap_icalcomponent_strip_errors, file);
	 newXS("Net::ICal::Libical::icalcomponent_convert_errors", _wrap_icalcomponent_convert_errors, file);
	 newXS("Net::ICal::Libical::icalrestriction_check", _wrap_icalrestriction_check, file);
	 newXS("Net::ICal::Libical::icalrecur_expand_recurrence", _wrap_icalrecur_expand_recurrence, file);
	 newXS("Net::ICal::Libical::new_array", _wrap_new_array, file);
	 newXS("Net::ICal::Libical::free_array", _wrap_free_array, file);
	 newXS("Net::ICal::Libical::access_array", _wrap_access_array, file);
	 newXS("Net::ICal::Libical::icalperl_get_property", _wrap_icalperl_get_property, file);
	 newXS("Net::ICal::Libical::icalperl_get_property_val", _wrap_icalperl_get_property_val, file);
	 newXS("Net::ICal::Libical::icalperl_get_parameter", _wrap_icalperl_get_parameter, file);
	 newXS("Net::ICal::Libical::icalperl_get_component", _wrap_icalperl_get_component, file);
/*
 * These are the pointer type-equivalency mappings. 
 * (Used by the SWIG pointer type-checker).
 */
	 SWIG_RegisterMapping("unsigned short","short",0);
	 SWIG_RegisterMapping("long","unsigned long",0);
	 SWIG_RegisterMapping("long","signed long",0);
	 SWIG_RegisterMapping("signed short","short",0);
	 SWIG_RegisterMapping("signed int","int",0);
	 SWIG_RegisterMapping("short","unsigned short",0);
	 SWIG_RegisterMapping("short","signed short",0);
	 SWIG_RegisterMapping("unsigned long","long",0);
	 SWIG_RegisterMapping("int","unsigned int",0);
	 SWIG_RegisterMapping("int","signed int",0);
	 SWIG_RegisterMapping("unsigned int","int",0);
	 SWIG_RegisterMapping("signed long","long",0);
	 ST(0) = &sv_yes;
	 XSRETURN(1);
}
