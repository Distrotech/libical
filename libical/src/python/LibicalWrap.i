/* -*- Mode: C -*-*/
/*======================================================================
  FILE: ical.i

  (C) COPYRIGHT 1999 Eric Busboom
  http://www.softwarestudio.org

  The contents of this file are subject to the Mozilla Public License
  Version 1.0 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and
  limitations under the License.

  The original author is Eric Busboom

  Contributions from:
  Graham Davison (g.m.davison@computer.org)

  ======================================================================*/  

%module LibicalWrap


%{
#include "libical/ical.h"
#include "libicalss/icalss.h"

#include <sys/types.h> /* for size_t */
#include <time.h>

%}

typedef int time_t;

// This is declared as an extern, but never used in the library.
%ignore icalfileset_safe_saves;

// Ignore these declarations because there does not exist a definition for them
%ignore _icalerror_set_errno(icalerrorenum);
%ignore icalattachtype_add_reference(struct icalattachtype* v);
%ignore icalattachtype_get_binary(struct icalattachtype* v);
%ignore icalattachtype_set_binary(struct icalattachtype* v, char* binary,
				int owns);
%ignore icalattachtype_get_url(struct icalattachtype* v);
%ignore icalattachtype_set_url(struct icalattachtype* v, char* url);
%ignore icalattachtype_free(struct icalattachtype* v);
%ignore icalattachtype_get_base64(struct icalattachtype* v);
%ignore icalattachtype_new(void);
%ignore icalattachtype_set_base64(struct icalattachtype* v, char* base64,
				int owns);
%ignore icalclassify_class_to_string(icalproperty_xlicclass c);
%ignore icalfileset_new_from_cluster(const char* path, icalcluster *cluster);
%ignore icalgauge_as_sql(icalcomponent* gauge);
%ignore icalgauge_new_clone(icalgauge* g, icalcomponent* comp);
%ignore icallangbind_get_component(icalcomponent *c, const char* comp);
%ignore icallangbind_get_parameter(icalproperty *p, const char* parameter);
%ignore icallangbind_get_property(icalcomponent *c, int n, const char* prop);
%ignore icallangbind_get_property_val(icalproperty* p);
%ignore icalmessage_new_cancel_all(icalcomponent* c,
					    const char* user,
					    const char* msg);
%ignore icalmessage_new_cancel_event(icalcomponent* c,
					    const char* user,
					    const char* msg);
%ignore icalmessage_new_cancel_instance(icalcomponent* c,
					    const char* user,
					    const char* msg);
%ignore icalmime_as_mime_string(char* icalcomponent);
%ignore icalparameter_is_valid(icalparameter* parameter);
%ignore icalparser_parse_value(icalvalue_kind kind, 
				   const char* str, icalcomponent** errors);
%ignore icalrecur_iterator_decrement_count(icalrecur_iterator*);
%ignore icalrestriction_is_parameter_allowed(icalproperty_kind property,
                                       icalparameter_kind parameter);
%ignore icalset_clear_select(icalset* set);
%ignore icalspanlist_make_free_list(icalspanlist* sl);
%ignore icalspanlist_make_busy_list(icalspanlist* sl);
%ignore icalspanlist_next_busy_time(icalspanlist* sl,
                                    struct icaltimetype t);
%ignore icaltime_compare_with_zone(const struct icaltimetype a,
        const struct icaltimetype b);
%ignore icaltime_days_in_year (const int year);
%ignore icaltime_from_string_with_zone(const char* str,
					const icaltimezone *zone);
%ignore icaltime_from_week_number(const int week_number,
					const int year);
%ignore icaltime_is_floating(const struct icaltimetype t);
%ignore icaltimezonetype_free(struct icaltimezonetype tzt);

#ifndef _DLOPEN_TEST
%ignore icalset_register_class(icalset *set);
#endif

#include "fcntl.h" /* For Open flags */
%include "libical/ical.h"
%include "libicalss/icalss.h"

// declare some internal functions which are not in the header file.
void icalproperty_set_parent(icalproperty* property,
			     icalcomponent* component);
icalcomponent* icalproperty_get_parent(const icalproperty* property);

void icalvalue_set_parent(icalvalue* value,
			     icalproperty* property);
icalproperty* icalvalue_get_parent(icalvalue* value);

void icalparameter_set_parent(icalparameter* param,
			     icalproperty* property);
icalproperty* icalparameter_get_parent(icalparameter* value);

