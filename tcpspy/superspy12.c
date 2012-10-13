//#include </usr/include/python2.6/Python.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
//#include <limits.h>
//#include <grp.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <pwd.h>
//#include <signal.h>
//#include <stdarg.h>
//#include <stdlib.h>
//#include <dirent.h>
//#include <syslog.h>
//#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#include <unistd.h>


void print_time();
int main (int argc, char *argv[])
{
	//printf("inside main()\n");
	unsigned long lcl;
	unsigned long lclp;
	unsigned long rmt;
	unsigned long rmtp;
	unsigned long st;
	unsigned long uid;
	unsigned long ino;
	static FILE *fp = NULL;
	int x=0;
	char buf[1024];
	char t[30];
	struct timeval tv;
	time_t curtime1, curtime2, difference;
	
	if (fp == NULL) 
	{
		fp = fopen ("tcpinput", "r");
		if (fp == NULL) 
		{
			printf("/proc/net/tcp: %s", strerror (errno));
			rewind (fp);
			//fseek(fp, 90, SEEK_SET);
 
		}
	}

	//printf("opened tcp file\n");
	if (fgets (buf, sizeof(buf), fp) == NULL)
	{
		printf("/proc/net/tcp: missing header\n");
	}

	while (x<400)
	{
		rewind (fp);
		printf("time stamp# %d: ", x);
		gettimeofday(&tv, NULL); 
		curtime1=tv.tv_sec;
		strftime(t,30,"%m-%d-%Y  %T.",localtime(&curtime1));
		printf("%s%ld\n",t, tv.tv_usec);

		//print_time();

		while (fgets (buf, sizeof(buf), fp) != NULL) 
		{
			//printf("inside while (fgets(buf, buf.size, tcp)!=NULL\n");
	
			sscanf (buf,"%*d: %lx:%lx %lx:%lx %lx %*x:%*x %*x:%*x %*x %lu %*d %*d", &lcl, &lclp, &rmt, &rmtp, &st, &uid);
			printf("%lx %lx %lx %lx %lx\n", lcl, lclp, rmt, rmtp, st);
		

		}

	//sleep(7);
	x++;

	}
	gettimeofday(&tv, NULL); 
	curtime2=tv.tv_sec;
	strftime(t,30,"%m-%d-%Y  %T.",localtime(&curtime2));
	difference=curtime2-curtime1;
	printf("%s%ld\n",t, tv.tv_usec);
	printf("time elapsed: ");
	printf("%lu\n", difference);

	fclose(fp);
	//system("python geo_helper.py");
	return 0;
}
void print_time()
{

	char t[30];
	struct timeval tv;
	time_t curtime;
   
	gettimeofday(&tv, NULL); 
	curtime=tv.tv_sec;
	strftime(t,30,"%m-%d-%Y  %T.",localtime(&curtime));
	printf("%s%ld\n",t, tv.tv_usec);
	//fprintf(out, "%s%ld\n ",t, tv.tv_usec);
}

//work on integration
//to call brians program and get it to respond
//put a shell around tcpspy that invokes superspy or tcpspy

//check.sourceforge.net c-unit tester
