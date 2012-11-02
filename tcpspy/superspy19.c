#include </usr/include/python2.7/Python.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sqlite3.h>
#include <time.h>


int main (int argc, char *argv[])
{
	//tcp buffer variables
	unsigned long lcl;
	unsigned long lclp;
	unsigned long rmt;
	unsigned long rmtp;
	unsigned long st;
	unsigned long uid;
	unsigned long ino;

	//db and hex to string variables
	int error=0;
	sqlite3 *DB;
	sqlite3_stmt *stmt;
	char lclstring[100];
	char lclpstring[100]; 
	char rmtstring[100];
	char rmtpstring[100]; 
	char ststring[100];
	
	static FILE *fp = NULL;
	int x=0;
	char buf[1024];
	char t[30];
	struct timeval tv;
	time_t curtime;
	
	gettimeofday(&tv, NULL); 
	curtime=tv.tv_sec;
	strftime(t, 30,  "%T.",localtime(&curtime));

//system ("python sqlite_helper.py");
//system ("python interface.py");

// initialize Python
Py_Initialize();

// execute some python code
//PyRun_SimpleString("print 'importing sqlite3'\n");
//PyRun_SimpleString("import sqlite3\n");
//PyRun_SimpleString("print db.execute('SELECT * FROM tcp').fetchall()\n");
//PyRun_SimpleString("db.close()\n");
//PyRun_SimpleString("from time import time,ctime\n"
//                     "print 'Today is',ctime(time())\n");
//close python
Py_Finalize();
	


	//open the database "super_spy.db"
	printf("opening the database...");
	error = sqlite3_open("super_spy.db", &DB);
		if (error)
		{
                	printf("Can not open database");
			printf(" %d\n", error);
                	return 1;
		}
		else
		{
			printf("database open\n");

		}


	if (fp == NULL) 
	{	
		printf("opening tcp file in local folder...\n");
		fp = fopen ("tcp", "r");
		if (fp == NULL) 
		{
			printf("local tcp: %s", strerror (errno));
			rewind (fp); 
		}
	}

	if (fgets (buf, sizeof(buf), fp) == NULL)
	{
		printf("/proc/net/tcp: missing header\n");
	}

	printf("reading local tcp...\n");
	while (x<3)
	{

		while (fgets (buf, sizeof(buf), fp) != NULL) 
		{	
			//print time (terminal)
			gettimeofday(&tv, NULL); 
			curtime=tv.tv_sec;
			strftime(t,30,"%m-%d-%Y  %T.",localtime(&curtime));
			//printf("%s%ld\n",t,tv.tv_usec);


			//fill up buffer from tcp file
			sscanf (buf,"%*d: %lx:%lx %lx:%lx %lx %*x:%*x %*x:%*x %*x %lu %*d %*d", &lcl, &lclp, &rmt, &rmtp, &st, &uid);


			//store each variable as a srting
			sprintf(lclstring, "%lx", lcl);
			printf("%s ", lclstring);
			sprintf(lclpstring, "%lx", lclp);
			printf("%s ", lclpstring);
			sprintf(rmtstring, "%lx", rmt);
			printf("%s ", rmtstring);
			sprintf(rmtpstring, "%lx", rmtp);
			printf("%s ", rmtpstring);
			sprintf(ststring, "%lx", st);
			printf("%s\n", ststring);


			// Create a prepared statement for db
			error = sqlite3_prepare_v2(DB, "insert into tcp values (?, ?, ?, ?, ?);", -1, &stmt, NULL);
				if (error != SQLITE_OK)
				{
					printf("error = sqlite3_prepare_v2: ");
					printf("%d\n", error);
				}
			//bind variables to columns in db
			 sqlite3_bind_text(stmt, 1, lclstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 2, lclpstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 3, rmtstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 4, rmtpstring, 100, SQLITE_STATIC); 
			 sqlite3_bind_text(stmt, 5, ststring, 100, SQLITE_STATIC); 


			// Execute the sql statement to fill super_spy.db
			error = sqlite3_step(stmt);
				if (error != SQLITE_DONE)
				{
					printf("error = sqlite3_step: ");
					printf("%d\n", error);				
				}
			//reset sql statement for next turn in loop
			sqlite3_reset(stmt);
				
			sleep(0);
		}
	x++;

	}

	//free the file
	fclose(fp);
	
	// Free the sql statement
	error = sqlite3_finalize(stmt);

	return 0;
}



//the most important thing now is to have tcpspy and superspy working from the same code...
//some kind of wrapper or something
//
//
//http://www.linuxjournal.com/article/3641
//had to install python-dev to embed into c program (for Python.h) and still had to link to library
//for some reason python not linking to .so
//gcc superspy15.c -o spy -I/usr/include/python2.7 -pthread -lm -ldl -lutil -lpython2.7
//gcc -o sqltest2 sql_test2.c -lsqlite3
//
//work on integration - check depend file in repo
//fix time issue
//no redundant connections
//put a shell around tcpspy that invokes superspy or tcpspy

//check.sourceforge.net c-unit tester

//Store last entry in /proc/net/tcp as a variable.  Check that against the
//file to avoid redundancy.

//gcc superspy19.c -o spy -lsqlite3 -I/usr/include/python2.7 -pthread -lm -ldl -lutil -lpython2.7


