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

/*
FUNCTION
<<puts>>---write a character string

INDEX
	puts
INDEX
	_puts_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int puts(const char *<[s]>);

	int _puts_r(struct _reent *<[reent]>, const char *<[s]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int puts(<[s]>)
	char *<[s]>;

	int _puts_r(<[reent]>, <[s]>)
	struct _reent *<[reent]>;
	char *<[s]>;

DESCRIPTION
<<puts>> writes the string at <[s]> (followed by a newline, instead of
the trailing null) to the standard output stream.

The alternate function <<_puts_r>> is a reentrant version.  The extra
argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
If successful, the result is a nonnegative integer; otherwise, the
result is <<EOF>>.

PORTABILITY
ANSI C requires <<puts>>, but does not specify that the result on
success must be <<0>>; any non-negative value is permitted.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <string.h>
#include "fvwrite.h"
#include "local.h"

/*
 * Write the given string to stdout, appending a newline.
 */

int
_DEFUN(_puts_r, (ptr, s),
       struct _reent *ptr _AND
       _CONST char * s)
{
  char *p = s;

  _REENT_SMALL_CHECK_INIT (ptr);
  while (*p)
  {
      if (_putc_r (ptr, *p, _stdout_r (ptr)) == EOF)
          return EOF;
      p++;
  }
  return (_putc_r (ptr, '\n', _stdout_r (ptr)));
}

#ifndef _REENT_ONLY

int
_DEFUN(puts, (s),
       char _CONST * s)
{
  char *p = s;

  _REENT_SMALL_CHECK_INIT (_REENT);
  while (*p)
  {
      if (_putc_r (_REENT, *p, _stdout_r (_REENT)) == EOF)
          return EOF;
      p++;
  }
  return (_putc_r (_REENT, '\n', _stdout_r (_REENT)));
}

#endif
