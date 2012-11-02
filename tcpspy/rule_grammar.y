%{
/*
 * rule_grammar.y, rule_grammar.c - bison parser for rules
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
 * $Id: rule_grammar.y,v 1.6 2001/01/05 14:04:59 fyre Stab $
 */
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "rcsid.h"
#include "rule.h"

RCSID("$Id: rule_grammar.y,v 1.6 2001/01/05 14:04:59 fyre Stab $");

int yyerror ();
extern int yylex ();

extern int firstrule, gotrule;

%}

%union {
	unsigned short boolean;
	uid_t user;
	struct {
		u_int16_t low, high;
	} port;
	struct {
		u_int32_t addr, mask;
	} addr;
	unsigned long exe;
}

%token LPAREN RPAREN

%token USER
%token <user> USER_SPEC
%token LPORT RPORT
%token <port> PORT_SPEC
%token LADDR RADDR
%token <addr> ADDR_SPEC
%token EXE
%token <exe> EXE_SPEC

%left <boolean> OR AND
%right <boolean> NOT

%type <boolean> test

%%

ruleset	:	rule ruleset		{
	if (!firstrule) rule_gen_or ();
	gotrule = 1;
	firstrule = 0;
}

	|	/* empty */		{ } 
	;

rule	:	rule OR rule		{ rule_gen_or (); }
	|	rule AND rule		{ rule_gen_and (); }
	|	NOT rule		{ rule_gen_not (); }
	|	LPAREN rule RPAREN	{ }
	|	test			{ }
	;

test	:	USER USER_SPEC		{ rule_gen_user ($2); }
	|	LPORT PORT_SPEC		{ rule_gen_lport ($2.low, $2.high); }
	|	RPORT PORT_SPEC		{ rule_gen_rport ($2.low, $2.high); }
	|	LADDR ADDR_SPEC		{ rule_gen_laddr ($2.addr, $2.mask); }
	|	RADDR ADDR_SPEC		{ rule_gen_raddr ($2.addr, $2.mask); }
	|	EXE EXE_SPEC		{ rule_gen_exe ($2); }
	;

%%

int yyerror (char *s)
{
	fprintf (stderr, "tcpspy: rule parser: %s\n", s);
	exit (EXIT_FAILURE);
}
