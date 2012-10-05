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


void print_time(FILE *out);
int main (int argc, char *argv[])
{
	//printf("inside main()\n");
	unsigned long lcl;
	unsigned long lclp;
	unsigned long rmt;
	unsigned long rmtp;
	unsigned long uid;
	unsigned long ino;
	static FILE *fp = NULL;
	int x;
	char buf[1024];
	
	
	if (fp == NULL) 
	{
		fp = fopen ("tcpinput", "r");
		if (fp == NULL) printf("/proc/net/tcp: %s", strerror (errno));
		{
			rewind (fp);
		}
	}
	//printf("opened tcp file\n");
	if (fgets (buf, sizeof (buf), fp) == NULL)
	{
		printf("/proc/net/tcp: missing header\n");
	}
	while (x<2)
	{
		rewind (fp);
		while (fgets (buf, sizeof (buf), fp) != NULL) 
		{
			//printf("inside while (fgets(buf, buf.size, tcp)!=NULL\n");
			unsigned long st;
	
			if (sscanf (buf, "%*d: %lx:%lx %lx:%lx %lx %*x:%*x %*x:%*x %*x %lu %*d %lu", &lcl, &lclp, &rmt, &rmtp, &st, &uid, &ino) != 7) 
			{
				//printf("/proc/net/tcp: warning: incomplete line");
				continue;
			}
			else
			{	
				//printf("inside statement to print\n");
				print_time(fp);
				printf(buf,"%*d: %lx:%lx %lx:%lx %lx %*x:%*x %*x:%*x %*x %lu %*d %lu", &lcl, &lclp, &rmt, &rmtp, &st, &uid, &ino);
			}
		}
	sleep(5);
	x++;
	}
	return 0;
}
void print_time(FILE *out)
{
	char t[30];
	struct timeval tv;
	time_t curtime;
   
	gettimeofday(&tv, NULL); 
	curtime=tv.tv_sec;
	strftime(t,30,"%m-%d-%Y  %T.",localtime(&curtime));
	printf(" %s%ld ",t, tv.tv_usec);
	//fprintf(out, " %s%ld ",t, tv.tv_usec);
}

//work on integration
//to call brians program and get it to respond
//put a shell around tcpspy that invokes superspy or tcpspy
//gnat chart tue/thur - 2 to 5
