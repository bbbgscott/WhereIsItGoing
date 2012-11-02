# Makefile for tcpspy
# Tim J. Robbins, 2000
# $Id: Makefile,v 2.5 2001/04/25 01:40:17 tim Stab $

# You may specify the syslog facility to use here. If in doubt, use LOG_DAEMON
# or LOG_LOCAL[0-7]. See the syslog(3) manual page for a complete list of
# facilities.
CFLAGS+=-DFACILITY=LOG_LOCAL1

# You may also override the default number of buckets in the connection table
# here, but this not usually necessary.
#CFLAGS+=-DCONNTABLE_BUCKETS=5003

# Comment out the following line to enable debugging (slower!)
CFLAGS+=-DNDEBUG

# Add any other options for the compiler here
CFLAGS+=-O2 -Wall -W -lsqlite3


default: tcpspy
all: tcpspy doc

tcpspy: log.o rule_lexer.o rule_grammar.o rule.o tcpspy.o
	gcc log.o rule_lexer.o rule_grammar.o rule.o tcpspy.o -o tcpspy -lsqlite3

log.o: log.c

rule_lexer.o: rule_lexer.c
rule_lexer.c: rule_grammar.c rule_lexer.l
	flex -Prule -orule_lexer.c rule_lexer.l

rule_grammar.o: rule_grammar.c
rule_grammar.c: rule_grammar.y
	bison -p rule -o rule_grammar.c -d rule_grammar.y

rule.o: rule.c

tcpspy.o: tcpspy.c

doc:
	groff -Tps -man tcpspy.8 >tcpspy.ps
	ps2pdf tcpspy.ps tcpspy.pdf

clean:
	rm -f log.o rule_lexer.o rule_grammar.o rule_lexer.c \
		rule_grammar.c rule_grammar.h rule.o tcpspy.o tcpspy \
		tcpspy.ps tcpspy.pdf

install: tcpspy
	install -m 644 -D tcpspy.8 /usr/local/man/man8/tcpspy.8
	install -m 755 -D tcpspy /usr/local/sbin/tcpspy
