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
<<fseek>>, <<fseeko>>---set file position

INDEX
	fseek
INDEX
	fseeko
INDEX
	_fseek_r
INDEX
	_fseeko_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fseek(FILE *<[fp]>, long <[offset]>, int <[whence]>)
	int fseeko(FILE *<[fp]>, off_t <[offset]>, int <[whence]>)
	int _fseek_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	             long <[offset]>, int <[whence]>)
	int _fseeko_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	             off_t <[offset]>, int <[whence]>)

TRAD_SYNOPSIS
	#include <stdio.h>
	int fseek(<[fp]>, <[offset]>, <[whence]>)
	FILE *<[fp]>;
	long <[offset]>;
	int <[whence]>;

	int fseeko(<[fp]>, <[offset]>, <[whence]>)
	FILE *<[fp]>;
	off_t <[offset]>;
	int <[whence]>;

	int _fseek_r(<[ptr]>, <[fp]>, <[offset]>, <[whence]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	long <[offset]>;
	int <[whence]>;

	int _fseeko_r(<[ptr]>, <[fp]>, <[offset]>, <[whence]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	off_t <[offset]>;
	int <[whence]>;

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fseek>>/<<fseeko>> to set the position for the file identified by
<[fp]>.  The value of <[offset]> determines the new position, in one
of three ways selected by the value of <[whence]> (defined as macros
in `<<stdio.h>>'):

<<SEEK_SET>>---<[offset]> is the absolute file position (an offset
from the beginning of the file) desired.  <[offset]> must be positive.

<<SEEK_CUR>>---<[offset]> is relative to the current file position.
<[offset]> can meaningfully be either positive or negative.

<<SEEK_END>>---<[offset]> is relative to the current end of file.
<[offset]> can meaningfully be either positive (to increase the size
of the file) or negative.

See <<ftell>>/<<ftello>> to determine the current file position.

RETURNS
<<fseek>>/<<fseeko>> return <<0>> when successful.  On failure, the
result is <<EOF>>.  The reason for failure is indicated in <<errno>>:
either <<ESPIPE>> (the stream identified by <[fp]> doesn't support
repositioning) or <<EINVAL>> (invalid file position).

PORTABILITY
ANSI C requires <<fseek>>.

<<fseeko>> is defined by the Single Unix specification.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include "local.h"

#define	POS_ERR	(-(_fpos_t)1)

/*
 * Seek the given file to the given offset.
 * `Whence' must be one of the three SEEK_* macros.
 */

int
_DEFUN(_fseek_r, (ptr, fp, offset, whence),
       struct _reent *ptr _AND
       register FILE *fp  _AND
       long offset        _AND
       int whence)
{
  _fpos_t _EXFNPTR(seekfn, (struct _reent *, _PTR, _fpos_t, int));
  _fpos_t target;
  _fpos_t curoff = 0;
  size_t n;
#ifdef __USE_INTERNAL_STAT64
  struct stat64 st;
#else
  struct stat st;
#endif
  int havepos;

  /* Make sure stdio is set up.  */

  CHECK_INIT (ptr, fp);

  __sfp_lock_acquire ();
  _flockfile (fp);

  /* If we've been doing some writing, and we're in append mode
     then we don't really know where the filepos is.  */

  if (fp->_flags & __SAPP && fp->_flags & __SWR)
    {
      /* So flush the buffer and seek to the end.  */
      _fflush_r (ptr, fp);
    }

  /* Have to be able to seek.  */

  if ((seekfn = fp->_seek) == NULL)
    {
      ptr->_errno = ESPIPE;	/* ??? */
      _funlockfile (fp);
      __sfp_lock_release ();
      return EOF;
    }

  /*
   * Change any SEEK_CUR to SEEK_SET, and check `whence' argument.
   * After this, whence is either SEEK_SET or SEEK_END.
   */

  switch (whence)
    {
    case SEEK_CUR:
      /*
       * In order to seek relative to the current stream offset,
       * we have to first find the current stream offset a la
       * ftell (see ftell for details).
       */
      _fflush_r (ptr, fp);   /* may adjust seek offset on append stream */
      if (fp->_flags & __SOFF)
	curoff = fp->_offset;
      else
	{
	  curoff = seekfn (ptr, fp->_cookie, (_fpos_t) 0, SEEK_CUR);
	  if (curoff == -1L)
	    {
	      _funlockfile (fp);
	      __sfp_lock_release ();
	      return EOF;
	    }
	}
      if (fp->_flags & __SRD)
	{
	  curoff -= fp->_r;
	  if (HASUB (fp))
	    curoff -= fp->_ur;
	}
      else if (fp->_flags & __SWR && fp->_p != NULL)
	curoff += fp->_p - fp->_bf._base;

      offset += curoff;
      whence = SEEK_SET;
      havepos = 1;
      break;

    case SEEK_SET:
    case SEEK_END:
      havepos = 0;
      break;

    default:
      ptr->_errno = EINVAL;
      _funlockfile (fp);
      __sfp_lock_release ();
      return (EOF);
    }

  /*
   * Can only optimise if:
   *	reading (and not reading-and-writing);
   *	not unbuffered; and
   *	this is a `regular' Unix file (and hence seekfn==__sseek).
   * We must check __NBF first, because it is possible to have __NBF
   * and __SOPT both set.
   */

  if (fp->_bf._base == NULL)
    __smakebuf_r (ptr, fp);

  /* We do not do fseek optimization any more, for the sake of code size. */
  /*
   * We get here if we cannot optimise the seek ... just
   * do it.  Allow the seek function to change fp->_bf._base.
   */

dumb:
  if (_fflush_r (ptr, fp)
      || seekfn (ptr, fp->_cookie, offset, whence) == POS_ERR)
    {
      _funlockfile (fp);
      __sfp_lock_release ();
      return EOF;
    }
  /* success: clear EOF indicator and discard ungetc() data */
  if (HASUB (fp))
    FREEUB (ptr, fp);
  fp->_p = fp->_bf._base;
  fp->_r = 0;
  /* fp->_w = 0; *//* unnecessary (I think...) */
  fp->_flags &= ~__SEOF;
  /* Reset no-optimization flag after successful seek.  The
     no-optimization flag may be set in the case of a read
     stream that is flushed which by POSIX/SUSv3 standards,
     means that a corresponding seek must not optimize.  The
     optimization is then allowed if no subsequent flush
     is performed.  */
  memset (&fp->_mbstate, 0, sizeof (_mbstate_t));
  _funlockfile (fp);
  __sfp_lock_release ();
  return 0;
}

#ifndef _REENT_ONLY

int
_DEFUN(fseek, (fp, offset, whence),
       register FILE *fp _AND
       long offset       _AND
       int whence)
{
  return _fseek_r (_REENT, fp, offset, whence);
}

#endif /* !_REENT_ONLY */
