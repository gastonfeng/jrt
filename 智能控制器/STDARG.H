/*			- STDARG.H -

   The ANSI macros for reading variable argument lists (...).

   Version: 3.00 [IANR]

*/

#ifndef _STDARG_INCLUDED

#if (__TID__ >> 8) == 4		 /* Z80/64180 */

typedef struct
  {
    char	*parm;
    char	*addr;
    char	argc;
  } va_list[1];

char *_get_varg (va_list ap, char type, char size);

# define va_start(ap,parmN) ((ap[0].addr = ((char *) &parmN) + sizeof(parmN)),\
                             (ap[0].parm = _args$))

# define va_arg(ap,mode) *(mode *) _get_varg (ap, _argt$(mode), sizeof(mode))

#else		/* Not Z80 */

typedef void *va_list[1];

#if (__TID__ >> 8) == 1 && (__TID__ & 15) == 0

/* Single chip 8051 memory model special */

# define va_start(ap,parmN) (ap[0] = &parmN)
# define va_arg(ap,mode) (*--(*(mode **)ap))

#else		/* Not Z80 or 8051/small */

# define va_start(ap,parmN) ap[0] = ((char *) &parmN) + sizeof(parmN)
# define va_arg(ap,mode) (*(*(mode **) ap)++)

#endif

#endif

# define va_end(ap) 	/* -g option warns here - OK */

#define _STDARG_INCLUDED

#endif
