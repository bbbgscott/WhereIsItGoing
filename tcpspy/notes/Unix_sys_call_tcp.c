/*
system_call.c
scott heinz

use unix system call to print out first 5 lines of tcp
to text file.
*/

#include<stdio.h>

int main(int argc, char *argv[])
{

system("more +2 /proc/net/tcp | cut -f5,6,7 -d' '>test.txt");

return 0;
}
