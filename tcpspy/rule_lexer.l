%{
/*
 * rule_lexer.l, rule_lexer.c - lex lexer for rules
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
 * $Id: rule_lexer.l,v 1.8 2001/01/05 14:05:26 fyre Stab $
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "rcsid.h"
#include "rule_grammar.h"
#include "rule.h"

RCSID("$Id: rule_lexer.l,v 1.8 2001/01/05 14:05:26 fyre Stab $");

static void rule_lexer_error (const char *msg);

#define YY_FATAL_ERROR(msg) rule_lexer_error(msg)

%}

%option noyywrap
%option nounput
%option never-interactive
%option case-insensitive
%option warn

%x WANT_USER WANT_PORT WANT_ADDR WANT_EXE

DIGIT			[0-9]
NUMBER			{DIGIT}+
QSTRING			\"[^"]*\"
IPADDR			{NUMBER}\.{NUMBER}\.{NUMBER}\.{NUMBER}

%%

"("			return LPAREN;
")"			return RPAREN;
"or"			return OR;
"and"			return AND;
"not"			return NOT;

<INITIAL>user		{
			BEGIN (WANT_USER);
			return USER;
			}
<WANT_USER>{NUMBER}	{
			BEGIN (INITIAL);
			yytext[yyleng] = '\0';
			rulelval.user = (uid_t) atoi (yytext); 
			return USER_SPEC;
			}
<WANT_USER>{QSTRING}	{
			struct passwd *pw;

			BEGIN (INITIAL);

			yytext[yyleng-1] = '\0';
			if ((pw = getpwnam (yytext + 1)) == NULL)
				YY_FATAL_ERROR ("unknown user");
			rulelval.user = pw->pw_uid;
			return USER_SPEC;
			}

<INITIAL>lport		{
			BEGIN (WANT_PORT);
			return LPORT;
			}
<INITIAL>rport		{
			BEGIN (WANT_PORT);
			return RPORT;
			}
<WANT_PORT>({NUMBER})?-?({NUMBER})?	{
			const char *p = yytext;
			yytext[yyleng] = '\0';
	
			BEGIN (INITIAL);

			rulelval.port.low = 0;
			while (isdigit (*p)) {
				rulelval.port.low *= 10;
				rulelval.port.low += (*p++) - '0';
			}
			if (*p == '-') {
				p++;
				if (isdigit (*p)) {
					rulelval.port.high = 0;
					while (isdigit (*p)) {
						rulelval.port.high *= 10;
						rulelval.port.high += (*p++) - '0';
					}
				} else rulelval.port.high = 65535;
			} else {
				rulelval.port.high = rulelval.port.low;
			}
			if (rulelval.port.high < rulelval.port.low)
				YY_FATAL_ERROR ("bad port range, high < low");
			return PORT_SPEC;
			}
<WANT_PORT>{QSTRING}	{
			struct servent *se;

			BEGIN (INITIAL);

			yytext[yyleng - 1] = '\0';
			if ((se = getservbyname (yytext + 1, "tcp")) == NULL)
				YY_FATAL_ERROR ("unknown service");
			rulelval.port.low = rulelval.port.high = (u_int16_t) ntohs (se->s_port);
			return PORT_SPEC;
			}

<INITIAL>laddr		{
			BEGIN (WANT_ADDR);
			return LADDR;
			}
<INITIAL>raddr		{
			BEGIN (WANT_ADDR);
			return RADDR;
			}
<WANT_ADDR>{IPADDR}(\/{IPADDR})? {
			struct in_addr in;
			char *s;
			BEGIN (INITIAL);

			yytext[yyleng] = '\0';
			if ((s = strchr (yytext, '/')) != NULL) {
				*s++ = '\0'; 
				if (inet_aton (s, &in) == 0)
					YY_FATAL_ERROR ("bad net mask");
				rulelval.addr.mask = in.s_addr;
			} else rulelval.addr.mask = 0xFFFFFFFF;
			if (inet_aton (yytext, &in) == 0)
				YY_FATAL_ERROR ("bad IP address");
			rulelval.addr.addr = in.s_addr;

			return ADDR_SPEC;
			}

<INITIAL>exe		{
			BEGIN (WANT_EXE);
			return EXE;
			}

<WANT_EXE>{QSTRING}	{
			yytext[yyleng - 1] = '\0';
			rulelval.exe = st_store (yytext + 1);
			return EXE_SPEC;
			}

<*>#.*\n		{ /* eat comments */ }

<*>[ \n\t]		{ /* whitespace is ignored */ }

<*>.			{ return *yytext; }

%%

static void rule_lexer_error (const char *msg)
{
	(void)yy_fatal_error; /* XXX Nasty hack to silence warning */
	fprintf (stderr, "tcpspy: rule lexer: %s\n", msg);
	exit (EXIT_FAILURE);
}

/*
 * Hack: The prototype for yy_scan_string() is
 * 		YY_BUFFER_STATE yy_scan_string (const char *str);
 * ... but rule.c does not know what YY_BUFFER_STATE is.
 */
void rule_lexer_scan_string (const char *r)
{
	yy_scan_string (r);
}
