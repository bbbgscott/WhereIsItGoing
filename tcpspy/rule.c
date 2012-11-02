/*
 * rule.c - Bytecode compiler and interpreter for rules
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
 * $Id: rule.c,v 1.9 2001/01/05 14:04:41 fyre Stab $
 */

/*
 * Rule Bytecode Generator and Interpreter
 *
 * This file contains the routines that are called by the parser to generate
 * bytecode, and the routines that execute the bytecode.
 */

#include <assert.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "rcsid.h"

/*
 * These header files contain the data types used in "rule_grammar.h".
 */
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#include "rule_grammar.h"
#include "rule.h"

RCSID("$Id: rule.c,v 1.9 2001/01/05 14:04:41 fyre Stab $");

typedef unsigned long bytecode_t;
typedef unsigned char bool_t;

/*
 * Exported functions.
 */
void rule_gen_user (uid_t uid);
void rule_gen_lport (u_int16_t low, u_int16_t high);
void rule_gen_rport (u_int16_t low, u_int16_t high);
void rule_gen_laddr (u_int32_t addr, u_int32_t mask);
void rule_gen_raddr (u_int32_t addr, u_int32_t mask);
void rule_gen_exe (unsigned long estrid);
void rule_gen_or (void);
void rule_gen_and (void);
void rule_gen_not (void);
int rule_eval (uid_t r_uid, u_int32_t laddr, u_int16_t lport, 
		u_int32_t raddr, u_int16_t rport, const char *mexe);
void rule_parse (const char *r);
unsigned long st_store (const char *s);

/*
 * Internal helper functions.
 */
static void add_code (bytecode_t c);
static const char *st_retrieve (unsigned long id);

/*
 * Module-level variables.
 */

#define CODE_SIZE_STEP		512
#define STACK_SIZE_STEP		512
#define STRINGTAB_SIZE_STEP	16

/*
 * The bytecode is stored in an array that is resized according to demand
 * at runtime.
 */
static bytecode_t *code = NULL;
static size_t code_size = 0 /* size of block pointed to by 'code' */, 
	code_length = 0; /* number of instructions in 'code' */

/*
 * The string table is an array of pointers to strings.
 */
static char **stringtab = NULL;
static size_t stringtab_size = 0, stringtab_length = 0;

/*
 * Bytecode instructions.
 */
#define BC_NONE		0
#define BC_USER		1
#define BC_LPORT	2
#define BC_RPORT	3
#define BC_LADDR	4
#define BC_RADDR	5
#define BC_EXE		6

#define BC_OR		64
#define BC_AND		65
#define BC_NOT		66

/*
 * Comparisons.
 */

/*
 * rule_gen_user ()
 *
 * Generate a user comparison.
 */
void rule_gen_user (uid_t uid)
{
	add_code (BC_USER);
	add_code ((bytecode_t) uid);
}

/*
 * rule_gen_lport ()
 *
 * Generate a local port comparison.
 */
void rule_gen_lport (u_int16_t low, u_int16_t high)
{
	add_code (BC_LPORT);
	add_code ((bytecode_t) low);
	add_code ((bytecode_t) high);
}

/*
 * rule_gen_rport ()
 *
 * Generate a remote port comparison.
 */
void rule_gen_rport (u_int16_t low, u_int16_t high)
{
	add_code (BC_RPORT);
	add_code ((bytecode_t) low);
	add_code ((bytecode_t) high);
}

/*
 * rule_gen_laddr ()
 *
 * Generate a local address comparison.
 */
void rule_gen_laddr (u_int32_t addr, u_int32_t mask)
{
	add_code (BC_LADDR);
	add_code ((bytecode_t) addr);
	add_code ((bytecode_t) mask);
}

/*
 * rule_gen_raddr ()
 *
 * Generate a remote address comparison.
 */
void rule_gen_raddr (u_int32_t addr, u_int32_t mask)
{
	add_code (BC_RADDR);
	add_code ((bytecode_t) addr);
	add_code ((bytecode_t) mask);
}

/*
 * rule_gen_exe ()
 *
 * Generate an executable filename comparison.
 */
void rule_gen_exe (unsigned long estrid)
{
	add_code (BC_EXE);
	add_code ((bytecode_t) estrid);
}

/*
 * Logical Operations.
 */

/*
 * rule_gen_or ()
 *
 * Generate a logical or.
 */
void rule_gen_or (void)
{
	add_code (BC_OR);
}

/*
 * rule_gen_and ()
 *
 * Generate a logical and.
 */
void rule_gen_and (void)
{
	add_code (BC_AND);
}

/*
 * rule_gen_not ()
 *
 * Generate a logical not.
 */
void rule_gen_not (void)
{
	add_code (BC_NOT);
}

/*
 * Macros used by rule_eval ().
 */

/*
 * Code macros.
 */
#define NEXTCODE (assert (ip < code_length) , code[ip++])
#define PEEKCODE ((ip < code_length) ? code[ip] : BC_NONE)

/*
 * Stack macros.
 */
#define POP (stack[--stack_ptr])
#define PEEK (stack[stack_ptr - 1])
#define PUSH(b)					\
if ((stack_ptr + 1) > stack_size) {		\
	bool_t *p;				\
						\
	stack_size += STACK_SIZE_STEP;		\
	if ((p = realloc (stack, sizeof (*p) * stack_size)) == NULL) \
		panic ("tcpspy: memory exhausted\n"); \
	stack = p;				\
	assert ((stack_ptr + 1) <= stack_size);	\
}						\
stack[stack_ptr++] = (b);

/*
 * Optimisation macros.
 *
 * SHORTCIRCUIT: "Short circuit" optimisation:
 * 	1. X OR Y is true if X is true, regardless of value of Y
 * 	2. X AND Y is false if X is false, regardless of value of Y
 */
#define SHORTCIRCUIT 				\
if ((PEEKCODE == BC_OR) && (PEEK == 1)) {	\
		(void) NEXTCODE;		\
		(void) POP;			\
		PUSH (1);			\
		break;				\
} else if ((PEEKCODE == BC_AND) && (PEEK == 0)) { \
		(void) NEXTCODE;		\
		(void) POP;			\
		PUSH (0);			\
		break;				\
}						

/*
 * rule_eval ()
 *
 * Execute the rule on the provided connection information.
 *
 * Returns nonzero if the connection matches the rule, zero otherwise.
 */
int rule_eval (uid_t muid, u_int32_t mladdr, u_int16_t mlport, 
		u_int32_t mraddr, u_int16_t mrport, const char *mexe)
{
	size_t ip = 0;
	unsigned int c;
	static bool_t *stack = NULL;
	static size_t stack_size = 0, stack_ptr = 0;

	stack_ptr = 0;

	for (ip = 0; ip < code_length; ) {
		c = NEXTCODE;
		switch (c) {

			/*
			 * Comparisons.
			 */

			case BC_USER:
				/*
				 * User comparison.
				 */
				{
				uid_t uid;

				uid = (uid_t) NEXTCODE;

				SHORTCIRCUIT;

				PUSH ((uid == muid) ? 1 : 0);
				}
				break;
			case BC_LPORT:
				/*
				 * Local port comparison.
				 */
				{
				u_int16_t hiport, loport;

				loport = (u_int16_t) NEXTCODE;
				hiport = (u_int16_t) NEXTCODE;

				SHORTCIRCUIT;

				PUSH (((mlport >= loport) && (mlport <= hiport))
						? 1 : 0);
				}
				break;
			case BC_RPORT:
				/*
				 * Remote port comparison.
				 */
				{
				u_int16_t hiport, loport;

				loport = (u_int16_t) NEXTCODE;
				hiport = (u_int16_t) NEXTCODE;

				SHORTCIRCUIT;

				PUSH (((mrport >= loport) && (mrport <= hiport))
						? 1 : 0);
				}
				break;
			case BC_LADDR:
				/*
				 * Local address comparison.
				 */
				{
				u_int32_t addr, mask;

				addr = (u_int32_t) NEXTCODE;
				mask = (u_int32_t) NEXTCODE;

				SHORTCIRCUIT;

				PUSH (((mladdr & mask) == addr) ? 1 : 0);
				}
				break;	
			case BC_RADDR:
				/*
				 * Remote address comparison.
				 */
				{
				u_int32_t addr, mask;

				addr = (u_int32_t) NEXTCODE;
				mask = (u_int32_t) NEXTCODE;

				SHORTCIRCUIT;

				PUSH (((mraddr & mask) == addr) ? 1 : 0);
				}
				break;	

			case BC_EXE:
				/*
				 * Executable filename comparison.
				 */
				{
				unsigned long estrid;
				const char *exe;

				estrid = (unsigned long) NEXTCODE;

				SHORTCIRCUIT;

				exe = st_retrieve (estrid);

				/*
				 * This instruction is always true if the -p
				 * option was not specified.
				 */
				if (mexe == NULL) {
					static int warned = 0;

					if (warned == 0) {
						logmsg ("warning: \"exe\" comparison used when executable names are not available; ignored");
						warned = 1;
					}
					
					PUSH (1);
					break;
				}

				/*
				 * Now compare using shell-style globbing.
				 *
				 * Note: Braces around statements because
				 * PUSH is a macro.
				 */
				if (fnmatch (exe, mexe, FNM_PATHNAME) == 0) {
					PUSH (1);
				} else {
					PUSH (0);
				}

				}
				break;

			/*
			 * Logical operations.
			 */

			case BC_OR:
				/*
				 * Logical or.
				 */
				{
				bool_t a, b;
				
				a = POP; b = POP;
				PUSH (((a == 1) || (b == 1)) ? 1 : 0);
				}
				break;
			case BC_AND:
				/*
				 * Logical and.
				 */
				{
				bool_t a, b;

				a = POP; b = POP;
				PUSH (((a == 1) && (b == 1)) ? 1 : 0);
				}
				break;
			case BC_NOT:
				/*
				 * Logical not.
				 */
				{
				bool_t a;

				a = POP;
				PUSH ((a == 0) ? 1 : 0);
				}
				break;

			default:
				/*
				 * Unhandled bytecode instruction, choke.
				 */
				assert (0);
		}
	}

	assert (stack_ptr == 1);
	return POP;
}

#undef NEXTCODE
#undef PEEKCODE
#undef POP
#undef PEEK
#undef PUSH
#undef SHORTCIRCUIT

/*
 * rule_parse ()
 *
 * Parse and compile the specified rule.
 */
void rule_parse (const char *r)
{
	extern int ruleparse (void);
	extern void rule_lexer_scan_string (const char *r);
	
	assert (r != NULL);
	rule_lexer_scan_string (r);
	if (ruleparse () != 0)
		abort ();
}

/*
 * rule_parse_file ()
 *
 * Parse and compile the specified open file.
 */
void rule_parse_file (FILE *fp)
{
	extern void rulerestart (FILE *);
	extern int ruleparse (void);

	rulerestart (fp);
	ruleparse ();
}

/*
 * add_code ()
 *
 * Add a bytecode or piece of data to the code array, dynamically resizing the
 * array as necessary.
 */
static void add_code (bytecode_t c)
{
	if ((code_length + 1) > code_size) {
		bytecode_t *p;
		
		code_size += CODE_SIZE_STEP;
		if ((p = realloc (code, sizeof (*p) * code_size)) == NULL)
			panic ("memory exhausted");
		code = p;
		assert ((code_length + 1) <= code_size);
	}
	code[code_length++] = c;
}

/*
 * st_store ()
 *
 * Store the specified string in the string table.
 *
 * Returns an integer that identifies the string and may be used with
 * st_retrieve () to retrieve the string.
 */
unsigned long st_store (const char *s)
{
	unsigned long id;
	
	assert (s != NULL);

	if ((stringtab_length + 1) > stringtab_size) {
		char **p;

		stringtab_size += STRINGTAB_SIZE_STEP;
		if ((p = realloc (stringtab, sizeof (*p) * stringtab_size)) ==
				NULL)
			panic ("memory exhausted");
		stringtab = p;
		assert ((stringtab_length + 1) <= stringtab_size);
	}

	id = stringtab_length;
	if ((stringtab[id] = strdup (s)) == NULL)
		panic ("memory exhausted");
	stringtab_length++;

	return id;
}

/*
 * st_retrieve ()
 *
 * Retrieve a string using an identifier returned by st_store ().
 *
 * Returns a pointer to the string.
 */
static const char *st_retrieve (unsigned long id)
{
	assert (id < stringtab_length); 
	assert (stringtab != NULL);
	assert (stringtab[id] != NULL);

	return stringtab[id];
}
