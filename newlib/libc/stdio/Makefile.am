## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	clearerr.c			\
	fclose.c			\
	fdopen.c			\
	feof.c				\
	ferror.c			\
	fflush.c			\
	fgetc.c			\
	fgetpos.c			\
	fgets.c			\
	fileno.c			\
	findfp.c			\
	flags.c			\
	fopen.c			\
	fprintf.c			\
	fputc.c			\
	fputs.c			\
	fread.c			\
	freopen.c			\
	fscanf.c			\
	fseek.c			\
	fsetpos.c			\
	ftell.c			\
	fvwrite.c			\
	fwalk.c			\
	fwrite.c			\
	getc.c				\
	getchar.c			\
	getc_u.c			\
	getchar_u.c			\
	getdelim.c			\
	getline.c			\
	gets.c				\
	makebuf.c			\
	perror.c			\
	printf.c			\
	putc.c				\
	putchar.c			\
	putc_u.c			\
	putchar_u.c			\
	puts.c				\
	refill.c			\
	remove.c			\
	rename.c			\
	rewind.c			\
	rget.c				\
	scanf.c			\
	sccl.c				\
	setbuf.c			\
	setbuffer.c			\
	setlinebuf.c			\
	setvbuf.c			\
	snprintf.c			\
	sprintf.c			\
	sscanf.c			\
	stdio.c			\
	tmpfile.c			\
	tmpnam.c			\
	ungetc.c			\
	vdprintf.c			\
	vprintf.c			\
	vscanf.c			\
	vsnprintf.c			\
	vsprintf.c			\
	vsscanf.c			\
	wbuf.c				\
	wsetup.c

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_2_SOURCES =
else
ELIX_2_SOURCES = \
	asprintf.c		\
	fcloseall.c		\
	fseeko.c		\
	ftello.c		\
	getw.c			\
	mktemp.c		\
	putw.c			\
	vasprintf.c
endif

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_4_SOURCES =
else
if ELIX_LEVEL_2
ELIX_4_SOURCES =
else
if ELIX_LEVEL_3
ELIX_4_SOURCES =
else
ELIX_4_SOURCES = \
	asnprintf.c		\
	dprintf.c		\
	fgetwc.c		\
	fgetws.c		\
	fmemopen.c		\
	fopencookie.c		\
	fpurge.c		\
	fputwc.c		\
	fputws.c		\
	funopen.c		\
	fwide.c			\
	fwprintf.c		\
	fwscanf.c		\
	getwc.c			\
	getwchar.c		\
	open_memstream.c	\
	putwc.c			\
	putwchar.c		\
	swprintf.c		\
	swscanf.c		\
	ungetwc.c		\
	vasnprintf.c		\
	vfwscanf.c		\
	vswprintf.c		\
	vswscanf.c		\
	vwprintf.c		\
	vwscanf.c		\
	wprintf.c		\
	wscanf.c

endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

LIBADD_OBJS = \
	$(lpfx)vfprintf_float.$(oext) \
	$(lpfx)svfprintf.$(oext) \
	$(lpfx)svfscanf.$(oext) \
	$(lpfx)vfprintf.$(oext) \
	$(lpfx)vfprintf_i.$(oext) \
	$(lpfx)vfscanf.$(oext) \
	$(lpfx)vfscanf_i.$(oext) \
	$(lpfx)vfscanf_float.$(oext) \
	$(lpfx)svfwprintf.$(oext) \
	$(lpfx)vfwprintf.$(oext) \
	$(lpfx)svfwscanf.$(oext) \
	$(lpfx)vfwscanf.$(oext)

libstdio_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdio.la
libstdio_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
libstdio_la_LIBADD = $(LIBADD_OBJS)
libstdio_la_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

# The rules for INTEGER_ONLY object files are deleted.

$(lpfx)vfprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

# Add rules compiling vfprintf_i.c and vfprintf_float.c

$(lpfx)vfprintf_i.$(oext): vfprintf_i.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfprintf_i.c -o $@

$(lpfx)vfprintf_float.$(oext): vfprintf_float.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfprintf_float.c -o $@

$(lpfx)vfwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -fshort-enums -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)vfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

# Add rules compiling vfscanf_i.c and vfscanf_float.c

$(lpfx)vfscanf_i.$(oext): vfscanf_i.c
	$(LIB_COMPILE) -c $(srcdir)/vfscanf_i.c -o $@

$(lpfx)vfscanf_float.$(oext): vfscanf_float.c
	$(LIB_COMPILE) -c $(srcdir)/vfscanf_float.c -o $@

$(lpfx)vfwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

CHEWOUT_FILES = \
	clearerr.def		\
	dprintf.def		\
	fclose.def		\
	fcloseall.def		\
	fdopen.def		\
	feof.def		\
	ferror.def		\
	fflush.def		\
	fgetc.def		\
	fgetpos.def		\
	fgets.def		\
	fgetwc.def		\
	fgetws.def		\
	fileno.def		\
	fmemopen.def		\
	fopen.def		\
	fopencookie.def		\
	fpurge.def		\
	fputc.def		\
	fputs.def		\
	fputwc.def		\
	fputws.def		\
	fread.def		\
	freopen.def		\
	fseek.def		\
	fsetpos.def		\
	ftell.def		\
	funopen.def		\
	fwide.def		\
	fwrite.def		\
	getc.def		\
	getc_u.def		\
	getchar.def		\
	getchar_u.def		\
	getdelim.def		\
	getline.def		\
	gets.def		\
	getw.def		\
	getwc.def		\
	getwchar.def		\
	mktemp.def		\
	open_memstream.def	\
	perror.def		\
	putc.def		\
	putc_u.def		\
	putchar.def		\
	putchar_u.def		\
	puts.def		\
	putw.def		\
	putwc.def		\
	putwchar.def		\
	remove.def		\
	rename.def		\
	rewind.def		\
	setbuf.def		\
	setbuffer.def		\
	setlinebuf.def		\
	setvbuf.def		\
	sprintf.def		\
	sscanf.def		\
	swprintf.def		\
	swscanf.def		\
	tmpfile.def		\
	tmpnam.def		\
	ungetc.def		\
	ungetwc.def		\
	vfprintf.def		\
	vfprintf_i.def		\
	vfprintf_float.def	\
	vfscanf.def		\
	vfscanf_i.def		\
	vfscanf_float.def		\
	vfwprintf.def		\
	vfwscanf.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/stdio.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

$(lpfx)fclose.$(oext): local.h
$(lpfx)fdopen.$(oext): local.h
$(lpfx)fflush.$(oext): local.h
$(lpfx)fgetwc.$(oext): local.h
$(lpfx)fgetws.$(oext): local.h
$(lpfx)findfp.$(oext): local.h
$(lpfx)fmemopen.$(oext): local.h
$(lpfx)fopen.$(oext): local.h
$(lpfx)fopencookie.$(oext): local.h
$(lpfx)fpurge.$(oext): local.h
$(lpfx)fputs.$(oext): fvwrite.h
$(lpfx)fputwc.$(oext): local.h
$(lpfx)fputws.$(oext): local.h fvwrite.h
$(lpfx)fread.$(oext): local.h
$(lpfx)freopen.$(oext): local.h
$(lpfx)fseek.$(oext): local.h
$(lpfx)ftell.$(oext): local.h
$(lpfx)funopen.$(oext): local.h
$(lpfx)fvwrite.$(oext): local.h fvwrite.h
$(lpfx)fwalk.$(oext): local.h
$(lpfx)fwide.$(oext): local.h
$(lpfx)fwprintf.$(oext): local.h
$(lpfx)fwrite.$(oext): local.h fvwrite.h
$(lpfx)fwscanf.$(oext): local.h
$(lpfx)getwc.$(oext): local.h
$(lpfx)getwchar.$(oext): local.h
$(lpfx)makebuf.$(oext): local.h
$(lpfx)open_memstream.$(oext): local.h
$(lpfx)puts.$(oext): fvwrite.h
$(lpfx)putwc.$(oext): local.h
$(lpfx)putwchar.$(oext): local.h
$(lpfx)refill.$(oext): local.h
$(lpfx)scanf.$(oext): local.h
$(lpfx)setbuf.$(oext): local.h
$(lpfx)setvbuf.$(oext): local.h
$(lpfx)sprintf.$(oext): local.h
$(lpfx)sscanf.$(oext): local.h
$(lpfx)stdio.$(oext): local.h
$(lpfx)svfprintf.$(oext): local.h vfprintf_local.h
$(lpfx)svfscanf.$(oext): local.h floatio.h
$(lpfx)swprintf.$(oext): local.h
$(lpfx)swscanf.$(oext): local.h
$(lpfx)ungetc.$(oext): local.h
$(lpfx)ungetwc.$(oext): local.h
$(lpfx)vfprintf.$(oext): local.h vfprintf_local.h
$(lpfx)vfprintf_i.$(oext): local.h vfprintf_local.h
$(lpfx)vfprintf_float.$(oext): local.h vfprintf_local.h
$(lpfx)vfscanf.$(oext): local.h floatio.h vfscanf_local.h
$(lpfx)vfscanf_i.$(oext): local.h floatio.h vfscanf_local.h
$(lpfx)vfscanf_float.$(oext): local.h floatio.h vfscanf_local.h
$(lpfx)vfwprintf.$(oext): local.h
$(lpfx)vfwscanf.$(oext): local.h
$(lpfx)vscanf.$(oext): local.h
$(lpfx)vsscanf.$(oext): local.h
$(lpfx)vswprintf.$(oext): local.h
$(lpfx)vswscanf.$(oext): local.h
$(lpfx)vwprintf.$(oext): local.h
$(lpfx)vwscanf.$(oext): local.h
$(lpfx)wbuf.$(oext): local.h fvwrite.h
$(lpfx)wprintf.$(oext): local.h
$(lpfx)wscanf.$(oext): local.h
$(lpfx)wsetup.$(oext): local.h
