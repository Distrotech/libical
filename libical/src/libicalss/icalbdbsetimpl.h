/* -*- Mode: C -*-
  ======================================================================
  FILE: icalbdbsetimpl.h
  CREATOR: dml 12 December 2001
  (C) COPYRIGHT 2001, Critical Path

  $Id: icalbdbsetimpl.h,v 1.2 2002-06-27 02:30:58 acampi Exp $
  $Locker:  $
 ======================================================================*/

#ifndef ICALBDBSETIMPL_H
#define ICALBDBSETIMPL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "icalgauge.h"
#include <db.h>

/* This definition is in its own file so it can be kept out of the
   main header file, but used by "friend classes" like icaldirset*/

struct icalbdbset_impl {
  icalset super;		/**< parent class */
  const char *path;
  const char *subdb;
  const char *sindex;
  const char *key;
  void *data;
  int datasize;
  int changed;
  int expand;
  icalrecur_iterator *ritr; /*the last iterator*/
  int occurrence_no; /* the last occurrence number*/
  icalcomponent* last_component; /*the pending recurring component to be processed  */
  icalcomponent* cluster;
  icalgauge* gauge;
  DB_ENV *dbenv;
  DB *dbp;
  DB *sdbp;
  DBC *dbcp;
};

#endif
