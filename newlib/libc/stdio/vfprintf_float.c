/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <newlib.h>

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/lock.h>
#include <stdarg.h>
#include "local.h"
#include "../stdlib/local.h"
#include "fvwrite.h"
#include "vfieeefp.h"

#include "vfprintf_local.h"

char *
__cvt(struct _reent *data, _PRINTF_FLOAT_TYPE value, int ndigits, int flags,
      char *sign, int *decpt, int ch, int *length, char *buf);

int
__exponent(char *p0, int exp, int fmtch);

#ifdef FLOATING_POINT

/* Using reentrant DATA, convert finite VALUE into a string of digits
   with no decimal point, using NDIGITS precision and FLAGS as guides
   to whether trailing zeros must be included.  Set *SIGN to nonzero
   if VALUE was negative.  Set *DECPT to the exponent plus one.  Set
   *LENGTH to the length of the returned string.  CH must be one of
   [aAeEfFgG]; if it is [aA], then the return string lives in BUF,
   otherwise the return value shares the mprec reentrant storage.  */
char *
__cvt(struct _reent *data, _PRINTF_FLOAT_TYPE value, int ndigits, int flags,
      char *sign, int *decpt, int ch, int *length, char *buf)
{
	int mode, dsgn;
	char *digits, *bp, *rve;
	union double_union tmp;

	tmp.d = value;
	if (word0 (tmp) & Sign_bit) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = '-';
	} else
		*sign = '\000';

	if (ch == 'f' || ch == 'F') {
		mode = 3;		/* ndigits after the decimal point */
	} else {
		/* To obtain ndigits after the decimal point for the 'e'
		 * and 'E' formats, round to ndigits + 1 significant
		 * figures.
		 */
		if (ch == 'e' || ch == 'E') {
			ndigits++;
		}
		mode = 2;		/* ndigits significant digits */
	}

	digits = _DTOA_R (data, value, mode, ndigits, decpt, &dsgn, &rve);

	if ((ch != 'g' && ch != 'G') || flags & ALT) {	/* Print trailing zeros */
		bp = digits + ndigits;
		if (ch == 'f' || ch == 'F') {
			if (*digits == '0' && value)
				*decpt = -ndigits + 1;
			bp += *decpt;
		}
		if (value == 0)	/* kludge for __dtoa irregularity */
			rve = bp;
		while (rve < bp)
			*rve++ = '0';
	}
	*length = rve - digits;
	return (digits);
}

int
__exponent(char *p0, int exp, int fmtch)
{
	register char *p, *t;
	char expbuf[MAXEXPLEN];
#define isa 0

	p = p0;
	*p++ = isa ? 'p' - 'a' + fmtch : fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = '-';
	}
	else
		*p++ = '+';
	t = expbuf + MAXEXPLEN;
	if (exp > 9) {
		do {
			*--t = to_char (exp % 10);
		} while ((exp /= 10) > 9);
		*--t = to_char (exp);
		for (; t < expbuf + MAXEXPLEN; *p++ = *t++);
	}
	else {
		if (!isa)
			*p++ = '0';
		*p++ = to_char (exp);
	}
	return (p - p0);
}
/* Decode and print floating point number specified by "eEfgG". */
int 
_printf_float (struct _reent *data,
	       struct _prt_data_t *pdata,
	       FILE *fp,
	       int (*pfunc)(struct _reent *, int, FILE *),
	       va_list *ap)
{
	char *decimal_point = _localeconv_r (data)->decimal_point;
	size_t decp_len = strlen (decimal_point);
	char softsign;		/* temporary negative sign for floats */
# define _fpvalue (pdata->_double_)
	int expt;		/* integer value of exponent */
	int expsize = 0;	/* character count for expstr */
	int ndig = 0;		/* actual number of digits returned by cvt */
	char *cp;
	int n;
	int realsz;		/* field size expanded by dprec(not for _printf_float) */
	char code = pdata->code;

	if (pdata->flags & LONGDBL) {
		_fpvalue = (double) GET_ARG (N, *ap, _LONG_DOUBLE);
	} else {
		_fpvalue = GET_ARG (N, *ap, double);
	}

	/* do this before tricky precision changes

	   If the output is infinite or NaN, leading
	   zeros are not permitted.  Otherwise, scanf
	   could not read what printf wrote.
	 */
	if (isinf (_fpvalue)) {
		if (_fpvalue < 0)
			pdata->l_buf[0] = '-';
		if (code <= 'G') /* 'A', 'E', 'F', or 'G' */
			cp = "INF";
		else
			cp = "inf";
		pdata->size = 3;
		pdata->flags &= ~ZEROPAD;
                goto print_float;
	}
	if (isnan (_fpvalue)) {
		if (code <= 'G') /* 'A', 'E', 'F', or 'G' */
			cp = "NAN";
		else
			cp = "nan";
		pdata->size = 3;
		pdata->flags &= ~ZEROPAD;
                goto print_float;
	}

	if (pdata->prec == -1) {
		pdata->prec = DEFPREC;
	} else if ((code == 'g' || code == 'G') && pdata->prec == 0) {
		pdata->prec = 1;
	}

	pdata->flags |= FPT;

	cp = __cvt (data, _fpvalue, pdata->prec, pdata->flags, &softsign,
		    &expt, code, &ndig, cp);

	if (code == 'g' || code == 'G') {
		if (expt <= -4 || expt > pdata->prec)
			code -= 2; /* 'e' or 'E' */
		else
			code = 'g';
	}
	if (code <= 'e') {	/* 'a', 'A', 'e', or 'E' fmt */
		--expt;
		expsize = __exponent (pdata->expstr, expt, code);
		pdata->size = expsize + ndig;
		if (ndig > 1 || pdata->flags & ALT)
			++pdata->size;
		} else {
			if (code == 'f') {		/* f fmt */
				if (expt > 0) {
					pdata->size = expt;
					if (pdata->prec || pdata->flags & ALT)
						pdata->size += pdata->prec + 1;
				} else	/* "0.X" */
					pdata->size = (pdata->prec || pdata->flags & ALT)
						  		? pdata->prec + 2
						  		: 1;
			} else if (expt >= ndig) { /* fixed g fmt */
				pdata->size = expt;
				if (pdata->flags & ALT)
					++pdata->size;
			} else
				pdata->size = ndig + (expt > 0 ?
							     1 : 2 - expt);
				pdata->lead = expt;
		}

		if (softsign)
			pdata->l_buf[0] = '-';
print_float:
	if (_printf_common (data, pdata, &realsz, fp, pfunc) == -1)
		goto error;

	if ((pdata->flags & FPT) == 0) {
		PRINT (cp, pdata->size);
	} else {	/* glue together f_p fragments */
		if (code >= 'f') {	/* 'f' or 'g' */
			if (_fpvalue == 0) {
				/* kludge for __dtoa irregularity */
				PRINT ("0", 1);
				if (expt < ndig || pdata->flags & ALT) {
					PRINT (decimal_point, decp_len);
					PAD (ndig - 1, pdata->zero);
				}
			} else if (expt <= 0) {
				PRINT ("0", 1);
				if (expt || ndig || pdata->flags & ALT) {
					PRINT (decimal_point, decp_len);
					PAD (-expt, pdata->zero);
					PRINT (cp, ndig);
				}
			} else {
				char *convbuf = cp;
				PRINTANDPAD(cp, convbuf + ndig,
					    pdata->lead, pdata->zero);
				cp += pdata->lead;
				if (expt < ndig || pdata->flags & ALT)
				    PRINT (decimal_point, decp_len);
				PRINTANDPAD (cp, convbuf + ndig,
					     ndig - expt, pdata->zero);
			}
		} else {	/* 'a', 'A', 'e', or 'E' */
			if (ndig > 1 || pdata->flags & ALT) {
				PRINT (cp, 1);
				cp++;
				PRINT (decimal_point, decp_len);
				if (_fpvalue) {
					PRINT (cp, ndig - 1);
				} else	/* 0.[0..] */
					/* __dtoa irregularity */
					PAD (ndig - 1, pdata->zero);
			} else	/* XeYYY */
				PRINT (cp, 1);
			PRINT (pdata->expstr, expsize);
		}
	}
	
	/* left-adjusting padding (always blank) */
	if (pdata->flags & LADJUST)
		PAD (pdata->width - realsz, pdata->blank);

	return (pdata->width > realsz ? pdata->width : realsz);
error:
	return -1;
}

#endif
