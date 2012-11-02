/*
 * log.c - Logging routines
 *
 * This file is part of tcpspy, a TCP/IP connection monitor. 
 *
 * Copyright (c) 2000, 2001 Tim J. Robbins. 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: log.c,v 1.5 2001/01/05 14:05:47 fyre Stab $
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "rcsid.h"

RCSID("$Id: log.c,v 1.5 2001/01/05 14:05:47 fyre Stab $");

/*
 * Exported functions.
 */
void log_set_syslog (void);
void log_set_stdout (void);
void logmsg (const char *fmt, ...);
void vlogmsg (const char *fmt, va_list ap);
void panic (const char *fmt, ...);
void vpanic (const char *fmt, va_list ap);

/*
 * Internal helper functions.
 */
static void vlogmsg_i (int panic, const char *fmt, va_list ap);

/*
 * Methods of logging, set via log_set_foo () family of functions. LM_SYSLOG
 * is the default.
 */
#define LM_SYSLOG	0	/* via syslog(3) */
#define LM_STDOUT	1	/* standard output */
static int logmethod = LM_SYSLOG;

/*
 * log_set_syslog ()
 *
 * Log via syslog(3). openlog () should be called before any of the logging
 * routines to set the ident and facility.
 */
void log_set_syslog (void)
{
	logmethod = LM_SYSLOG;
}

/*
 * log_set_stdout ()
 *
 * Log to standard output.
 */
void log_set_stdout (void)
{
	logmethod = LM_STDOUT;
}

/*
 * logmsg ()
 * 
 * printf-style function to log messages of normal priority.
 */
void logmsg (const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	vlogmsg (fmt, ap);
	va_end (ap);
}

/*
 * vlogmsg ()
 *
 * vprintf-style function to log messages of normal priority.
 */
void vlogmsg (const char *fmt, va_list ap)
{
	vlogmsg_i (0, fmt, ap);	
}

/*
 * panic ()
 *
 * printf-style function to log critical errors and terminate.
 */
void panic (const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	vpanic (fmt, ap);
	/* Not reached, vpanic () exits */
	va_end (ap);
}

/*
 * vpanic ()
 *
 * vprintf-style function to log critical errors and terminate.
 */
void vpanic (const char *fmt, va_list ap)
{
	vlogmsg_i (1, fmt, ap);
	exit (EXIT_FAILURE);
}

/*
 * vlogmsg_i ()
 *
 * Internal routine called by logmsg (), vlogmsg (), panic () and vpanic ().
 * Logs message by specified method.
 */
static void vlogmsg_i (int panic, const char *fmt, va_list ap)
{
	switch (logmethod) {
		case LM_SYSLOG:
			vsyslog (panic == 0 ? LOG_NOTICE : LOG_ERR, fmt, ap);
			break;

		case LM_STDOUT:	
			{
			char timebuf[100];
			time_t now;
			struct tm *t;
			FILE *fp;

			/*
			 * Timestamp the message. If an error occurs, work
			 * around it so messages aren't silently lost.
			 */
			if ((now = time (NULL)) == (time_t) -1)
				snprintf (timebuf, sizeof (timebuf), 
						"<unknown>");
			else if ((t = localtime (&now)) == NULL)
				snprintf (timebuf, sizeof (timebuf), "EPOCH+%u",
					      	(unsigned int) now);
			else if (strftime (timebuf, sizeof (timebuf), 
						"%b %d %H:%M:%S", t) == 0)
				snprintf (timebuf, sizeof (timebuf), "%u", 
						(unsigned int) now);
			/*
			 * Normal messages go to stdout, panic messages go
			 * to stderr.
			 */
			fp = (panic == 0) ? stdout : stderr;
			
			fprintf (fp, "%s: ", timebuf);

			vfprintf (fp, fmt, ap);
			fputc ('\n', fp);
			
			fflush (fp);
			}
			break;
		default:
			assert (0);
	}
}


/*
Oct 30 15:42:00: disconnect: user ubuntu, local 192.168.1.2:49681, remote 198.189.237.151:https
*/
