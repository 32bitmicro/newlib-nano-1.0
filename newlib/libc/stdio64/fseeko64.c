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
<<fseeko64>>---set file position for large file

INDEX
	fseeko64
INDEX
	_fseeko64_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fseeko64(FILE *<[fp]>, _off64_t <[offset]>, int <[whence]>)
	int _fseeko64_r (struct _reent *<[ptr]>, FILE *<[fp]>,
                         _off64_t <[offset]>, int <[whence]>)
TRAD_SYNOPSIS
	#include <stdio.h>

	int fseeko64(<[fp]>, <[offset]>, <[whence]>)
	FILE *<[fp]>;
	_off64_t <[offset]>;
	int <[whence]>;

	int _fseeko64_r (<[ptr]>, <[fp]>, <[offset]>, <[whence]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	_off64_t <[offset]>;
	int <[whence]>;

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fseeko64>> to set the position for the file identified by
<[fp]> that was opened via <<fopen64>>.  The value of <[offset]> determines
the new position, in one of three ways selected by the value of <[whence]>
(defined as macros in `<<stdio.h>>'):

<<SEEK_SET>>---<[offset]> is the absolute file position (an offset
from the beginning of the file) desired.  <[offset]> must be positive.

<<SEEK_CUR>>---<[offset]> is relative to the current file position.
<[offset]> can meaningfully be either positive or negative.

<<SEEK_END>>---<[offset]> is relative to the current end of file.
<[offset]> can meaningfully be either positive (to increase the size
of the file) or negative.

See <<ftello64>> to determine the current file position.

RETURNS
<<fseeko64>> returns <<0>> when successful.  On failure, the
result is <<EOF>>.  The reason for failure is indicated in <<errno>>:
either <<ESPIPE>> (the stream identified by <[fp]> doesn't support
repositioning or wasn't opened via <<fopen64>>) or <<EINVAL>>
(invalid file position).

PORTABILITY
<<fseeko64>> is a glibc extension.

Supporting OS subroutines required: <<close>>, <<fstat64>>, <<isatty>>,
<<lseek64>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "local.h"

#define	POS_ERR	(-(_fpos64_t)1)

#ifdef __LARGE64_FILES

/*
 * Seek the given file to the given offset.
 * `Whence' must be one of the three SEEK_* macros.
 */

_off64_t
_DEFUN (_fseeko64_r, (ptr, fp, offset, whence),
     struct _reent *ptr _AND
     register FILE *fp _AND
     _off64_t offset _AND
     int whence)
{
  _fpos64_t _EXFNPTR(seekfn, (struct _reent *, void *, _fpos64_t, int));
  _fpos64_t target, curoff;
  size_t n;

  struct stat64 st;
  int havepos;

  /* Only do 64-bit seek on large file.  */
  if (!(fp->_flags & __SL64))
    {
      if ((_off_t) offset != offset)
	{
	  ptr->_errno = EOVERFLOW;
	  return EOF;
	}
      return (_off64_t) _fseeko_r (ptr, fp, offset, whence);
    }

  /* Make sure stdio is set up.  */

  CHECK_INIT (ptr, fp);

  _flockfile (fp);

  curoff = fp->_offset;

  /* If we've been doing some writing, and we're in append mode
     then we don't really know where the filepos is.  */

  if (fp->_flags & __SAPP && fp->_flags & __SWR)
    {
      /* So flush the buffer and seek to the end.  */
      _fflush_r (ptr, fp);
    }

  /* Have to be able to seek.  */

  if ((seekfn = fp->_seek64) == NULL)
    {
      ptr->_errno = ESPIPE;	/* ??? */
      _funlockfile(fp);
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
	  curoff = seekfn (ptr, fp->_cookie, (_fpos64_t) 0, SEEK_CUR);
	  if (curoff == -1L)
	    {
	      _funlockfile(fp);
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
      _funlockfile(fp);
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
      _funlockfile(fp);
      return EOF;
    }
  /* success: clear EOF indicator and discard ungetc() data */
  if (HASUB (fp))
    FREEUB (ptr, fp);
  fp->_p = fp->_bf._base;
  fp->_r = 0;
  /* fp->_w = 0; *//* unnecessary (I think...) */
  fp->_flags &= ~__SEOF;
  _funlockfile(fp);
  return 0;
}

#ifndef _REENT_ONLY

_off64_t
_DEFUN (fseeko64, (fp, offset, whence),
     register FILE *fp _AND
     _off64_t offset _AND
     int whence)
{
  return _fseeko64_r (_REENT, fp, offset, whence);
}

#endif /* !_REENT_ONLY */

#endif /* __LARGE64_FILES */
