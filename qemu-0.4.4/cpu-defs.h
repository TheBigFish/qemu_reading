/*
 * common defines for all CPUs
 * 
 * Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef CPU_DEFS_H
#define CPU_DEFS_H

#include "config.h"
#include <setjmp.h>

#define EXCP_INTERRUPT 	256 /* async interruption */
#define EXCP_HLT        257 /* hlt instruction reached */
#define EXCP_DEBUG      258 /* cpu stopped after a breakpoint or singlestep */

#define MAX_BREAKPOINTS 32

#define CPU_TLB_SIZE 256

typedef struct CPUTLBEntry {
    uint32_t address;
    uint32_t addend;
} CPUTLBEntry;

#endif
