/*
 *-----------------------------------------------------------------------------
 *  Copyright (c) KEIL ELEKTRONIK GmbH and Franklin Software, Inc., 1987-1992
 *-----------------------------------------------------------------------------
 */
/* STDARG.H: prototypes for variable argument lists V3.00        */

#ifndef NULL
 #define NULL ((void *) 0)
#endif

typedef char *va_list;
#define va_start(ap,v) ap = (va_list)&v + sizeof(v)
#define va_arg(ap,t) (((t *)ap)++[0])
#define va_end(ap) (ap)
