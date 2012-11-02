/*
 * rcsid.h - Revision Control System Id macro
 *
 * This file is part of tcpspy, a TCP/IP connection monitor.
 *
 * $Id: rcsid.h,v 1.1 2000/12/19 09:57:21 fyre Stab $
 */

#ifndef RCSID_H
#define RCSID_H

#ifdef __GNUC__
#  define RCSID(x) __attribute__ ((unused)) static char *rcsid = x
#else
#  define RCSID(x) static char *rcsid = x
#endif

#endif
