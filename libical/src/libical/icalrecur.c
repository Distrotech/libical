/* -*- Mode: C -*-
  ======================================================================
  FILE: icalrecur.c
  CREATOR: eric 16 May 2000
  
  $Id: icalrecur.c,v 1.1.1.1 2001-01-02 07:33:02 ebusboom Exp $
  $Locker:  $
    

 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/


  How this code works:

  Processing starts when the caller generates a new recurrence
  iterator via icalrecur_iterator_new(). This routine copies the
  recurrence rule into the iterator and extracts things like start and
  end dates. Then, it checks if the rule is legal, using some logic
  from RFC2445 and some logic that probably should be in RFC2445.

  Then, icalrecur_iterator_new() re-writes some of the BY*
  arrays. This involves ( via a call to setup_defaults() ) :

  1) For BY rule parts with no data ( ie BYSECOND was not specified )
  copy the corresponding time part from DTSTART into the BY array. (
  So impl->by_ptrs[BY_SECOND] will then have one element if is
  originally had none ) This only happens if the BY* rule part data
  would expand the number of occurrences in the occurrence set. This
  lets the code ignore DTSTART later on and still use it to get the
  time parts that were not specified in any other way.
  
  2) For the by rule part that are not the same interval as the
  frequency -- for HOURLY anything but BYHOUR, for instance -- copy the
  first data element from the rule part into the first occurrence. For
  example, for "INTERVAL=MONTHLY and BYHOUR=10,30", initialize the
  first time to be returned to have an hour of 10.

  Finally, for INTERVAL=YEARLY, the routine expands the rule to get
  all of the days specified in the rule. The code will do this for
  each new year, and this is the first expansion. This is a special
  case for the yearly interval; no other frequency gets expanded this
  way. The yearly interval is the most complex, so some special
  processing is required.

  After creating a new iterator, the caller will make successive calls
  to icalrecur_iterator_next() to get the next time specified by the
  rule. The main part of this routine is a switch on the frequency of
  the rule. Each different frequency is handled by a different
  routine. 

  For example, next_hour handles the case of INTERVAL=HOURLY, and it
  is called by other routines to get the next hour. First, the routine
  tries to get the next minute part of a time with a call to
  next_minute(). If next_minute() returns 1, it has reached the end of
  its data, usually the last element of the BYMINUTE array. Then, if
  there is data in the BYHOUR array, the routine changes the hour to
  the next one in the array. If INTERVAL=HOURLY, the routine advances
  the hour by the interval.

  If the routine used the last hour in the BYHOUR array, and the
  INTERVAL=HOURLY, then the routine calls increment_monthday() to set
  the next month day. The increment_* routines may call higher routine
  to increment the month or year also.

  The code for INTERVAL=DAILY is handled by next_day(). First, the
  routine tries to get the next hour part of a time with a call to
  next_hour. If next_hour() returns 1, it has reached the end of its
  data, usually the last element of the BYHOUR array. This means that
  next_day() should increment the time to the next day. If FREQUENCY==DAILY,
  the routine increments the day by the interval; otherwise, it
  increments the day by 1.

  Next_day() differs from next_hour because it does not use the BYDAY
  array to select an appropriate day. Instead, it returns every day (
  incrementing by 1 if the frequency is not DAILY with INTERVAL!=1)
  Any days that are not specified in an non-empty BYDAY array are
  filtered out later.

  Generally, the flow of these routine is for a next_* call a next_*
  routine of a lower interval ( next_day calls next_hour) and then to
  possibly call an increment_* routine of an equal or higher
  interval. ( next_day calls increment_monthday() )

  When the call to the original next_* routine returns,
  icalrecur_iterator_next() will check the returned data against other
  BYrule parts to determine if is should be excluded by calling
  check_contracting_rules. Generally, a contracting rule is any with a
  larger time span than the interval. For instance, if
  INTERVAL=DAILY, BYMONTH is a contracting rule part. 

  Check_contracting_rules() uses check_restriction() to do its
  work. Check_restriction() uses expand_map[] to determine if a rule
  is contracting, and if it is, and if the BY rule part has some data,
  then the routine checks if the value of a component of the time is
  part of the byrule part. For instance, for "INTERVAL=DAILY;
  BYMONTH=6,10", check_restriction() would check that the time value
  given to it has a month of either 6 or 10.
  icalrecurrencetype_test()

  Finally, icalrecur_iterator_next() does a few other checks on the
  time value, and if it passes, it returns the time.

  A note about the end_of_data flag. This flag is usually set early in
  a next_* routine and returned in the end. The way it is used allows
  the next_* routine to set the last time back to the first element in
  a BYxx rule, and then signal to the higer level routine to increment
  the next higher level. For instance. WITH FREQ=MONTHLY;BYDAY=TU,FR,
  After next_weekday_by_month runs though both TU and FR, it sets the
  week day back to TU and sets end_of_data. This signals next_month to
  increment the month.


 ======================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "icalrecur.h"

#ifdef ICAL_NO_LIBICAL
#define icalerror_set_errno(x)
#define  icalerror_check_arg_rv(x,y)
#else
#include "icalerror.h"
#include "icalmemory.h"
#endif

#include <stdlib.h> /* for malloc */
#include <errno.h> /* for errno */
#include <string.h> /* for strdup and index */
#include <assert.h>
#include <stddef.h> /* For offsetof() macro */

#define TEMP_MAX 1024


/*********************** Rule parsing routines ************************/

struct icalrecur_parser {
	const char* rule;
        char* copy;
	char* this_clause;
	char* next_clause;

	struct icalrecurrencetype rt;
};

const char* icalrecur_first_clause(struct icalrecur_parser *parser)
{
    char *idx;
    parser->this_clause = parser->copy;
    
    idx = index(parser->this_clause,';');

    if (idx == 0){
	parser->next_clause = 0;
	return 0;
    }

    *idx = 0;
    idx++;
    parser->next_clause = idx;

    return parser->this_clause;

}

const char* icalrecur_next_clause(struct icalrecur_parser *parser)
{
    char* idx;

    parser->this_clause = parser->next_clause;

    if(parser->this_clause == 0){
	return 0;
    }

    idx = index(parser->this_clause,';');

    if (idx == 0){
	parser->next_clause = 0;
    } else {

	*idx = 0;
	idx++;
	parser->next_clause = idx;
    }
	
    return parser->this_clause;

}

void icalrecur_clause_name_and_value(struct icalrecur_parser *parser,
				     char** name, char** value)
{
    char *idx;

    *name = parser->this_clause;

    idx = index(parser->this_clause,'=');

    if (idx == 0){
	*name = 0;
	*value = 0;
	return;
    }
    
    *idx = 0;
    idx++;
    *value = idx;
}

void icalrecur_add_byrules(struct icalrecur_parser *parser, short *array,
			   int size, char* vals)
{
    char *t, *n;
    int i=0;
    int sign = 1;
    short v;

    n = vals;

    while(n != 0){

	if(i == size){
	    return;
	}
	
	t = n;

	n = index(t,',');

	if(n != 0){
	    *n = 0;
	    n++;
	}
	
	/* Get optional sign. HACK. sign is not allowed for all BYxxx
           rule parts */
	if( *t == '-'){
	    sign = 1;
	    t++;
	} else if (*t == '+'){
	    sign = -1;
	    t++;
	}

	v = atoi(t) * sign ;


	array[i++] = v;
	array[i] =  ICAL_RECURRENCE_ARRAY_MAX;

    }

}

void icalrecur_add_bydayrules(struct icalrecur_parser *parser, const char* vals)
{

    char *t, *n;
    int i=0;
    int sign = 1;
    int weekno = 0;
    icalrecurrencetype_weekday wd;
    short *array = parser->rt.by_day;
    char* end;

    end = (char*)vals+strlen(vals);
    n = vals;

    while(n != 0){
	

	t = n;

	n = index(t,',');

	if(n != 0){
	    *n = 0;
	    n++;
	}
	
	/* Get optional sign. */
	if( *t == '-'){
	    sign = -1;
	    t++;
	} else if (*t == '+'){
	    sign = 1;
	    t++;
	}

	/* Get Optional weekno */
	if( sscanf(t,"%d",&weekno) != 0){
	    if (n != 0){
		int weeknolen = (n-t)-3; /* 3 -> one for \0, 2 for day name */
		/* could use abs(log10(weekno))+1, but that needs libm */
		t += weeknolen;
	    } else {
		t = end -2;
	    }
	}

	wd = icalrecur_string_to_weekday(t);

	array[i++] = wd + sign*8*weekno;
	array[i] =  ICAL_RECURRENCE_ARRAY_MAX;

    }

}


struct icalrecurrencetype icalrecurrencetype_from_string(const char* str)
{
    struct icalrecur_parser parser;

    icalerror_check_arg_re(str!=0,"str",parser.rt);

    /* Set up the parser struct */

    memset(&parser,0,sizeof(parser));
    icalrecurrencetype_clear(&parser.rt);

    parser.rule = str;
    parser.copy = strdup(parser.rule);
    parser.this_clause = parser.copy;

    if(parser.copy == 0){
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return parser.rt;
    }

    /* Loop through all of the clauses */
    for(icalrecur_first_clause(&parser); 
	parser.this_clause != 0;
	icalrecur_next_clause(&parser))
    {
	char *name, *value;
	icalrecur_clause_name_and_value(&parser,&name,&value);

	if (strcmp(name,"FREQ") == 0){
	    parser.rt.freq = icalrecur_string_to_recurrence(value);
	} else if (strcmp(name,"COUNT") == 0){
	    parser.rt.count = atoi(value);
	} else if (strcmp(name,"UNTIL") == 0){
	    parser.rt.until = icaltime_from_string(value);
	} else if (strcmp(name,"INTERVAL") == 0){
	    parser.rt.interval = atoi(value);
	} else if (strcmp(name,"WKST") == 0){
	    parser.rt.week_start = icalrecur_string_to_weekday(value);
	} else if (strcmp(name,"BYSECOND") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_second,
				  ICAL_BY_SECOND_SIZE,value);
	} else if (strcmp(name,"BYMINUTE") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_minute,
				  ICAL_BY_MINUTE_SIZE,value);
	} else if (strcmp(name,"BYHOUR") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_hour,
				  ICAL_BY_HOUR_SIZE,value);
	} else if (strcmp(name,"BYDAY") == 0){
	    icalrecur_add_bydayrules(&parser,value);
	} else if (strcmp(name,"BYMONTHDAY") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_month_day,
				  ICAL_BY_MONTHDAY_SIZE,value);
	} else if (strcmp(name,"BYYEARDAY") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_year_day,
				  ICAL_BY_YEARDAY_SIZE,value);
	} else if (strcmp(name,"BYWEEKNO") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_week_no,
				  ICAL_BY_WEEKNO_SIZE,value);
	} else if (strcmp(name,"BYMONTH") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_month,
				  ICAL_BY_MONTH_SIZE,value);
	} else if (strcmp(name,"BYSETPOS") == 0){
	    icalrecur_add_byrules(&parser,parser.rt.by_set_pos,
				  ICAL_BY_SETPOS_SIZE,value);
	} else {
	    /* error */
	}
	
    }

    free(parser.copy);

    return parser.rt;

}

#ifndef ICAL_NO_LIBICAL

struct { char* str;size_t offset; short limit;  } recurmap[] = 
{
    {";BYSECOND=",offsetof(struct icalrecurrencetype,by_second),60},
    {";BYMINUTE=",offsetof(struct icalrecurrencetype,by_minute),60},
    {";BYHOUR=",offsetof(struct icalrecurrencetype,by_hour),24},
    {";BYDAY=",offsetof(struct icalrecurrencetype,by_day),7},
    {";BYMONTHDAY=",offsetof(struct icalrecurrencetype,by_month_day),31},
    {";BYYEARDAY=",offsetof(struct icalrecurrencetype,by_year_day),366},
    {";BYWEEKNO=",offsetof(struct icalrecurrencetype,by_week_no),52},
    {";BYMONTH=",offsetof(struct icalrecurrencetype,by_month),12},
    {";BYSETPOS=",offsetof(struct icalrecurrencetype,by_set_pos),366},
    {0,0,0},
};

/* A private routine in icalvalue.c */
void print_datetime_to_string(char* str,  struct icaltimetype *data);

char* icalrecurrencetype_as_string(struct icalrecurrencetype *recur)
{
    char* str;
    char *str_p;
    size_t buf_sz = 200;
    char temp[20];
    int i,j;

    if(recur->freq == ICAL_NO_RECURRENCE){
	return 0;
    }

    str = (char*)icalmemory_tmp_buffer(buf_sz);
    str_p = str;

    icalmemory_append_string(&str,&str_p,&buf_sz,"FREQ=");
    icalmemory_append_string(&str,&str_p,&buf_sz,
			     icalrecur_recurrence_to_string(recur->freq));

    if(recur->until.year != 0){
	
	temp[0] = 0;
	print_datetime_to_string(temp,&(recur->until));
	
	icalmemory_append_string(&str,&str_p,&buf_sz,";UNTIL=");
	icalmemory_append_string(&str,&str_p,&buf_sz, temp);
    }

    if(recur->count != 0){
	sprintf(temp,"%d",recur->count);
	icalmemory_append_string(&str,&str_p,&buf_sz,";COUNT=");
	icalmemory_append_string(&str,&str_p,&buf_sz, temp);
    }

    if(recur->interval != 0){
	sprintf(temp,"%d",recur->interval);
	icalmemory_append_string(&str,&str_p,&buf_sz,";INTERVAL=");
	icalmemory_append_string(&str,&str_p,&buf_sz, temp);
    }
    
    for(j =0; recurmap[j].str != 0; j++){
	short* array = (short*)(recurmap[j].offset+ (size_t)recur);
	short limit = recurmap[j].limit;

	/* Skip unused arrays */
	if( array[0] != ICAL_RECURRENCE_ARRAY_MAX ) {

	    icalmemory_append_string(&str,&str_p,&buf_sz,recurmap[j].str);
	    
	    for(i=0; 
		i< limit  && array[i] != ICAL_RECURRENCE_ARRAY_MAX;
		i++){
		if (j == 3) { /* BYDAY */
		    short dow = icalrecurrencetype_day_day_of_week(array[i]);
		    const char *daystr = icalrecur_weekday_to_string(dow);
		    short pos;

		    pos = icalrecurrencetype_day_position(array[i]);  
		    
		    if (pos == 1)
			icalmemory_append_string(&str,&str_p,&buf_sz,daystr);
		    else {
			sprintf(temp,"%d%s",pos,daystr);
			icalmemory_append_string(&str,&str_p,&buf_sz,temp);
		    }                  
		    
		} else {
		    sprintf(temp,"%d",array[i]);
		    icalmemory_append_string(&str,&str_p,&buf_sz, temp);
		}
		
		if( (i+1)<limit &&array[i+1] 
		    != ICAL_RECURRENCE_ARRAY_MAX){
		    icalmemory_append_char(&str,&str_p,&buf_sz,',');
		}
	    }	 
	}   
    }

    return  str;
}
#endif



/************************* occurrence iteration routiens ******************/

enum byrule {
    NO_CONTRACTION = -1,
    BY_SECOND = 0,
    BY_MINUTE = 1,
    BY_HOUR = 2,
    BY_DAY = 3,
    BY_MONTH_DAY = 4,
    BY_YEAR_DAY = 5,
    BY_WEEK_NO = 6,
    BY_MONTH = 7,
    BY_SET_POS
};



struct icalrecur_iterator_impl {
	
	struct icaltimetype dtstart; /* Hack. Make into time_t */
	struct icaltimetype last; /* last time return from _iterator_next*/
	int occurrence_no; /* number of step made on t iterator */
	struct icalrecurrencetype rule;

	short days[366];
	short days_index;

	enum byrule byrule;
	short by_indices[9];
	short orig_data[9]; /* 1 if there was data in the byrule */


	short *by_ptrs[9]; /* Pointers into the by_* array elements of the rule */
};

int icalrecur_iterator_sizeof_byarray(short* byarray)
{
    int array_itr;

    for(array_itr = 0; 
	byarray[array_itr] != ICAL_RECURRENCE_ARRAY_MAX;
	array_itr++){
    }

    return array_itr;
}

enum expand_table {
    UNKNOWN  = 0,
    CONTRACT = 1,
    EXPAND =2,
    ILLEGAL=3
};

/* The split map indicates, for a particular interval, wether a BY_*
   rule part expands the number of instances in the occcurrence set or
   contracts it. 1=> contract, 2=>expand, and 3 means the pairing is
   not allowed. */
struct expand_split_map_struct 
{ 
	icalrecurrencetype_frequency frequency;

	/* Elements of the 'map' array correspond to the BYxxx rules:
           Second,Minute,Hour,Day,Month Day,Year Day,Week No,Month*/

	short map[8];
}; 

struct expand_split_map_struct expand_map[] =
{
    {ICAL_SECONDLY_RECURRENCE,{1,1,1,1,1,1,1,1}},
    {ICAL_MINUTELY_RECURRENCE,{2,1,1,1,1,1,1,1}},
    {ICAL_HOURLY_RECURRENCE,  {2,2,1,1,1,1,1,1}},
    {ICAL_DAILY_RECURRENCE,   {2,2,2,1,1,1,1,1}},
    {ICAL_WEEKLY_RECURRENCE,  {2,2,2,2,3,3,1,1}},
    {ICAL_MONTHLY_RECURRENCE, {2,2,2,2,2,3,3,1}},
    {ICAL_YEARLY_RECURRENCE,  {2,2,2,2,2,2,2,2}},
    {ICAL_NO_RECURRENCE,      {0,0,0,0,0,0,0,0}}

};



/* Check that the rule has only the two given interday byrule parts. */
int icalrecur_two_byrule(struct icalrecur_iterator_impl* impl,
			 enum byrule one,enum byrule two)
{
    short test_array[9];
    enum byrule itr;
    int passes = 0;

    memset(test_array,0,9);

    test_array[one] = 1;
    test_array[two] = 1;

    for(itr = BY_DAY; itr != BY_SET_POS; itr++){

	if( (test_array[itr] == 0  &&
	     impl->by_ptrs[itr][0] != ICAL_RECURRENCE_ARRAY_MAX
	    ) ||
	    (test_array[itr] == 1  &&
	     impl->by_ptrs[itr][0] == ICAL_RECURRENCE_ARRAY_MAX
		) 
	    ) {
	    /* test failed */
	    passes = 0;
	}
    }

    return passes;

} 

/* Check that the rule has only the one given interdat byrule parts. */
int icalrecur_one_byrule(struct icalrecur_iterator_impl* impl,enum byrule one)
{
    int passes = 1;
    enum byrule itr;

    for(itr = BY_DAY; itr != BY_SET_POS; itr++){
	
	if ((itr==one && impl->by_ptrs[itr][0] == ICAL_RECURRENCE_ARRAY_MAX) ||
	    (itr!=one && impl->by_ptrs[itr][0] != ICAL_RECURRENCE_ARRAY_MAX)) {
	    passes = 0;
	}
    }

    return passes;
} 

int count_byrules(struct icalrecur_iterator_impl* impl)
{
    int count = 0;
    enum byrule itr;

    for(itr = BY_DAY; itr <= BY_SET_POS; itr++){
	if(impl->by_ptrs[itr][0] != ICAL_RECURRENCE_ARRAY_MAX){
	    count++;
	}
    }

    return count;
}


void setup_defaults(struct icalrecur_iterator_impl* impl, 
		    enum byrule byrule, icalrecurrencetype_frequency req,
		    short deftime, int *timepart)
{

    icalrecurrencetype_frequency freq;
    freq = impl->rule.freq;

    /* Re-write the BY rule arrays with data from the DTSTART time so
       we don't have to explicitly deal with DTSTART */

    if(impl->by_ptrs[byrule][0] == ICAL_RECURRENCE_ARRAY_MAX &&
	expand_map[freq].map[byrule] != CONTRACT){
	impl->by_ptrs[byrule][0] = deftime;
    }

    /* Initialize the first occurence */
    if( freq != req && expand_map[freq].map[byrule] != CONTRACT){
	*timepart = impl->by_ptrs[byrule][0];
    }


}

int expand_year_days(struct icalrecur_iterator_impl* impl,short year);

int has_by_data(struct icalrecur_iterator_impl* impl, enum byrule byrule){

    return (impl->orig_data[byrule] == 1);
}


icalrecur_iterator* icalrecur_iterator_new(struct icalrecurrencetype rule, 
					   struct icaltimetype dtstart)
{
    struct icalrecur_iterator_impl* impl;
    icalrecurrencetype_frequency freq;

    if ( ( impl = (struct icalrecur_iterator_impl *)
	   malloc(sizeof(struct icalrecur_iterator_impl))) == 0) {
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return 0;
    }

    memset(impl,0,sizeof(struct icalrecur_iterator_impl));

    impl->rule = rule;
    impl->last = dtstart;
    impl->dtstart = dtstart;
    impl->days_index =0;
    impl->occurrence_no = 0;
    freq = impl->rule.freq;

    /* Set up convienience pointers to make the code simpler. Allows
       us to iterate through all of the BY* arrays in the rule. */

    impl->by_ptrs[BY_MONTH]=impl->rule.by_month;
    impl->by_ptrs[BY_WEEK_NO]=impl->rule.by_week_no;
    impl->by_ptrs[BY_YEAR_DAY]=impl->rule.by_year_day;
    impl->by_ptrs[BY_MONTH_DAY]=impl->rule.by_month_day;
    impl->by_ptrs[BY_DAY]=impl->rule.by_day;
    impl->by_ptrs[BY_HOUR]=impl->rule.by_hour;
    impl->by_ptrs[BY_MINUTE]=impl->rule.by_minute;
    impl->by_ptrs[BY_SECOND]=impl->rule.by_second;
    impl->by_ptrs[BY_SET_POS]=impl->rule.by_set_pos;

    memset(impl->orig_data,0,9);

    impl->orig_data[BY_MONTH]
	= (impl->rule.by_month[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_WEEK_NO]
      =(impl->rule.by_week_no[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_YEAR_DAY]
    =(impl->rule.by_year_day[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_MONTH_DAY]
    =(impl->rule.by_month_day[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_DAY]
	= (impl->rule.by_day[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_HOUR]
	= (impl->rule.by_hour[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_MINUTE]
     = (impl->rule.by_minute[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_SECOND]
     = (impl->rule.by_second[0]!=ICAL_RECURRENCE_ARRAY_MAX);
    impl->orig_data[BY_SET_POS]
     = (impl->rule.by_set_pos[0]!=ICAL_RECURRENCE_ARRAY_MAX);


    /* Check if the recurrence rule is legal */

    /* If the BYYEARDAY appears, no other date rule part may appear.   */

    if(icalrecur_two_byrule(impl,BY_YEAR_DAY,BY_MONTH) ||
       icalrecur_two_byrule(impl,BY_YEAR_DAY,BY_WEEK_NO) ||
       icalrecur_two_byrule(impl,BY_YEAR_DAY,BY_MONTH_DAY) ||
       icalrecur_two_byrule(impl,BY_YEAR_DAY,BY_DAY) ){

	icalerror_set_errno(ICAL_USAGE_ERROR);

	return 0;
    }

    /* BYWEEKNO and BYMONTH rule parts may not both appear.*/

    if(icalrecur_two_byrule(impl,BY_WEEK_NO,BY_MONTH)){
	icalerror_set_errno(ICAL_USAGE_ERROR);

	icalerror_set_errno(ICAL_USAGE_ERROR);
	return 0;
    }

    /* BYWEEKNO and BYMONTHDAY rule parts may not both appear.*/

    if(icalrecur_two_byrule(impl,BY_WEEK_NO,BY_MONTH_DAY)){
	icalerror_set_errno(ICAL_USAGE_ERROR);

	icalerror_set_errno(ICAL_USAGE_ERROR);
	return 0;
    }


    /*For MONTHLY recurrences (FREQ=MONTHLY) neither BYYEARDAY nor
      BYWEEKNO may appear. */

    if(freq == ICAL_MONTHLY_RECURRENCE && 
       ( icalrecur_one_byrule(impl,BY_WEEK_NO) ||
	 icalrecur_one_byrule(impl,BY_YEAR_DAY)) ) {

	icalerror_set_errno(ICAL_USAGE_ERROR);
	return 0;
    }


    /*For WEEKLY recurrences (FREQ=WEEKLY) neither BYMONTHDAY nor
      BYYEARDAY may appear. */

    if(freq == ICAL_WEEKLY_RECURRENCE && 
       ( icalrecur_one_byrule(impl,BY_MONTH_DAY) ||
	 icalrecur_one_byrule(impl,BY_YEAR_DAY)) ) {
	
	icalerror_set_errno(ICAL_USAGE_ERROR);
	return 0;
    }


    /* Rewrite some of the rules and set up defaults to make later
       processing easier. Primarily, t involves copying an element
       from the start time into the coresponding BY_* array when the
       BY_* array is empty */


    setup_defaults(impl,BY_SECOND,ICAL_SECONDLY_RECURRENCE,impl->dtstart.second,
		   &(impl->last.second));

    setup_defaults(impl,BY_MINUTE,ICAL_MINUTELY_RECURRENCE,impl->dtstart.minute,
		   &(impl->last.minute));

    setup_defaults(impl,BY_HOUR,ICAL_HOURLY_RECURRENCE,impl->dtstart.hour,
		   &(impl->last.hour));

    setup_defaults(impl,BY_MONTH_DAY,ICAL_DAILY_RECURRENCE,impl->dtstart.day,
		   &(impl->last.day));

    setup_defaults(impl,BY_MONTH,ICAL_MONTHLY_RECURRENCE,impl->dtstart.month,
		   &(impl->last.month));

    if(impl->rule.freq == ICAL_WEEKLY_RECURRENCE ){

       if(impl->by_ptrs[BY_DAY][0] == ICAL_RECURRENCE_ARRAY_MAX){

	   /* Weekly recurrences with no BY_DAY data should occur on the
	      same day of the week as the start time . */
	   impl->by_ptrs[BY_DAY][0] = icaltime_day_of_week(impl->dtstart);

       } else {
	  /* If there is BY_DAY data, then we need to move the initial
	     time to the start of the BY_DAY data. That if if the
	     start time is on a Wednesday, and the rule has
	     BYDAY=MO,WE,FR, move the initial time back to
	     monday. Otherwise, jumping to the next week ( jumping 7
	     days ahead ) will skip over some occurrences in the
	     second week. */
	  
	  /* This is probably a HACK. There should be some more
             general way to solve this problem */

	  short dow = impl->by_ptrs[BY_DAY][0]-icaltime_day_of_week(impl->last);

	  if(dow < 0) {
	      /* initial time is after first day of BY_DAY data */

	      impl->last.day += dow;
	      impl->last = icaltime_normalize(impl->last);
	  }
      }
      

    }


    if(impl->rule.freq == ICAL_YEARLY_RECURRENCE){
	expand_year_days(impl,impl->dtstart.year);
    }


    /* If this is a monthly interval with by day data, then we need to
       set the last value to the appropriate day of the month */

    if(impl->rule.freq == ICAL_MONTHLY_RECURRENCE &&
       has_by_data(impl,BY_DAY)) {

	short dow = icalrecurrencetype_day_day_of_week(
	    impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]]);  
	short pos =  icalrecurrencetype_day_position(
	    impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]]);  
	
	short poscount = 0;
	short days_in_month = 
	    icaltime_days_in_month(impl->last.month, impl->last.year)  ; 
	
	for(impl->last.day = 1;
	    impl->last.day <= days_in_month;
	    impl->last.day++){
	    
	    if(icaltime_day_of_week(impl->last) == dow){
		if(++poscount == pos){
		    break;
		}
	    }
	}

	if(impl->last.day > days_in_month){
	    icalerror_set_errno(ICAL_USAGE_ERROR);
	    return 0;
	}
	
    }


    return impl;
}


void icalrecur_iterator_free(icalrecur_iterator* i)
{
    
    struct icalrecur_iterator_impl* impl = 
	(struct icalrecur_iterator_impl*)i;

    icalerror_check_arg_rv((impl!=0),"impl");

    free(impl);

}




void increment_year(struct icalrecur_iterator_impl* impl, int inc)
{
    impl->last.year+=inc;
}




void increment_month(struct icalrecur_iterator_impl* impl, int inc)
{
    int years;

    impl->last.month+=inc;

    /* Months are offset by one */
    impl->last.month--;

    years = impl->last.month / 12;

    impl->last.month = impl->last.month % 12;

    impl->last.month++;

    if (years != 0){
	increment_year(impl,years);
    }
}

void increment_monthday(struct icalrecur_iterator_impl* impl, int inc)
{
    int i;

    for(i=0; i<inc; i++){
	
	short days_in_month = 
	    icaltime_days_in_month(impl->last.month,impl->last.year);

	impl->last.day++;
	
	if (impl->last.day > days_in_month){
	    impl->last.day = impl->last.day-days_in_month;
	    increment_month(impl,1);
	}
    }
}


void increment_hour(struct icalrecur_iterator_impl* impl, int inc)
{
    short days;

    impl->last.hour+=inc;

    days = impl->last.hour / 24;
    impl->last.hour = impl->last.hour % 24;

    if (impl->days != 0){
	increment_monthday(impl,days);
    }
}

void increment_minute(struct icalrecur_iterator_impl* impl, int inc)
{
    short hours;

    impl->last.minute+=inc;

    hours = impl->last.minute / 60;
     impl->last.minute =  impl->last.minute % 60;

     if (hours != 0){
	increment_hour(impl,hours);
    }

}

void increment_second(struct icalrecur_iterator_impl* impl, int inc)
{
    short minutes;

    impl->last.second+=inc;
    
    minutes = impl->last.second / 60;
    impl->last.second = impl->last.second % 60;
    
    if (minutes != 0)
    {
	increment_minute(impl, minutes);
    }                 
}

#if 0
#include "ical.h"
void test_increment()
{
    struct icalrecur_iterator_impl impl;

    impl.last =  icaltime_from_string("20000101T000000Z");

    printf("Orig: %s\n",icaltime_as_ctime(impl.last));
    
    increment_second(&impl,5);
    printf("+ 5 sec    : %s\n",icaltime_as_ctime(impl.last));

    increment_second(&impl,355);
    printf("+ 355 sec  : %s\n",icaltime_as_ctime(impl.last));

    increment_minute(&impl,5);
    printf("+ 5 min    : %s\n",icaltime_as_ctime(impl.last));

    increment_minute(&impl,360);
    printf("+ 360 min  : %s\n",icaltime_as_ctime(impl.last));
    increment_hour(&impl,5);
    printf("+ 5 hours  : %s\n",icaltime_as_ctime(impl.last));
    increment_hour(&impl,43);
    printf("+ 43 hours : %s\n",icaltime_as_ctime(impl.last));
    increment_monthday(&impl,3);
    printf("+ 3 days   : %s\n",icaltime_as_ctime(impl.last));
    increment_monthday(&impl,600);
    printf("+ 600 days  : %s\n",icaltime_as_ctime(impl.last));
	
}

#endif 

short next_second(struct icalrecur_iterator_impl* impl)
{

  short has_by_data = (impl->by_ptrs[BY_SECOND][0]!=ICAL_RECURRENCE_ARRAY_MAX);
  short this_frequency = (impl->rule.freq == ICAL_SECONDLY_RECURRENCE);

  short end_of_data = 0;

  assert(has_by_data || this_frequency);

  if(  has_by_data ){
      /* Ignore the frequency and use the byrule data */

      impl->by_indices[BY_SECOND]++;

      if (impl->by_ptrs[BY_SECOND][impl->by_indices[BY_SECOND]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){
	  impl->by_indices[BY_SECOND] = 0;

	  end_of_data = 1;
      }


      impl->last.second = 
	  impl->by_ptrs[BY_SECOND][impl->by_indices[BY_SECOND]];
      
      
  } else if( !has_by_data &&  this_frequency ){
      /* Compute the next value from the last time and the frequency interval*/
      increment_second(impl, impl->rule.interval);

  }

  /* If we have gone through all of the seconds on the BY list, then we
     need to move to the next minute */

  if(has_by_data && end_of_data && this_frequency ){
      increment_minute(impl,1);
  }

  return end_of_data;

}

int next_minute(struct icalrecur_iterator_impl* impl)
{

  short has_by_data = (impl->by_ptrs[BY_MINUTE][0]!=ICAL_RECURRENCE_ARRAY_MAX);
  short this_frequency = (impl->rule.freq == ICAL_MINUTELY_RECURRENCE);

  short end_of_data = 0;

  assert(has_by_data || this_frequency);


  if (next_second(impl) == 0){
      return 0;
  }

  if(  has_by_data ){
      /* Ignore the frequency and use the byrule data */

      impl->by_indices[BY_MINUTE]++;
      
      if (impl->by_ptrs[BY_MINUTE][impl->by_indices[BY_MINUTE]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){

	  impl->by_indices[BY_MINUTE] = 0;
	  
	  end_of_data = 1;
      }

      impl->last.minute = 
	  impl->by_ptrs[BY_MINUTE][impl->by_indices[BY_MINUTE]];

  } else if( !has_by_data &&  this_frequency ){
      /* Compute the next value from the last time and the frequency interval*/
      increment_minute(impl,impl->rule.interval);
  } 

/* If we have gone through all of the minutes on the BY list, then we
     need to move to the next hour */

  if(has_by_data && end_of_data && this_frequency ){
      increment_hour(impl,1);
  }

  return end_of_data;
}

int next_hour(struct icalrecur_iterator_impl* impl)
{

  short has_by_data = (impl->by_ptrs[BY_HOUR][0]!=ICAL_RECURRENCE_ARRAY_MAX);
  short this_frequency = (impl->rule.freq == ICAL_HOURLY_RECURRENCE);

  short end_of_data = 0;

  assert(has_by_data || this_frequency);

  if (next_minute(impl) == 0){
      return 0;
  }

  if(  has_by_data ){
      /* Ignore the frequency and use the byrule data */

      impl->by_indices[BY_HOUR]++;
      
      if (impl->by_ptrs[BY_HOUR][impl->by_indices[BY_HOUR]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){
	  impl->by_indices[BY_HOUR] = 0;
	  
	  end_of_data = 1;
      }

      impl->last.hour = 
	  impl->by_ptrs[BY_HOUR][impl->by_indices[BY_HOUR]];

  } else if( !has_by_data &&  this_frequency ){
      /* Compute the next value from the last time and the frequency interval*/
      increment_hour(impl,impl->rule.interval);

  }

  /* If we have gone through all of the hours on the BY list, then we
     need to move to the next day */

  if(has_by_data && end_of_data && this_frequency ){
      increment_monthday(impl,1);
  }

  return end_of_data;

}

int next_day(struct icalrecur_iterator_impl* impl)
{

  short has_by_data = (impl->by_ptrs[BY_DAY][0]!=ICAL_RECURRENCE_ARRAY_MAX);
  short this_frequency = (impl->rule.freq == ICAL_DAILY_RECURRENCE);

  assert(has_by_data || this_frequency);

  if (next_hour(impl) == 0){
      return 0;
  }

  /* Always increment through the interval, since this routine is not
     called by any other next_* routine, and the days that are
     excluded will be taken care of by restriction filtering */

  if(this_frequency){
      increment_monthday(impl,impl->rule.interval);
  } else {
      increment_monthday(impl,1);
  }


  return 0;

}

/* This routine is only called by next_month and next_year, so it does
   not have a clause for this_frequency */
int next_monthday(struct icalrecur_iterator_impl* impl)
{

  short has_by_data = (impl->by_ptrs[BY_MONTH_DAY][0]!=ICAL_RECURRENCE_ARRAY_MAX);
  short mday;
  short end_of_data = 0;

  assert(has_by_data );

  if (next_hour(impl) == 0){
      return 0;
  }

  impl->by_indices[BY_MONTH_DAY]++;
  
  mday = impl->by_ptrs[BY_MONTH_DAY][impl->by_indices[BY_MONTH_DAY]];

  if ( mday ==ICAL_RECURRENCE_ARRAY_MAX){
      impl->by_indices[BY_MONTH_DAY] = 0;
      
      end_of_data = 1;
  }

  if (mday > 0){
      impl->last.day = mday;
  } else {
      short days_in_month = icaltime_days_in_month(impl->last.month,
						   impl->last.year);
      impl->last.day = days_in_month-mday+1;
  }

  if(has_by_data && end_of_data ){
      increment_month(impl,1);
  }

  return end_of_data;

}

int next_yearday(struct icalrecur_iterator_impl* impl)
{

  short has_by_data = (impl->by_ptrs[BY_YEAR_DAY][0]!=ICAL_RECURRENCE_ARRAY_MAX);

  short end_of_data = 0;

  assert(has_by_data );

  if (next_hour(impl) == 0){
      return 0;
  }

  impl->by_indices[BY_YEAR_DAY]++;
  
  if (impl->by_ptrs[BY_YEAR_DAY][impl->by_indices[BY_YEAR_DAY]]
      ==ICAL_RECURRENCE_ARRAY_MAX){
      impl->by_indices[BY_YEAR_DAY] = 0;
      
      end_of_data = 1;
  }
  
  impl->last.day = 
      impl->by_ptrs[BY_YEAR_DAY][impl->by_indices[BY_YEAR_DAY]];
  
  if(has_by_data && end_of_data){
      increment_year(impl,1);
  }

  return end_of_data;

}

/* This routine is only called by next_week. It is certain that BY_DAY
has data */

int next_weekday_by_week(struct icalrecur_iterator_impl* impl)
{

  short end_of_data = 0;
  short start_of_week, dow;
  struct icaltimetype next;

  if (next_hour(impl) == 0){
      return 0;
  }

  assert( impl->by_ptrs[BY_DAY][0]!=ICAL_RECURRENCE_ARRAY_MAX);

  while(1) {

      impl->by_indices[BY_DAY]++; /* Look at next elem in BYDAY array */
      
      /* Are we at the end of the BYDAY array? */
      if (impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){
	  
	  impl->by_indices[BY_DAY] = 0; /* Reset to 0 */      
	  end_of_data = 1; /* Signal that we're at the end */
      }
      
      /* Add the day of week offset to to the start of this week, and use
	 that to get the next day */
      dow = impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]];  
      start_of_week = icaltime_start_doy_of_week(impl->last);
      
      dow--; /*Sun is 1, not 0 */

      if(dow+start_of_week <1 && !end_of_data){
	  /* The selected date is in the previous year. */
	  continue;
      }

      next = icaltime_from_day_of_year(start_of_week + dow,impl->last.year);

      impl->last.day =  next.day;
      impl->last.month =  next.month;
      impl->last.year =  next.year;
  
      return end_of_data;
  }

}

int next_weekday_by_month(struct icalrecur_iterator_impl* impl)
{

  short end_of_data = 0;
  struct icaltimetype start_of_month; /* Start of month */
  short pos, poscount, dow, days_in_month;

  if (next_hour(impl) == 0){
      return 0;
  }

  assert( impl->by_ptrs[BY_DAY][0]!=ICAL_RECURRENCE_ARRAY_MAX);

  while(1) {
      impl->by_indices[BY_DAY]++; /* Look at next elem in BYDAY array */
      
      /* Are we at the end of the BYDAY array? */
      if (impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){
	  
	  impl->by_indices[BY_DAY] = 0; /* Reset to 0 */      
	  end_of_data = 1; /* Signal that we're at the end */
      }

      dow = icalrecurrencetype_day_day_of_week(
	  impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]]);  
      pos =  icalrecurrencetype_day_position(
	  impl->by_ptrs[BY_DAY][impl->by_indices[BY_DAY]]);  

      start_of_month = impl->last;

      /* Find right day in month. HACK. Find an arithmetic way to do
         this */

      poscount = 0;
      days_in_month = 
	  icaltime_days_in_month(impl->last.month, impl->last.year)  ; 

      for(start_of_month.day = 1;
	  start_of_month.day <= days_in_month;
	  start_of_month.day++){

	  if(icaltime_day_of_week(start_of_month) == dow){
	      if(++poscount == pos){
		  break;
	      }
	  }
      }

      if (!end_of_data == 1 && 
	  (
	      start_of_month.day > days_in_month ||
	      icaltime_compare(start_of_month,impl->last) <= 0
	      )
	  ){
	  continue;
      }

      impl->last.day =  start_of_month.day;
      impl->last.month =  start_of_month.month;
      impl->last.year =  start_of_month.year;
  
      return end_of_data;
  }
}

int next_month(struct icalrecur_iterator_impl* impl)
{

  short this_frequency = (impl->rule.freq == ICAL_MONTHLY_RECURRENCE);

  short end_of_data = 0;

  assert( has_by_data(impl,BY_MONTH) || this_frequency);

  /* Week day data overrides monthday data */
  if(has_by_data(impl,BY_DAY)){
      /* For this case, the weekdays are relative to the
         month. BYDAY=FR -> First Friday in month, etc. */
      if (next_weekday_by_month(impl) == 0){
	  return 0;
      }
  } else {
      if (next_monthday(impl) == 0){
	  return 0;
      }
  }


  if(has_by_data(impl,BY_MONTH) ){
      /* Ignore the frequency and use the byrule data */

      impl->by_indices[BY_MONTH]++;
      
      if (impl->by_ptrs[BY_MONTH][impl->by_indices[BY_MONTH]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){
	  impl->by_indices[BY_MONTH] = 0;
	  
	  end_of_data = 1;
      }

      impl->last.month = 
	  impl->by_ptrs[BY_MONTH][impl->by_indices[BY_MONTH]];

  } else if( !has_by_data(impl,BY_MONTH) &&  this_frequency ){

      if(has_by_data(impl,BY_DAY)){

	  short dayinc = 28;

	  /* BY_DAY data specified a day of week, but incrementing the
             month changes the day of the week -- Nov 2 is not the
             same DOW as Oct 2. So, we need to fix the day of week by
             incrementing in even weeks into the next month. . */

	
  if ( impl->last.day + dayinc 
	       <= icaltime_days_in_month(impl->last.month, impl->last.year)){
	      dayinc += 7;
	  }

	  increment_monthday(impl,dayinc);

      } else {

	  /* Compute the next value from the last time and the
	     frequency interval*/
	  increment_month(impl,impl->rule.interval);
      }
	  
  }
  
  
  if(has_by_data(impl,BY_MONTH) && end_of_data && this_frequency ){
      increment_year(impl,1);
  }
  return end_of_data;

}


int next_week(struct icalrecur_iterator_impl* impl)
{
  short has_by_data = (impl->by_ptrs[BY_WEEK_NO][0]!=ICAL_RECURRENCE_ARRAY_MAX);
  short this_frequency = (impl->rule.freq == ICAL_WEEKLY_RECURRENCE);
  short end_of_data = 0;


  if (next_weekday_by_week(impl) == 0){
      return 0;
  }

  if( impl->by_ptrs[BY_WEEK_NO][0]!=ICAL_RECURRENCE_ARRAY_MAX){
    /* Use the Week Number byrule data */
      int week_no;
      struct icaltimetype t;
      
      impl->by_indices[BY_WEEK_NO]++;
      
      if (impl->by_ptrs[BY_WEEK_NO][impl->by_indices[BY_WEEK_NO]]
	  ==ICAL_RECURRENCE_ARRAY_MAX){
	  impl->by_indices[BY_WEEK_NO] = 0;
	  
	  end_of_data = 1;
      }
      
      t = impl->last;
      t.month=1; /* HACK, should be setting to the date of the first week of year*/
      t.day=1;
      
      week_no = impl->by_ptrs[BY_WEEK_NO][impl->by_indices[BY_WEEK_NO]];
      
      impl->last.day += week_no*7;

      impl->last = icaltime_normalize(impl->last);
      
  } else if( !has_by_data &&  this_frequency ){


      increment_monthday(impl,7*impl->rule.interval);
  }

  if(has_by_data && end_of_data && this_frequency ){
      increment_year(impl,1);
  }

  return end_of_data;
  
}



/* For INTERVAL=YEARLY, set up the days[] array in the iterator to
   list all of the days of the current year that are specified in this
   rule. */

int expand_year_days(struct icalrecur_iterator_impl* impl,short year)
{
    int j,k;
    int days_index=0;
    struct icaltimetype t;


    t.is_date = 1; /* Needed to make day_of_year routines work property */

    memset(&t,0,sizeof(t));
    memset(impl->days,ICAL_RECURRENCE_ARRAY_MAX_BYTE,sizeof(impl->days));
    
    if(has_by_data(impl,BY_MONTH) && !has_by_data(impl,BY_MONTH_DAY)
	&& !has_by_data(impl,BY_DAY)){
	
        for(j=0;impl->by_ptrs[BY_MONTH][j]!=ICAL_RECURRENCE_ARRAY_MAX;j++){
	    struct icaltimetype t;
	    short month = impl->by_ptrs[BY_MONTH][j];	    
            short doy;

	    t = impl->dtstart;
	    t.year = year;
	    t.month = month;
	    t.is_date = 1;

	    doy = icaltime_day_of_year(t);
	    
            impl->days[days_index++] = doy;

        }


    }
    else if ( has_by_data(impl,BY_MONTH) && has_by_data(impl,BY_DAY)){

        for(j=0;impl->by_ptrs[BY_MONTH][j]!=ICAL_RECURRENCE_ARRAY_MAX;j++){
	    short month = impl->by_ptrs[BY_MONTH][j];
	    short days_in_month = icaltime_days_in_month(month,year);
		
	    struct icaltimetype t;
	    memset(&t,0,sizeof(struct icaltimetype));
	    t.day = 1;
	    t.year = year;
	    t.month = month;
	    t.is_date = 1;
	    
	    for(t.day = 1; t.day <=days_in_month; t.day++){
		
		short current_dow = icaltime_day_of_week(t);
		
		for(k=0;impl->by_ptrs[BY_DAY][k]!=ICAL_RECURRENCE_ARRAY_MAX;k++){
		    
		    enum icalrecurrencetype_weekday dow =
			icalrecurrencetype_day_day_of_week(impl->by_ptrs[BY_DAY][k]);
		    
		    if(current_dow == dow){
			short doy = icaltime_day_of_year(t);
			/* HACK, incomplete Nth day of week handling */
			impl->days[days_index++] = doy;
			
		    }
		}
            }
        }
    } else if (has_by_data(impl,BY_MONTH) && has_by_data(impl,BY_MONTH_DAY)){

        for(j=0;impl->by_ptrs[BY_MONTH][j]!=ICAL_RECURRENCE_ARRAY_MAX;j++){
            for(k=0;impl->by_ptrs[BY_MONTH_DAY][k]!=ICAL_RECURRENCE_ARRAY_MAX;k++)
           {
                short month = impl->by_ptrs[BY_MONTH][j];
                short month_day = impl->by_ptrs[BY_MONTH_DAY][k];
                short doy;

		t.day = month_day;
		t.month = month;
		t.year = year;
		t.is_date = 1;

		doy = icaltime_day_of_year(t);

		impl->days[days_index++] = doy;

            }
        }
    } else if (has_by_data(impl,BY_WEEK_NO) && !has_by_data(impl,BY_DAY)){

	struct icaltimetype t;
	short dow;

	t.day = impl->dtstart.day;
	t.month = impl->dtstart.month;
	t.year = year;
	t.is_date = 1;

        dow = icaltime_day_of_week(t);
	/* HACK Not finished */ 
	

    } else if (has_by_data(impl,BY_WEEK_NO) && has_by_data(impl,BY_DAY)){
	/* HACK Not finished */ 
    } else if (has_by_data(impl,BY_YEAR_DAY)){
	
	for(j=0;impl->by_ptrs[BY_YEAR_DAY][j]!=ICAL_RECURRENCE_ARRAY_MAX;j++){
	    short doy = impl->by_ptrs[BY_YEAR_DAY][j];
	    impl->days[days_index++] = doy;
	}
	    
    } else if (has_by_data(impl,BY_MONTH_DAY) ){
	/* HACK Not finished */ 

    } else if (has_by_data(impl,BY_DAY)){
	/* HACK Not finished */ 

    } else {
	assert(0);
	/* HACK Not finished */ 

    }

    return 0;
}                                  


int next_year(struct icalrecur_iterator_impl* impl)
{
    struct icaltimetype next;
    short end_of_data=0;

    if (next_hour(impl) == 0){
	return 0;
    }

    impl->days_index++;

    if (impl->days[impl->days_index] == ICAL_RECURRENCE_ARRAY_MAX){
	impl->days_index = 0;
	end_of_data = 1;
    }

    next = icaltime_from_day_of_year(impl->days[impl->days_index],impl->last.year);
    
    impl->last.day =  next.day;
    impl->last.month =  next.month;
  

    if(end_of_data){
	increment_year(impl,impl->rule.interval);
	expand_year_days(impl,impl->last.year);
    }
    
    return 1;
}

int check_restriction(struct icalrecur_iterator_impl* impl,
		      enum byrule byrule, short v)
{
    int pass = 0;
    int itr;
    icalrecurrencetype_frequency freq = impl->rule.freq;

    if(impl->by_ptrs[byrule][0]!=ICAL_RECURRENCE_ARRAY_MAX &&
	expand_map[freq].map[byrule] == CONTRACT){
	for(itr=0; impl->by_ptrs[byrule][itr]!=ICAL_RECURRENCE_ARRAY_MAX;itr++){
	    if(impl->by_ptrs[byrule][itr] == v){
		pass=1;
		break;
	    }
	}

	return pass;
    } else {
	/* This is not a contracting byrule, or it has no data, so the
           test passes*/
	return 1;
    }
}

int check_contracting_rules(struct icalrecur_iterator_impl* impl)
{
    enum byrule;

    int day_of_week=0;
    int week_no=0;
    int year_day=0;

    if (
	check_restriction(impl,BY_SECOND,impl->last.second) &&
	check_restriction(impl,BY_MINUTE,impl->last.minute) &&
	check_restriction(impl,BY_HOUR,impl->last.hour) &&
	check_restriction(impl,BY_DAY,day_of_week) &&
	check_restriction(impl,BY_WEEK_NO,week_no) &&
	check_restriction(impl,BY_MONTH_DAY,impl->last.day) &&
	check_restriction(impl,BY_MONTH,impl->last.month) &&
	check_restriction(impl,BY_YEAR_DAY,year_day) )
    {

	return 1;
    } else {
	return 0;
    }
}

struct icaltimetype icalrecur_iterator_next(icalrecur_iterator *itr)
{
    struct icalrecur_iterator_impl* impl = 
	(struct icalrecur_iterator_impl*)itr;
    
    if( (impl->rule.count!=0 &&impl->occurrence_no >= impl->rule.count) ||
       (!icaltime_is_null_time(impl->rule.until) && 
	icaltime_compare(impl->last,impl->rule.until) > 0)) {
	return icaltime_null_time();
    }

    if(impl->occurrence_no == 0 
       &&  icaltime_compare(impl->last,impl->dtstart) >= 0){

	impl->occurrence_no++;
	return impl->last;
    }

    do {
	switch(impl->rule.freq){
	    
	    case ICAL_SECONDLY_RECURRENCE: {
		next_second(impl);
		break;
	    }
	    case ICAL_MINUTELY_RECURRENCE: {
		next_minute(impl);
		break;
	    }
	    case ICAL_HOURLY_RECURRENCE: {
		next_hour(impl);
		break;
	    }
	    case ICAL_DAILY_RECURRENCE: {
		next_day(impl);
		break;
	    }
	    case ICAL_WEEKLY_RECURRENCE: {
		next_week(impl);
		break;
	    }
	    case ICAL_MONTHLY_RECURRENCE: {
		next_month(impl);
		break;
	    }
	    case ICAL_YEARLY_RECURRENCE:{
		next_year(impl);
		break;
	    }
	    default:{
		assert(0); /* HACK, need a better error */
	    }
	}    
	
	if(impl->last.year >= 2038){
	    /* HACK */
	    return icaltime_null_time();
	}
	
    } while(!check_contracting_rules(impl) 
	    || icaltime_compare(impl->last,impl->dtstart) < 0);
    
    
/* Ignore null times and times that are after the until time */
    if( !icaltime_is_null_time(impl->rule.until) && 
	icaltime_compare(impl->last,impl->rule.until) > 0 ) {
	return icaltime_null_time();
    }

    impl->occurrence_no++;

    return impl->last;
}


/************************** Type Routines **********************/


void icalrecurrencetype_clear(struct icalrecurrencetype *recur)
{
    memset(recur,ICAL_RECURRENCE_ARRAY_MAX_BYTE,
	   sizeof(struct icalrecurrencetype));

    recur->week_start = ICAL_MONDAY_WEEKDAY;
    recur->freq = ICAL_NO_RECURRENCE;
    recur->interval = 1;
    memset(&(recur->until),0,sizeof(struct icaltimetype));
    recur->count = 0;
}

/* The 'day' element of icalrecurrencetype_weekday is encoded to allow
reporesentation of both the day of the week ( Monday, Tueday), but
also the Nth day of the week ( First tuesday of the month, last
thursday of the year) These routines decode the day values. 

The day's position in the period ( Nth-ness) and the numerical value
of the day are encoded together as: pos*7 + dow
 */

enum icalrecurrencetype_weekday icalrecurrencetype_day_day_of_week(short day)
{
    return abs(day)%8;
}

short icalrecurrencetype_day_position(short day)
{
    short pos = (day-icalrecurrencetype_day_day_of_week(day))/8;

    if(pos == 0){
	pos = 1;
    }

    return pos;
}


/****************** Enumeration Routines ******************/

struct {icalrecurrencetype_weekday wd; const char * str; } 
wd_map[] = {
    {ICAL_SUNDAY_WEEKDAY,"SU"},
    {ICAL_MONDAY_WEEKDAY,"MO"},
    {ICAL_TUESDAY_WEEKDAY,"TU"},
    {ICAL_WEDNESDAY_WEEKDAY,"WE"},
    {ICAL_THURSDAY_WEEKDAY,"TH"},
    {ICAL_FRIDAY_WEEKDAY,"FR"},
    {ICAL_SATURDAY_WEEKDAY,"SA"},
    {ICAL_NO_WEEKDAY,0}
};

const char* icalrecur_weekday_to_string(icalrecurrencetype_weekday kind)
{
    int i;

    for (i=0; wd_map[i].wd  != ICAL_NO_WEEKDAY; i++) {
	if ( wd_map[i].wd ==  kind) {
	    return wd_map[i].str;
	}
    }

    return 0;
}

icalrecurrencetype_weekday icalrecur_string_to_weekday(const char* str)
{
    int i;

    for (i=0; wd_map[i].wd  != ICAL_NO_WEEKDAY; i++) {
	if ( strcmp(str,wd_map[i].str) == 0){
	    return wd_map[i].wd;
	}
    }

    return ICAL_NO_WEEKDAY;
}



struct {
	icalrecurrencetype_frequency kind;
	const char* str;
} freq_map[] = {
    {ICAL_SECONDLY_RECURRENCE,"SECONDLY"},
    {ICAL_MINUTELY_RECURRENCE,"MINUTELY"},
    {ICAL_HOURLY_RECURRENCE,"HOURLY"},
    {ICAL_DAILY_RECURRENCE,"DAILY"},
    {ICAL_WEEKLY_RECURRENCE,"WEEKLY"},
    {ICAL_MONTHLY_RECURRENCE,"MONTHLY"},
    {ICAL_YEARLY_RECURRENCE,"YEARLY"},
    {ICAL_NO_RECURRENCE,0}
};

const char* icalrecur_recurrence_to_string(icalrecurrencetype_frequency kind)
{
    int i;

    for (i=0; freq_map[i].kind != ICAL_NO_RECURRENCE ; i++) {
	if ( freq_map[i].kind == kind ) {
	    return freq_map[i].str;
	}
    }
    return 0;
}

icalrecurrencetype_frequency icalrecur_string_to_recurrence(const char* str)
{
    int i;

    for (i=0; freq_map[i].kind != ICAL_NO_RECURRENCE ; i++) {
	if ( strcmp(str,freq_map[i].str) == 0){
	    return freq_map[i].kind;
	}
    }
    return ICAL_NO_RECURRENCE;
}


int icalrecur_expand_recurrence(char* rule, time_t start,
				int count, time_t* array)
{
    struct icalrecurrencetype recur;
    icalrecur_iterator* ritr;
    time_t tt;
    struct icaltimetype icstart, next;
    int i = 0;

    memset(array, 0, count*sizeof(time_t));

    icstart = icaltime_from_timet(start,0);

    recur = icalrecurrencetype_from_string(rule);

    for(ritr = icalrecur_iterator_new(recur,icstart),
	next = icalrecur_iterator_next(ritr);
	!icaltime_is_null_time(next) && i < count;
	next = icalrecur_iterator_next(ritr)){
	
	tt = icaltime_as_timet(next);
	
	if (tt >= start ){
	    array[i++] = tt;
	}

    }

    icalrecur_iterator_free(ritr);

    return 1;
}
