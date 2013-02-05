/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* doc in vfprintf.c */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>

#ifndef _REENT_ONLY

int
_DEFUN(vsprintf, (str, fmt, ap),
       char *str        _AND
       const char *fmt _AND
       va_list ap)
{
  return _vsprintf_r (_REENT, str, fmt, ap);
}

int
_EXFUN(vsiprintf, (char *, const char *, __VALIST)
       _ATTRIBUTE ((alias("vsprintf"))));
#endif /* !_REENT_ONLY */

int
_DEFUN(_vsprintf_r, (ptr, str, fmt, ap),
       struct _reent *ptr _AND
       char *str          _AND
       const char *fmt   _AND
       va_list ap)
{
  int ret;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = INT_MAX;
  f._file = -1;  /* No file. */
  ret = _svfprintf_r (ptr, &f, fmt, ap);
  *f._p = 0;
  return ret;
}

int
_EXFUN(_vsiprintf_r, (struct _reent *, char *, const char *, __VALIST)
       _ATTRIBUTE ((alias("_vsprintf_r"))));
