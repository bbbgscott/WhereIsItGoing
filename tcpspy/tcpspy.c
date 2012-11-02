/*
 * tcpspy.c - TCP/IP connection monitor.
 *
 * This file is part of tcpspy, a TCP/IP connection monitor.
 *
 * Copyright (c) 2000, 2001, 2002 Tim J. Robbins. 
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
 * $Id: tcpspy.c,v 1.39.1.3 2001/07/02 11:55:36 tim Stab $
 */

//superspy addition
#include <sqlite3.h>
//----------------

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SYSLOG_NAMES
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "rcsid.h"
#include "rule.h"

RCSID("$Id: tcpspy.c,v 1.39.1.3 2001/07/02 11:55:36 tim Stab $");

/*
 * Defaults for compile-time settings. Descriptions of these are in
 * the Makefile.
 */
#ifndef CONNTABLE_BUCKETS
#define CONNTABLE_BUCKETS 5003
#endif
#ifndef FACILITY
#define FACILITY LOG_USER
#endif

int gotrule = 0;

static int stopped = 0, showprocs = 0, db = 0;
static int facility = FACILITY;

/*
 * This structure holds everything we need to know about a connection. We
 * use unsigned long instead of (for example) uid_t, ino_t to make hashing
 * easier.
 */
typedef struct conn {
	unsigned long lcl;
	unsigned long lclp;
	unsigned long rmt;
	unsigned long rmtp;
	unsigned long uid;
	unsigned long ino;

	char exe[PATH_MAX];
	
	struct conn *next;
} conn_t;

typedef struct conntable {
	conn_t *buckets[CONNTABLE_BUCKETS];
} conntable_t;

static int ct_init (conntable_t *ct);
static int ct_hash (conn_t *c);
static int ct_add (conntable_t *ct, conn_t *c);
static int ct_find (conntable_t *ct, conn_t *c);
static int ct_read (conntable_t *ct);
static int ct_free (conntable_t *ct);
static void huntinode (ino_t i, char *buf, size_t bufsize);
static void logconn (conn_t *c, const char *action);
static void logdb (conn_t *c, const char *action);
static void compare (conntable_t *old, conntable_t *new);
static void stopsig (int signal);

/*
 * stopsig ()
 *
 * Stop the program when certain signals are received. 
 */
static void stopsig (int signo)
{
	logmsg ("caught signal %d, shutting down", signo);
	stopped = 1;
}

/*
 * ct_init ()
 *
 * Initialise a connection table (hashtable).
 */
static int ct_init (conntable_t *ct)
{
	int i;
	
	assert (ct != NULL);

	for (i = 0; i < CONNTABLE_BUCKETS; i++)
		ct->buckets[i] = NULL;

	return 1;
}

/*
 * ct_hash ()
 *
 * Simple hash function for connections.
 */
static int ct_hash (conn_t *c)
{
	unsigned long h;

	assert (c != NULL);

	h = c->lcl ^ c->lclp ^ c->rmt ^ c->rmtp ^ c->uid ^ c->ino;

	return h % CONNTABLE_BUCKETS;
}

/*
 * ct_add ()
 *
 * Add a connection to the connection table.
 */
static int ct_add (conntable_t *ct, conn_t *c)
{
	conn_t *old, *newc;
	int bi;
	
	assert (ct != NULL);
	assert (c != NULL);

	newc = (conn_t *) malloc (sizeof (conn_t));
	if (newc == NULL) panic ("memory exhausted");	

	memcpy (newc, c, sizeof (conn_t));

	bi = ct_hash (c);
	old = ct->buckets[bi];
	ct->buckets[bi] = newc;
	ct->buckets[bi]->next = old;

	return 1;
}

/*
 * ct_find ()
 * 
 * Find a connection in a table, return nonzero if found, zero otherwise.
 */
static int ct_find (conntable_t *ct, conn_t *c)
{
	conn_t *bucket;
	
	assert (ct != NULL);
	assert (c != NULL);

	bucket = ct->buckets[ct_hash (c)];
	while (bucket != NULL) {
		if ((c->lcl == bucket->lcl) && (c->lclp == bucket->lclp) && 
			(c->rmt == bucket->rmt) && (c->rmtp == bucket->rmtp) &&
			(c->uid == bucket->uid) && (c->ino == bucket->ino)) {
			return 1;
		}
		bucket = bucket->next;
	}

	return 0;
}

/*
 * ct_read ()
 * 
 * Read /proc/net/tcp and add all connections to the table if connections
 * of that type are being watched.
 */
static int ct_read (conntable_t *ct)
{
	static FILE *fp = NULL;
	char buf[1024];
	conn_t c;
	
	assert (ct != NULL);

	if (fp == NULL) {
		fp = fopen ("/proc/net/tcp", "r");
		if (fp == NULL) panic ("/proc/net/tcp: %s", strerror (errno));
	}
	rewind (fp);

	if (fgets (buf, sizeof (buf), fp) == NULL)
		panic ("/proc/net/tcp: missing header");
	
	while (fgets (buf, sizeof (buf), fp) != NULL) {
		unsigned long st;

		if (sscanf (buf, "%*d: %lx:%lx %lx:%lx %lx %*x:%*x %*x:%*x %*x %lu %*d %lu", &c.lcl, &c.lclp, &c.rmt, &c.rmtp, &st, &c.uid, &c.ino) != 7) {
			logmsg ("/proc/net/tcp: warning: incomplete line");
			continue;
		}
		if ((c.ino == 0) || (st != TCP_ESTABLISHED)) continue;
		if (showprocs != 0)
			huntinode ((ino_t) c.ino, c.exe, sizeof (c.exe));

		if (ct_add (ct, &c) == 0)
			return 0;
	}

	return 1;
}

/*
 * ct_free ()
 *
 * Free a connection table.
 */
static int ct_free (conntable_t *ct)
{
	int i;

	assert (ct != NULL);

	for (i = 0; i < CONNTABLE_BUCKETS; i++) {
		conn_t *c0, *c1;

		c0 = ct->buckets[i];
		while (c0 != NULL) {
			c1 = c0->next;
			free (c0);
			c0 = c1;
		}
		ct->buckets[i] = NULL;
	}

	return 1;
}

/*
 * huntinode ()
 *
 * Find names processes using an inode and put them in a buffer.
 */
static void huntinode (ino_t i, char *buf, size_t bufsize)
{
	DIR *procdir;
	struct dirent *procent;
	
	assert (buf != NULL);
	*buf = '\0';

	if ((procdir = opendir ("/proc")) == NULL)
		panic ("/proc: %s", strerror (errno));
	while ((procent = readdir (procdir)) != NULL) {
		char fdbuf[PATH_MAX];
		DIR *fddir;
		struct dirent *fdent;

		/*
		 * No test needed for "." and ".." since they don't begin
		 * with digits.
		 */
		if (! isdigit (*procent->d_name))
			continue;
		
		snprintf (fdbuf, sizeof (fdbuf), "/proc/%s/fd", 
				procent->d_name);
		
		/*
		 * We're don't always run as root, we may get EPERM here,
		 * ignore it.
		 */
		if ((fddir = opendir (fdbuf)) == NULL)
			continue;

		while ((fdent = readdir (fddir)) != NULL) {
			int len;
			char lnkbuf[PATH_MAX], lnktgt[PATH_MAX];
			char exebuf[PATH_MAX], exetgt[PATH_MAX];
			ino_t this_ino;
			
			if (! isdigit (*fdent->d_name))
				continue;
			snprintf (lnkbuf, sizeof (lnkbuf), "%s/%s", fdbuf, 
					fdent->d_name);
			len = readlink (lnkbuf, lnktgt, sizeof (lnktgt) - 1);
			if (len < 0)
				continue;
			lnktgt[len] = '\0';
			if (sscanf (lnktgt, "socket:[%lu]", &this_ino) != 1)
				continue;
			if (this_ino != i)
				continue;

			snprintf (exebuf, sizeof (exebuf), "/proc/%s/exe", 
					procent->d_name);
			len = readlink (exebuf, exetgt, sizeof (exetgt) - 1);
			if (len < 0)
				continue;
			exetgt[len] = '\0';

			strncpy (buf, exetgt, bufsize);
			buf[bufsize - 1] = '\0';
		}

		closedir (fddir);
	}
	closedir (procdir);
}

/*
 * logconn ()
 *
 * Log a connection/disconnection.
 */
static void logconn (conn_t *c, const char *action)
{
	struct in_addr in;
	char laddr[16], raddr[16]; /* assume IPv4 nnn.nnn.nnn.nnn\0 */
	char *n;
	struct passwd *pw;
	char uidbuf[32];
	char *user;
	struct servent *se;
	char lport[64], rport[64];

	assert (c != NULL);
	assert (action != NULL);

	if ((gotrule != 0) && (rule_eval (c->uid, c->lcl, c->lclp, c->rmt, c->rmtp, (showprocs != 0) ? c->exe : NULL) == 0))
		return;

	in.s_addr = c->lcl;
	n = inet_ntoa (in);
	strncpy (laddr, n, sizeof (laddr));
	laddr[sizeof (laddr) - 1] = '\0';
	in.s_addr = c->rmt;
	n = inet_ntoa (in);
	strncpy (raddr, n, sizeof (raddr));
	raddr[sizeof (raddr) - 1] = '\0';

	snprintf (lport, sizeof (lport), "%lu", c->lclp);
	snprintf (rport, sizeof (rport), "%lu", c->rmtp);

	if ((se = getservbyport (htons (c->lclp), "tcp")) != NULL) {
		strncpy (lport, se->s_name, sizeof (lport));
		lport[sizeof (lport) - 1] = '\0';
	}
	if ((se = getservbyport (htons (c->rmtp), "tcp")) != NULL) {
		strncpy (rport, se->s_name, sizeof (rport));
		rport[sizeof (rport) - 1] = '\0';
	}

	if ((pw = getpwuid ((uid_t) c->uid)) == NULL) {
		snprintf (uidbuf, sizeof (uidbuf),
				"(uid %u)", (unsigned int)(uid_t)c->uid);
		user = uidbuf;
	} else {
		user = pw->pw_name;
	}

	if (showprocs != 0)
		logmsg ("%s: proc %.30s, user %s, local %s:%s, remote %s:%s",
			action, *c->exe != '\0' ? c->exe : "(unknown)", 
			user, laddr, lport, raddr, rport);
	else
		logmsg ("%s: user %s, local %s:%s, remote %s:%s",
				action, user, laddr, lport, raddr, rport);
}
/*
 * logdb ()
 *
 * Log a connection/disconnection to the database.
 */
static void logdb (conn_t *c, const char *action)
{

	struct in_addr in;
	char laddr[16], raddr[16]; /* assume IPv4 nnn.nnn.nnn.nnn\0 */
	char *n;
	struct passwd *pw;
	char uidbuf[32];
	char *user;
	struct servent *se;
	char lport[64], rport[64];


	//db variables
	int error=0;
	sqlite3 *DB;
	sqlite3_stmt *stmt;
	char lclstring[16];
	char lclpstring[64]; 
	char rmtstring[16];
	char rmtpstring[64]; 
	//db variables

	assert (c != NULL);
	assert (action != NULL);


	//open the database "super_spy.db"
	error = sqlite3_open("super_spy.db", &DB);
		if (error)
		{
                	printf("Can not open database, SQL error: ");
			printf(" %d\n", error);
		}

	if ((gotrule != 0) && (rule_eval (c->uid, c->lcl, c->lclp, c->rmt, c->rmtp, (showprocs != 0) ? c->exe : NULL) == 0))
		return;

	in.s_addr = c->lcl;
	n = inet_ntoa (in);
	strncpy (laddr, n, sizeof (laddr));
	laddr[sizeof (laddr) - 1] = '\0';
	in.s_addr = c->rmt;
	n = inet_ntoa (in);
	strncpy (raddr, n, sizeof (raddr));
	raddr[sizeof (raddr) - 1] = '\0';

	snprintf (lport, sizeof (lport), "%lu", c->lclp);
	snprintf (rport, sizeof (rport), "%lu", c->rmtp);

	sprintf(lclpstring, "%s", lport);//addition
	sprintf(rmtpstring, "%s", rport);//addition
	sprintf(lclstring, "%s", laddr);//addition
	sprintf(rmtstring, "%s", raddr);//addition


	// Create a prepared statement for db
	error = sqlite3_prepare_v2(DB, "insert into tcp values (?, ?, ?, ?, ?);", -1, &stmt, NULL);
		if (error != SQLITE_OK)
		{
			printf("error = sqlite3_prepare_v2: ");
			printf("%d\n", error);
		}
		//bind variables to columns in db
			 sqlite3_bind_text(stmt, 2, lclstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 3, lclpstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 4, rmtstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 5, rmtpstring, 100, SQLITE_STATIC); 


		// Execute the sql statement to fill super_spy.db
		error = sqlite3_step(stmt);
			if (error != SQLITE_DONE)
			{
				printf("error = sqlite3_step: ");
				printf("%d\n", error);				
			}
		//reset sql statement for next turn in loop
		sqlite3_reset(stmt);

		// Free the sql statement
		sqlite3_finalize(stmt);

		//Close db
		sqlite3_close(DB);
}

/*
 * compare ()
 *
 * Compare the `old' and `new' tables, logging any differences.
 */
static void compare (conntable_t *old, conntable_t *new)
{
	int i;
	
	assert (old != NULL);
	assert (new != NULL);
	
	for (i = 0; i < CONNTABLE_BUCKETS; i++) 
	{
		conn_t *bucket;

		bucket = old->buckets[i];
		while (bucket != NULL) 
		{
			if (ct_find (new, bucket) == 0)
				logconn (bucket, "disconnect");	
			bucket = bucket->next;
		}
	}

	for (i = 0; i < CONNTABLE_BUCKETS; i++) 
	{
		conn_t *bucket;

		bucket = new->buckets[i];
		while (bucket != NULL) 
		{
			if (ct_find (old, bucket) == 0)
			{
				logconn (bucket, "connect");
				if(db != 0)
				{
					logdb (bucket, "connect");
				}
			}
			bucket = bucket->next;
		}
	}
}

/*
 * getfacility ()
 *
 * Convert a facility name to a integer facility value for syslog().
 */
int getfacility (const char *s)
{
	int i;

	assert (s != NULL);

	for (i = 0; i < LOG_NFACILITIES - 2; i++) {
		assert (facilitynames[i].c_name != NULL);
		if (strcmp (facilitynames[i].c_name, s) == 0)
			return facilitynames[i].c_val;
	}

	return -1;
}

static void usage (void)
{
	fprintf (stderr, "usage: tcpspy [-dp] [-e rule]... [-f rulefile] [-I interval] "
			"[-U user]\n\t[-G group] [-F facility] [-I interval]\n");
	exit (EXIT_FAILURE);
}

int firstrule = 1;

int main (int argc, char *argv[])
{
	conntable_t old, new;
	unsigned long interval = 1000;
	int ch;
	uid_t dropuser = -1;
	gid_t dropgroup = -1, defgroup = -1;
	int debug = 0;

	log_set_syslog ();

	/*
	 * Parse our arguments.
	 */
	opterr = 0;
	while ((ch = getopt (argc, argv, "dpze:I:U:G:u:w:i:f:F:")) != -1) {
		switch (ch) {
			case 'd':
				debug = 1;
				log_set_stdout ();
				break;
			case 'p':
				showprocs = 1;
				break;
			case 'z':
				db = 1;
				break;
			case 'e':
				rule_parse (optarg);
				gotrule = 1;
				if (firstrule == 0) 
					/*
					 * Join multiple rules together with
					 * logical OR's.
					 */
					rule_gen_or ();
				firstrule = 0;
				/* Hide rule from `ps' */
				memset (optarg, '\0', strlen (optarg));
				break;
			case 'f':
			{
				FILE *fp;

				if ((fp = fopen (optarg, "r")) == NULL) {
					fprintf (stderr, "tcpspy: %s: %s\n", optarg, strerror (errno));
					exit (EXIT_FAILURE);
				}

				rule_parse_file (fp);

				fclose (fp);
			}
			break;
			case 'I':
				interval = atoi (optarg);
				if (interval == 0) {
					fprintf (stderr, "tcpspy: bad interval\n");
					exit (EXIT_FAILURE);
				}
				break;
			case 'U':
				{
				struct passwd *pw;
				
				if (isdigit (*optarg)) {
					dropuser = atoi (optarg);
					pw = getpwuid (atoi (optarg));
				} else {
					if ((pw = getpwnam (optarg)) == NULL) {
						fprintf (stderr, "tcpspy: user `%s' unknown\n", optarg);
						exit (EXIT_FAILURE);
					}
					dropuser = pw->pw_uid;
				}
				
				/*
				 * Use the gid from the password file entry if
				 * possible, as a default.
				 */
				if (pw != NULL)
					defgroup = pw->pw_gid;
				else
					defgroup = (gid_t) -1;
				
				}
				break;
			case 'G':
				if (isdigit (*optarg))
					dropgroup = atoi (optarg);
				else {
					struct group *gr;

					if ((gr = getgrnam (optarg)) == NULL) {
						fprintf (stderr, "tcpspy: group `%s' unknown\n", optarg);
						exit (EXIT_FAILURE);
					}
					dropgroup = gr->gr_gid;
				}	
				break;
			case 'F':
				if ((facility = getfacility (optarg)) < 0) {
					fprintf (stderr, "tcpspy: bad facility name `%s'\n", optarg);
					exit (EXIT_FAILURE);
				}
				break;

			case 'u': case 'i':
				fprintf (stderr, "tcpspy: -%c option is obsolete\n", ch);
				/* fall through to usage message */
			default:
				usage();
		}
	}

	argc -= optind;
	argv += optind;

	/*
	 * Become an unprivileged user for safety purposes if requested.
	 */
	if ((dropgroup == (uid_t) -1) && (defgroup != (uid_t) -1))
		dropgroup = defgroup;
	if (dropgroup != (gid_t) -1) {
		if (setgid (dropgroup) < 0) {
			fprintf (stderr, "tcpspy: setgid: %s\n", strerror (errno));
			exit (EXIT_FAILURE);
		}
	}
	if (dropuser != (uid_t) -1) {
		if (setuid (dropuser) < 0) {
			fprintf (stderr, "tcpspy: setuid: %s\n", strerror (errno));
			exit (EXIT_FAILURE);
		}
	}

	/*
	 * Become a daemon by double-forking and detaching completely from
	 * the terminal.
	 */

	if (debug == 0) {
		pid_t p;

		/* 1st fork */
		p = fork();
		if (p < 0) {
			fprintf (stderr, "tcpspy: fork: %s\n",
					strerror (errno));
			exit (EXIT_FAILURE);
		} else if (p != 0)
			exit (0);

		/* 2nd fork */
		p = fork();
		if (p < 0) {
			fprintf (stderr, "tcpspy: fork: %s\n",
					strerror (errno));
			exit (EXIT_FAILURE);
		} else if (p != 0) {
			fprintf (stderr, "tcpspy 1.7d started (pid %d)\n", 
					(int) p);
			exit (EXIT_SUCCESS);
		}

		ioctl (STDIN_FILENO, TIOCNOTTY, NULL);
		close (STDIN_FILENO); 
		close (STDOUT_FILENO); 
		close (STDERR_FILENO);
		setpgid (0, 0);
		chdir ("/");
	} else
		fprintf (stderr, "tcpspy 1.7d started (debug)\n");

	signal (SIGTERM, stopsig);
	if (debug != 0) signal (SIGINT, stopsig);

	/*
	 * Initialisation's done, start watching for connections.
	 */

	openlog ("tcpspy", LOG_PID, facility);
	if (debug == 0)
		/*
		 * .. for the benefit of those reading the logs. Useless to
		 * people running it in debug mode.
		 */
		logmsg ("tcpspy started, monitoring connections");

	if (ct_init (&old) == 0) panic ("ct_init failed");
	if (ct_read (&old) == 0) panic ("ct_read failed");
	
	while (stopped == 0) {
		struct timeval tv1, tv2;
		static double elapsed = 0.0;
		static int slow_warn = 0;

		gettimeofday (&tv1, NULL);

		if (ct_init (&new) == 0) panic ("ct_init failed");
		if (ct_read (&new) == 0) panic ("ct_read failed");
		compare (&old, &new);
		if (ct_free (&old) == 0) panic ("ct_free failed");
		memcpy (&old, &new, sizeof (conntable_t));

		gettimeofday (&tv2, NULL);

		/*
		 * If the time taken to poll the currently open connections is longer than
		 * the time between checks, emit a warning message.
		 */
		elapsed += (double) ((tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec));
		elapsed /= 2;
		if ((elapsed > (double)(interval * 1000)) && (slow_warn == 0)) {
			slow_warn = 1;
			logmsg ("warning: running slowly, suggest increasing interval with -I option");
		}

		usleep (interval * 1000);
	}

	if (ct_free (&old) == 0) panic ("ct_free failed");
	closelog ();

	return EXIT_SUCCESS;
}
