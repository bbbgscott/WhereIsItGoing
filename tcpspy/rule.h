/*
 * rule.h - Bytecode compiler and interpreter for rules
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
 * $Id: rule.h,v 1.3 2001/01/05 14:04:41 fyre Stab $
 */

#ifndef RULE_H
#define RULE_H

#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

void rule_gen_user (uid_t uid);
void rule_gen_lport (u_int16_t low, u_int16_t high);
void rule_gen_rport (u_int16_t low, u_int16_t high);
void rule_gen_laddr (u_int32_t addr, u_int32_t mask);
void rule_gen_raddr (u_int32_t addr, u_int32_t mask);
void rule_gen_exe (unsigned long estrid);
void rule_gen_or (void);
void rule_gen_and (void);
void rule_gen_not (void);
int rule_eval (uid_t muid, u_int32_t mladdr, u_int16_t mlport,
		u_int32_t mraddr, u_int16_t mrport, const char *mexe);
void rule_parse (const char *r);
void rule_parse_file (FILE *fp);

unsigned long st_store (const char *s);

#endif
