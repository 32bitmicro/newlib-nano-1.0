/*-
 * Copyright (c) 2002-2004 Tim J. Robbins.
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<fgetws>>---get wide character string from a file or stream

INDEX
	fgetws
INDEX
	_fgetws_r

ANSI_SYNOPSIS
	#include <wchar.h>
	wchar_t *fgetws(wchar_t *<[ws]>, int <[n]>, FILE *<[fp]>);

	#include <wchar.h>
	wchar_t *_fgetws_r(struct _reent *<[ptr]>, wchar_t *<[ws]>, int <[n]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	wchar_t *fgetws(<[ws]>,<[n]>,<[fp]>)
	wchar_t *<[ws]>;
	int <[n]>;
	FILE *<[fp]>;

	#include <wchar.h>
	wchar_t *_fgetws_r(<[ptr]>, <[ws]>,<[n]>,<[fp]>)
	struct _reent *<[ptr]>;
	wchar_t *<[ws]>;
	int <[n]>;
	FILE *<[fp]>;

DESCRIPTION
Reads at most <[n-1]> wide characters from <[fp]> until a newline
is found. The wide characters including to the newline are stored
in <[ws]>. The buffer is terminated with a 0.

The <<_fgetws_r>> function is simply the reentrant version of
<<fgetws>> and is passed an additional reentrancy structure
pointer: <[ptr]>.

RETURNS
<<fgetws>> returns the buffer passed to it, with the data
filled in. If end of file occurs with some data already
accumulated, the data is returned with no other indication. If
no data are read, NULL is returned instead.

PORTABILITY
C99, POSIX.1-2001
*/

#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "local.h"

wchar_t *
_DEFUN(_fgetws_r, (ptr, ws, n, fp),
	struct _reent *ptr _AND
	wchar_t * ws _AND
	int n _AND
	FILE * fp)
{
  wchar_t *wsp;
  size_t nconv;
  const char *src;
  unsigned char *nl;

  __sfp_lock_acquire ();
  _flockfile (fp);
  ORIENT (fp, 1);

  if (n <= 0)
    {
      errno = EINVAL;
      goto error;
    }

  if (fp->_r <= 0 && __srefill_r (ptr, fp))
    /* EOF */
    goto error;
  wsp = ws;
  do
    {
      src = (char *) fp->_p;
      nl = memchr (fp->_p, '\n', fp->_r);
      nconv = _mbsrtowcs_r (ptr, wsp, &src,
			    nl != NULL ? (nl - fp->_p + 1) : fp->_r,
			    &fp->_mbstate);
      if (nconv == (size_t) -1)
	/* Conversion error */
	goto error;
      if (src == NULL)
	{
	  /*
	   * We hit a null byte. Increment the character count,
	   * since mbsnrtowcs()'s return value doesn't include
	   * the terminating null, then resume conversion
	   * after the null.
	   */
	  nconv++;
	  src = memchr (fp->_p, '\0', fp->_r);
	  src++;
	}
      fp->_r -= (unsigned char *) src - fp->_p;
      fp->_p = (unsigned char *) src;
      n -= nconv;
      wsp += nconv;
    }
  while (wsp[-1] != L'\n' && n > 1 && (fp->_r > 0
	 || __srefill_r (ptr, fp) == 0));
  if (wsp == ws)
    /* EOF */
    goto error;
  if (!mbsinit (&fp->_mbstate))
    /* Incomplete character */
    goto error;
  *wsp++ = L'\0';
  _funlockfile (fp);
  __sfp_lock_release ();
  return ws;

error:
  _funlockfile (fp);
  __sfp_lock_release ();
  return NULL;
}

wchar_t *
_DEFUN(fgetws, (ws, n, fp),
	wchar_t *ws _AND
	int n _AND
	FILE *fp)
{
  CHECK_INIT (_REENT, fp);
  return _fgetws_r (_REENT, ws, n, fp);
}
